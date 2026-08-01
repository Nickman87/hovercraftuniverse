// OgreCollisionProvider.cpp -- Phase B collision reconstruction.
// See docs/porting/phase-b-collision.md for the design this implements, and
// hu_collision/OgreCollisionProvider.h for the class-level summary.
#include "hu_collision/OgreCollisionProvider.h"

#include <OgreResourceGroupManager.h>
#include <OgreMeshManager.h>
#include <OgreMesh.h>
#include <OgreMeshSerializer.h>
#include <OgreDefaultHardwareBufferManager.h>
#include <OgreSubMesh.h>
#include <OgreHardwareVertexBuffer.h>
#include <OgreHardwareIndexBuffer.h>
#include <OgreVector.h>
#include <OgreQuaternion.h>
#include <OgreArchive.h>
#include <OgreLogManager.h>

#include <tinyxml/tinyxml.h>

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionShapes/btShapeHull.h>
#include <BulletCollision/CollisionShapes/btConvexHullShape.h>
#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>
#include <BulletCollision/CollisionShapes/btTriangleMesh.h>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <set>
#include <sstream>
#include <unordered_set>

namespace hu_collision {

namespace {

// ---------------------------------------------------------------------------
// Small helpers
// ---------------------------------------------------------------------------

std::string toLower(const std::string& s) {
    std::string r = s;
    std::transform(r.begin(), r.end(), r.begin(), [](unsigned char c) { return (char)std::tolower(c); });
    return r;
}

// ".\\levels\\junkyard.hkx" -> "junkyard" ; also tolerates forward slashes.
std::string stemOf(const std::string& path) {
    size_t slash = path.find_last_of("/\\");
    std::string file = (slash == std::string::npos) ? path : path.substr(slash + 1);
    size_t dot = file.find_last_of('.');
    return (dot == std::string::npos) ? file : file.substr(0, dot);
}

void log(const std::string& msg) {
    // Ogre::LogManager may or may not have a default log depending on how
    // early this runs; std::cerr is the fallback every other TODO(phaseB)
    // message in this porting effort already uses (see
    // havok_compat::todoPhaseBOnce), so mirror that rather than risk a null
    // Ogre log pointer.
    if (Ogre::LogManager::getSingletonPtr() && Ogre::LogManager::getSingleton().getDefaultLog()) {
        Ogre::LogManager::getSingleton().logMessage("[hu_collision] " + msg);
    } else {
        std::cerr << "[hu_collision] " << msg << std::endl;
    }
}

// Trigger externals types built in C++ as hkpAabbPhantom/hkpBoxShape volumes
// (HoverCraftUniverseWorld::createStart/createFinish/createCheckpoint/
// createPortal/createBoost) that never touch the .hkx -- see
// docs/porting/phase-b-collision.md section 3.4. Verified against the exact
// strings ServerLoader::parseWorldUserData() switches on
// (HovercraftUniverse/HovercraftUniverse/ServerLoader.cpp), NOT the design
// doc's list verbatim -- the doc says "SpeedBoost" but the real XML root
// tag/externals userData value is "Boost" (see SoccerField.scene's
// FinishRoom_ent item); ServerLoader.cpp is the ground truth here.
bool isTriggerType(const std::string& rootTag) {
    static const std::unordered_set<std::string> kTriggerTypes = {
        "Start", "StartPosition", "Finish", "CheckPoint", "Portal", "Boost", "PowerupSpawn", "ResetSpawn"
    };
    return kTriggerTypes.count(rootTag) != 0;
}

// Reads an element's first text/CDATA child, same as TiXmlElement::GetText()
// but tolerant of a null element.
std::string childText(const TiXmlElement* el) {
    if (!el) return std::string();
    const char* t = el->GetText();
    return t ? std::string(t) : std::string();
}

struct MeshNode {
    std::string name;      // the <entity name="..."> attribute -- matches what
                            // getOgreEntity()/mEntityName look up via
                            // findRigidBodyByName()
    std::string meshFile;
    Ogre::Vector3 worldPos = Ogre::Vector3::ZERO;
    Ogre::Quaternion worldRot = Ogre::Quaternion::IDENTITY;
    Ogre::Vector3 worldScale = Ogre::Vector3::UNIT_SCALE;
};

struct ParsedScene {
    std::vector<MeshNode> meshNodes;
    std::unordered_set<std::string> excludedNames; // trigger-type OgreEntity names
};

Ogre::Vector3 loadXYZ(const TiXmlElement* el, const Ogre::Vector3& def) {
    if (!el) return def;
    Ogre::Vector3 v = def;
    if (el->Attribute("x")) v.x = (Ogre::Real)atof(el->Attribute("x"));
    if (el->Attribute("y")) v.y = (Ogre::Real)atof(el->Attribute("y"));
    if (el->Attribute("z")) v.z = (Ogre::Real)atof(el->Attribute("z"));
    return v;
}

Ogre::Quaternion loadRotation(const TiXmlElement* el) {
    if (!el) return Ogre::Quaternion::IDENTITY;
    if (el->Attribute("qx")) {
        return Ogre::Quaternion(
            (Ogre::Real)atof(el->Attribute("qw") ? el->Attribute("qw") : "1"),
            (Ogre::Real)atof(el->Attribute("qx")),
            (Ogre::Real)atof(el->Attribute("qy") ? el->Attribute("qy") : "0"),
            (Ogre::Real)atof(el->Attribute("qz") ? el->Attribute("qz") : "0"));
    }
    if (el->Attribute("axisX")) {
        Ogre::Real angle = el->Attribute("angle") ? (Ogre::Real)atof(el->Attribute("angle")) : 0;
        Ogre::Vector3 axis(
            el->Attribute("axisX") ? (Ogre::Real)atof(el->Attribute("axisX")) : 0,
            el->Attribute("axisY") ? (Ogre::Real)atof(el->Attribute("axisY")) : 0,
            el->Attribute("axisZ") ? (Ogre::Real)atof(el->Attribute("axisZ")) : 0);
        Ogre::Quaternion q;
        q.FromAngleAxis(Ogre::Radian(angle), axis);
        return q;
    }
    return Ogre::Quaternion::IDENTITY;
}

// Recursively walks a <node> element (and its <node> children), composing
// world transforms exactly the way Ogre::Node::_updateFromParent() does
// (derived scale/orientation/position with default inheritScale/inheritOrientation
// both true -- see OgreMaxScene.cpp's LoadNode for the matching original
// parse this mirrors, independently, per docs/porting/phase-b-collision.md
// section 3.3: this is a second, from-scratch TinyXML pass, not a hook into
// OgreMax's own in-flight scene load).
void walkNode(const TiXmlElement* nodeEl,
              const Ogre::Vector3& parentPos, const Ogre::Quaternion& parentRot, const Ogre::Vector3& parentScale,
              std::vector<MeshNode>& out) {
    if (!nodeEl) return;

    const char* nameAttr = nodeEl->Attribute("name");
    std::string nodeName = nameAttr ? nameAttr : "";

    Ogre::Vector3 localPos = Ogre::Vector3::ZERO;
    Ogre::Quaternion localRot = Ogre::Quaternion::IDENTITY;
    Ogre::Vector3 localScale = Ogre::Vector3::UNIT_SCALE;

    // First pass: pick up this node's own position/rotation/scale (order in
    // the file is position/scale/rotation per every .scene sample seen, but
    // read whichever tags are present rather than assume order).
    for (const TiXmlElement* child = nodeEl->FirstChildElement(); child; child = child->NextSiblingElement()) {
        std::string tag = child->Value();
        if (tag == "position") localPos = loadXYZ(child, localPos);
        else if (tag == "scale") localScale = loadXYZ(child, localScale);
        else if (tag == "rotation") localRot = loadRotation(child);
    }

    Ogre::Vector3 worldScale = parentScale * localScale;
    Ogre::Quaternion worldRot = parentRot * localRot;
    // Ogre composes position as parent.pos + parent.rot * (parent.scale * local.pos)
    Ogre::Vector3 scaledLocalPos = parentScale * localPos;
    Ogre::Vector3 worldPos = parentPos + (parentRot * scaledLocalPos);

    // Second pass: recurse into child <node>s and collect <entity> children
    // attached directly to this node.
    for (const TiXmlElement* child = nodeEl->FirstChildElement(); child; child = child->NextSiblingElement()) {
        std::string tag = child->Value();
        if (tag == "node")
            walkNode(child, worldPos, worldRot, worldScale, out);
        else if (tag == "entity") {
            const char* entName = child->Attribute("name");
            const char* meshFile = child->Attribute("meshFile");
            if (meshFile && *meshFile) {
                MeshNode mn;
                mn.name = entName ? entName : nodeName;
                mn.meshFile = meshFile;
                mn.worldPos = worldPos;
                mn.worldRot = worldRot;
                mn.worldScale = worldScale;
                out.push_back(mn);
            }
        }
    }
}

bool parseSceneXml(const std::string& xmlText, ParsedScene& parsed) {
    TiXmlDocument doc;
    doc.Parse(xmlText.c_str());
    const TiXmlElement* root = doc.RootElement();
    if (!root) return false;

    const TiXmlElement* nodesEl = root->FirstChildElement("nodes");
    if (nodesEl) {
        for (const TiXmlElement* child = nodesEl->FirstChildElement(); child; child = child->NextSiblingElement()) {
            if (std::string(child->Value()) == "node")
                walkNode(child, Ogre::Vector3::ZERO, Ogre::Quaternion::IDENTITY, Ogre::Vector3::UNIT_SCALE, parsed.meshNodes);
        }
    }

    const TiXmlElement* externalsEl = root->FirstChildElement("externals");
    if (externalsEl) {
        for (const TiXmlElement* item = externalsEl->FirstChildElement("item"); item; item = item->NextSiblingElement("item")) {
            const TiXmlElement* userDataEl = item->FirstChildElement("userData");
            if (!userDataEl) continue;
            std::string userData = childText(userDataEl);
            if (userData.empty()) continue;

            TiXmlDocument udoc;
            udoc.Parse(userData.c_str());
            const TiXmlElement* uroot = udoc.RootElement();
            if (!uroot) continue;

            if (isTriggerType(uroot->Value())) {
                const TiXmlElement* ogreEntityEl = uroot->FirstChildElement("OgreEntity");
                std::string ogreEntity = childText(ogreEntityEl);
                if (!ogreEntity.empty()) parsed.excludedNames.insert(ogreEntity);
            }
        }
    }

    return true;
}

// Finds the .scene resource matching `stem` (case-insensitively, per
// docs/porting/phase-b-collision.md section 3.2's "junkyard.hkx" vs
// "Junkyard.scene" note) across every currently-defined Ogre resource
// group. Returns the exact-cased resource name and its owning group.
bool findSceneResource(const std::string& stem, std::string& outResourceName, std::string& outGroup) {
    std::string wantLower = toLower(stem) + ".scene";
    auto& rgm = Ogre::ResourceGroupManager::getSingleton();
    for (const auto& group : rgm.getResourceGroups()) {
        Ogre::StringVectorPtr names = rgm.listResourceNames(group);
        for (auto& name : *names) {
            if (toLower(name) == wantLower) {
                outResourceName = name;
                outGroup = group;
                return true;
            }
        }
    }
    return false;
}

// Resolving a bare "FootballPlanet.mesh" (exactly what a .scene's meshFile
// attribute gives -- no path) turns out to need an actual recursive search,
// not a direct join-and-stat: this game's asset layout nests each track's
// meshes one directory deeper than its .scene/.hkx/.path files (confirmed
// against the real data directory -- e.g. data/levels/SoccerField.scene
// sits next to a data/levels/SoccerField/ subfolder, and
// "FootballPlanet.mesh" actually lives at
// data/levels/SoccerField/models/FootballPlanet.mesh). Two consequences,
// both confirmed empirically via hu_collision_smoketest:
//  - Ogre::ResourceGroupManager::openResource()/resourceExists() (which
//    consult a per-group index built by a non-recursive
//    addResourceLocation()) never find it -- true for both a brand-new
//    group AND the game's own pre-existing "General" group.
//  - Ogre::Archive::exists(name) does a direct, non-recursive
//    concatenate_path(locationRoot, name) + stat -- also never finds it.
// findSceneResource() above works only because the .scene file itself IS a
// direct child of "levels/", not nested.
//
// Archive::find(pattern, recursive=true), by contrast, matches by pattern
// against each recursively-discovered file and returns the fully-qualified
// relative path (e.g. "SoccerField/models/FootballPlanet.mesh") -- which
// Archive::open() can then use directly. This is what actually resolves a
// bare mesh filename to its real nested location.
Ogre::DataStreamPtr openMeshStream(const std::string& sourceGroup, const std::string& meshFile) {
    auto& rgm = Ogre::ResourceGroupManager::getSingleton();
    for (const auto& loc : rgm.getResourceLocationList(sourceGroup)) {
        Ogre::StringVectorPtr matches = loc.archive->find(meshFile, /*recursive=*/true, /*dirs=*/false);
        if (!matches->empty()) {
            return loc.archive->open(matches->front());
        }
    }
    return Ogre::DataStreamPtr();
}

// Loads `meshFile` (found via `sourceGroup`'s archive locations, the same
// Ogre resource group the track's own .scene resolved to -- mesh and scene
// are always colocated in this game's data layout) as a brand-new,
// standalone Ogre::Mesh with guaranteed shadowed vertex/index buffers,
// WITHOUT going through MeshManager::load()/MeshManager's resource cache at
// all -- so there is no possible collision with an unshadowed instance the
// render path may have separately loaded under MeshManager's own cache (the
// single real, race-condition-relevant risk flagged in
// docs/porting/phase-b-collision.md section 3.6): this Mesh is never
// registered there in the first place.
Ogre::Mesh* loadMeshDirect(const std::string& sourceGroup, const std::string& meshFile) {
    Ogre::DataStreamPtr stream = openMeshStream(sourceGroup, meshFile);
    if (!stream) return nullptr;

    static int counter = 0;
    std::string uniqueName = "hu_collision::" + meshFile + "::" + std::to_string(++counter);
    auto* mesh = new Ogre::Mesh(Ogre::MeshManager::getSingletonPtr(), uniqueName, 0, sourceGroup, /*isManual=*/true);
    mesh->setVertexBufferPolicy(Ogre::HardwareBuffer::HBU_STATIC, /*shadowBuffer=*/true);
    mesh->setIndexBufferPolicy(Ogre::HardwareBuffer::HBU_STATIC, /*shadowBuffer=*/true);

    Ogre::MeshSerializer serializer;
    serializer.importMesh(stream, mesh);
    return mesh;
}

// Extracts world-space (scale-baked, per docs/porting/phase-b-collision.md
// section 3.5 -- position/rotation are carried separately via
// ReconstructedBody::worldTransform, not baked here) triangle data from
// every submesh (shared and dedicated vertex data alike). Standard
// Ogre lock-and-walk; see e.g. the long-standing "Intermediate Tutorial 3:
// Retrieving The Mesh V2" community recipe this follows the same shape as.
bool extractTriangles(Ogre::Mesh* mesh, const Ogre::Vector3& scale,
                      std::vector<btVector3>& outVerts, std::vector<int>& outIndices) {
    bool sharedAppended = false;
    size_t sharedBase = 0;

    for (unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i) {
        Ogre::SubMesh* submesh = mesh->getSubMesh(i);
        Ogre::VertexData* vertex_data = submesh->useSharedVertices ? mesh->sharedVertexData : submesh->vertexData;

        if (!vertex_data) continue;

        size_t vertexBase;
        bool needToAppendVertices;
        if (submesh->useSharedVertices) {
            vertexBase = sharedAppended ? sharedBase : outVerts.size();
            needToAppendVertices = !sharedAppended;
            if (!sharedAppended) { sharedAppended = true; sharedBase = vertexBase; }
        } else {
            vertexBase = outVerts.size();
            needToAppendVertices = true;
        }

        if (needToAppendVertices) {
            const Ogre::VertexElement* posElem = vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
            if (!posElem) continue;

            Ogre::HardwareVertexBufferSharedPtr vbuf = vertex_data->vertexBufferBinding->getBuffer(posElem->getSource());
            if (!vbuf) continue;

            unsigned char* vertexStart = static_cast<unsigned char*>(
                vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
            if (!vertexStart) {
                log("mesh readback FAILED: vertex buffer lock returned null (unshadowed buffer? see phase-b-collision.md section 3.6) for " + mesh->getName());
                return false;
            }

            for (size_t v = 0; v < vertex_data->vertexCount; ++v) {
                float* pReal = nullptr;
                posElem->baseVertexPointerToElement(vertexStart + v * vbuf->getVertexSize(), &pReal);
                Ogre::Vector3 pos(pReal[0], pReal[1], pReal[2]);
                pos *= scale; // bake scale into vertex data (see comment above)
                outVerts.push_back(btVector3(pos.x, pos.y, pos.z));
            }
            vbuf->unlock();
        }

        Ogre::IndexData* index_data = submesh->indexData;
        if (!index_data || index_data->indexCount == 0) continue;
        Ogre::HardwareIndexBufferSharedPtr ibuf = index_data->indexBuffer;
        if (!ibuf) continue;

        bool use32 = (ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);
        void* iStart = ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY);
        if (!iStart) {
            log("mesh readback FAILED: index buffer lock returned null for " + mesh->getName());
            return false;
        }

        if (use32) {
            uint32_t* p = static_cast<uint32_t*>(iStart);
            for (size_t t = 0; t < index_data->indexCount; ++t)
                outIndices.push_back((int)(vertexBase + p[t]));
        } else {
            uint16_t* p = static_cast<uint16_t*>(iStart);
            for (size_t t = 0; t < index_data->indexCount; ++t)
                outIndices.push_back((int)(vertexBase + p[t]));
        }
        ibuf->unlock();
    }

    return !outVerts.empty() && !outIndices.empty();
}

btCollisionShape* buildStaticShape(const std::vector<btVector3>& verts, const std::vector<int>& indices) {
    auto* triMesh = new btTriangleMesh();
    for (size_t t = 0; t + 2 < indices.size(); t += 3) {
        triMesh->addTriangle(verts[indices[t]], verts[indices[t + 1]], verts[indices[t + 2]]);
    }
    // useQuantizedAabbCompression=true: same BVH-backed shape the shim's own
    // hkpMeshShape already wraps (see havok_compat::hkpMeshShape) -- built
    // directly here (rather than via hkpMeshShape) since ReconstructedBody
    // carries a raw btCollisionShape*, wrapped later by
    // hkpHavokSnapshot::buildNamedBody() in HavokAll.h.
    return new btBvhTriangleMeshShape(triMesh, true);
}

// Convex hull + btShapeHull decimation, per docs/porting/phase-b-collision.md
// section 3.5 step 1-2 (the auto-fitted-capsule fallback in step 3 is left
// as a future config knob -- not wired here; see the report).
btCollisionShape* buildHullShape(const std::vector<btVector3>& verts, const std::string& debugName) {
    auto* rawHull = new btConvexHullShape();
    for (auto& v : verts) rawHull->addPoint(v, false);
    rawHull->recalcLocalAabb();

    btShapeHull* hull = new btShapeHull(rawHull);
    hull->buildHull(rawHull->getMargin());

    auto* decimated = new btConvexHullShape(
        (const btScalar*)hull->getVertexPointer(), hull->numVertices());

    btVector3 rawMin, rawMax, hullMin, hullMax;
    btTransform ident; ident.setIdentity();
    rawHull->getAabb(ident, rawMin, rawMax);
    decimated->getAabb(ident, hullMin, hullMax);
    std::ostringstream oss;
    oss << "hovercraft hull '" << debugName << "': raw mesh AABB extents ("
        << (rawMax - rawMin).x() << ", " << (rawMax - rawMin).y() << ", " << (rawMax - rawMin).z()
        << "), decimated hull (" << hull->numVertices() << " verts) AABB extents ("
        << (hullMax - hullMin).x() << ", " << (hullMax - hullMin).y() << ", " << (hullMax - hullMin).z() << ")";
    log(oss.str());

    delete hull;
    delete rawHull;
    return decimated;
}

} // anonymous namespace

// Actual implementation, wrapped by build() below in a catch-all -- see that
// wrapper's comment for why this matters a great deal more here than in
// typical Ogre code.
bool buildImpl(const std::string& hkxPath, std::vector<havok_compat::ReconstructedBody>& out) {
    log("build() called for '" + hkxPath + "'");
    std::string stem = stemOf(hkxPath);
    std::string sceneResourceName, sceneGroup;
    if (!findSceneResource(stem, sceneResourceName, sceneGroup)) {
        log("build FAILED: no .scene resource found matching '" + stem + "' (from hkxPath '" + hkxPath + "')");
        return false;
    }

    Ogre::DataStreamPtr stream;
    try {
        stream = Ogre::ResourceGroupManager::getSingleton().openResource(sceneResourceName, sceneGroup);
    } catch (const Ogre::Exception& e) {
        log("build FAILED: could not open '" + sceneResourceName + "' in group '" + sceneGroup + "': " + e.getFullDescription());
        return false;
    }
    std::string xmlText = stream->getAsString();

    ParsedScene parsed;
    if (!parseSceneXml(xmlText, parsed)) {
        log("build FAILED: TinyXML could not parse '" + sceneResourceName + "'");
        return false;
    }

    bool isHovercraft = toLower(hkxPath).find("hovercraft") != std::string::npos;

    log("parsed '" + sceneResourceName + "': " + std::to_string(parsed.meshNodes.size()) +
        " mesh-bearing node(s), " + std::to_string(parsed.excludedNames.size()) +
        " trigger-excluded name(s), kind=" + (isHovercraft ? "hovercraft" : "level"));

    for (const auto& mn : parsed.meshNodes) {
        if (!isHovercraft && parsed.excludedNames.count(mn.name)) {
            log("  excluding trigger-shaped node '" + mn.name + "' (mesh " + mn.meshFile + ") from collision -- see phase-b-collision.md section 3.4");
            continue;
        }

        Ogre::Mesh* mesh = nullptr;
        try {
            mesh = loadMeshDirect(sceneGroup, mn.meshFile); // shadowed vertex+index buffers, see section 3.6
        } catch (const Ogre::Exception& e) {
            log("  mesh load FAILED for '" + mn.name + "' (" + mn.meshFile + "): " + e.getFullDescription());
            continue;
        }
        if (!mesh) {
            log("  mesh load FAILED for '" + mn.name + "' (" + mn.meshFile + "): openResource returned no stream");
            continue;
        }

        std::vector<btVector3> verts;
        std::vector<int> indices;
        bool readOk = extractTriangles(mesh, mn.worldScale, verts, indices);
        delete mesh; // done with it either way -- see loadMeshDirect's comment on why this is safe/exclusive
        if (!readOk) {
            log("  mesh readback FAILED for '" + mn.name + "' (" + mn.meshFile + ") -- see section 3.6");
            continue;
        }

        havok_compat::ReconstructedBody rb;
        rb.name = mn.name;
        rb.worldTransform.setIdentity();
        rb.worldTransform.setOrigin(btVector3(mn.worldPos.x, mn.worldPos.y, mn.worldPos.z));
        rb.worldTransform.setRotation(btQuaternion(mn.worldRot.x, mn.worldRot.y, mn.worldRot.z, mn.worldRot.w));

        if (isHovercraft) {
            rb.isStatic = false;
            rb.shape = buildHullShape(verts, mn.name);
        } else {
            rb.isStatic = true;
            rb.shape = buildStaticShape(verts, indices);
        }
        out.push_back(rb);

        log("  built body '" + mn.name + "' from " + mn.meshFile + " (" + std::to_string(verts.size()) +
            " verts, " + std::to_string(indices.size() / 3) + " tris" + (isHovercraft ? ", convex hull" : ", BVH triangle mesh") + ")");
    }

    log("build finished for '" + hkxPath + "': " + std::to_string(out.size()) + " body(ies) reconstructed");
    return true;
}

bool OgreCollisionProvider::build(const std::string& hkxPath, std::vector<havok_compat::ReconstructedBody>& out) {
    // This runs on HavokThread.cpp's dedicated physics thread, synchronously
    // inside AbstractHavokWorld::load() -- and any exception that escapes it
    // is caught by HavokThread.cpp's runHavok(), which reports it via a
    // MODAL MessageBox (MB_TASKMODAL) that nothing in an unattended/headless
    // test run will ever click, permanently deadlocking both the server and
    // any connected client (see docs/porting/phase-b-collision.md's
    // verification section and HavokThread.cpp's catch handlers). Ogre
    // itself throws Ogre::Exception liberally (missing resource, bad mesh,
    // etc.), so this outermost catch-all is not optional defensive style --
    // it is what keeps a single bad track/mesh from hanging the whole
    // harness instead of just failing that one body with a loud log line.
    try {
        return buildImpl(hkxPath, out);
    } catch (const Ogre::Exception& e) {
        log("build FAILED with Ogre::Exception for '" + hkxPath + "': " + e.getFullDescription());
    } catch (const std::exception& e) {
        log(std::string("build FAILED with std::exception for '") + hkxPath + "': " + e.what());
    } catch (...) {
        log("build FAILED with an unknown exception for '" + hkxPath + "'");
    }
    out.clear();
    return false;
}

void registerDefaultProvider() {
    // The dedicated server process this runs in never selects a
    // RenderSystem (HUDedicatedServer::run() never calls
    // Ogre::Root::initialise() -- there is no render window to create), so
    // Ogre::HardwareBufferManager::getSingleton() -- normally created by
    // whichever RenderSystem gets initialised -- never exists there.
    // Ogre::Mesh's vertex/index buffer creation always goes through that
    // singleton (Mesh::getHardwareBufferManager()), so deserializing ANY
    // mesh (this provider's core job) segfaults on a null singleton without
    // it -- confirmed empirically via hu_collision_smoketest. Ogre ships
    // exactly this scenario's answer: a plain-RAM, RenderSystem-independent
    // HardwareBufferManager, for headless mesh loading/manipulation.
    // Guarded on getSingletonPtr() so this is a no-op in single-player,
    // where the client half of the process shares this same Ogre::Root and
    // already has a real, GPU-backed one from its own RenderSystem --
    // constructing a second one would fight over Ogre::Singleton's
    // exactly-one-instance contract.
    if (!Ogre::HardwareBufferManager::getSingletonPtr()) {
        new Ogre::DefaultHardwareBufferManager(); // process-lifetime, self-registering singleton
    }

    static OgreCollisionProvider provider; // process lifetime, non-owning registration
    havok_compat::setCollisionProvider(&provider);
}

}
