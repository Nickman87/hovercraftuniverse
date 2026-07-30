// smoketest.cpp -- standalone verification tool for the Phase B collision
// provider, independent of the full game harness (ZoidCom/RaceState/Lobby).
// See docs/porting/phase-b-collision.md's verification section: step 1
// ("findRigidBodyByName returns non-null for every name requested") is
// explicitly called out as independently checkable by log inspection alone,
// which is exactly what this tool does -- it drives
// hkpHavokSnapshot::load()/hkpPhysicsData::findRigidBodyByName() directly,
// without going through the game's network/lobby/race state machine at all.
//
// Not part of the shipped game; not linked into HovercraftUniverse.exe.
// Owned by this porting effort, verification-only.
#include <hu_collision/OgreCollisionProvider.h>
#include <havok_compat/CollisionProvider.h>
#include <havok_compat/HavokAll.h>

#include <OgreRoot.h>
#include <OgreConfigFile.h>
#include <OgreResourceGroupManager.h>
#include <OgreMaterialManager.h>
#include <OgreDefaultHardwareBufferManager.h>

#include <windows.h>
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    std::string dataDir = (argc > 1) ? argv[1] : "C:\\hu-modern-run\\data";
    std::vector<std::string> hkxPaths = {
        ".\\levels\\SoccerField.hkx",
        ".\\levels\\SimpleTrack2.hkx",
    };

    SetCurrentDirectory(dataDir.c_str());

    Ogre::Root* root = new Ogre::Root("", "", "smoketest_ogre.log");

    // No RenderSystem is ever selected on this headless process (mirroring
    // HUDedicatedServer::run(), which never calls Root::initialise() either
    // -- the dedicated server has no render window). Without a RenderSystem,
    // Ogre::HardwareBufferManager::getSingleton() is never created (each
    // RenderSystem registers its own GPU-backed subclass on init), so
    // Mesh::getHardwareBufferManager()->createVertexBuffer/createIndexBuffer
    // segfaults on a null singleton the instant MeshSerializer::importMesh
    // tries to build actual buffers -- confirmed empirically. Ogre ships
    // exactly this scenario's answer: a plain-RAM, RenderSystem-independent
    // HardwareBufferManager implementation for headless mesh
    // loading/manipulation. One instance registers itself as the active
    // singleton for the rest of the process.
    auto* bufferManager = new Ogre::DefaultHardwareBufferManager();

    Ogre::ConfigFile cf;
    cf.load("resources.cfg");
    Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
    while (seci.hasMoreElements()) {
        Ogre::String secName = seci.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap* settings = seci.getNext();
        for (auto& it : *settings) {
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(it.second, it.first, secName);
        }
    }
    // Deliberately mirroring HUDedicatedServer::run() exactly here (not just
    // "close enough"): no initialiseAllResourceGroups() call, same as the
    // real dedicated server -- resource groups get lazily initialised on
    // first access. An earlier version of this tool called
    // initialiseAllResourceGroups() explicitly, which parses ALL scripts
    // (materials/programs) up front; without a render system that crashes
    // in Ogre::TextureManager (an assert on a null singleton, since no
    // RenderSystem ever registers one) -- a divergence from the real
    // server's setup, not a bug in the collision provider itself.
    Ogre::ResourceGroupManager::getSingleton()._unregisterScriptLoader(Ogre::MaterialManager::getSingletonPtr());

    hu_collision::registerDefaultProvider();
    havok_compat::CollisionProvider* provider = havok_compat::collisionProvider();
    if (!provider) {
        std::cerr << "[smoketest] FAILED: no provider registered" << std::endl;
        return 1;
    }

    int overallFailures = 0;
    for (auto& hkxPath : hkxPaths) {
        std::cout << "\n[smoketest] ===== build(" << hkxPath << ") =====" << std::endl;
        std::vector<havok_compat::ReconstructedBody> bodies;
        bool ok = provider->build(hkxPath, bodies);
        std::cout << "[smoketest] build() returned " << (ok ? "true" : "false")
                  << ", " << bodies.size() << " body(ies)" << std::endl;
        for (auto& b : bodies) {
            std::cout << "[smoketest]   body: '" << b.name << "' shape=" << (b.shape ? "non-null" : "NULL")
                      << " isStatic=" << b.isStatic << std::endl;
        }
        if (!ok || bodies.empty()) overallFailures++;
    }

    // Second pass: drive the ACTUAL call path the game uses --
    // hkIstream/hkpHavokSnapshot::load()/hkpPhysicsData::findRigidBodyByName()
    // -- rather than calling the provider directly, so this also exercises
    // HavokAll.h's buildNamedBody()/m_namedBodies wiring, not just
    // OgreCollisionProvider::build() in isolation. Matches exactly what
    // HoverCraftUniverseWorld::createAsteroid()/createStaticBody() look up
    // for SoccerField (HoverCraftUniverseWorld.cpp): Asteroid nodes use
    // getOgreEntity() (here, "FootballPlanet"), StaticBody nodes likewise
    // (here, "Wall"/"Ball"/"Goal01"/"Goal02"/"Blue01"/"Red01").
    std::cout << "\n[smoketest] ===== hkpHavokSnapshot::load()/findRigidBodyByName() on SoccerField =====" << std::endl;
    std::vector<std::string> soccerFieldNames = {
        "FootballPlanet", "Wall", "Ball", "Goal01", "Goal02", "Blue01", "Red01"
    };
    {
        hkIstream infile(".\\levels\\SoccerField.hkx");
        std::cout << "[smoketest] hkIstream.isOk()=" << infile.isOk() << std::endl;
        hkPackfileReader::AllocatedData* loadedData = nullptr;
        hkpPhysicsData* physicsData = hkpHavokSnapshot::load(infile.getStreamReader(), &loadedData);
        std::cout << "[smoketest] hkpHavokSnapshot::load() returned " << (physicsData ? "non-null" : "NULL") << std::endl;
        for (auto& name : soccerFieldNames) {
            hkpRigidBody* body = physicsData->findRigidBodyByName(name.c_str());
            std::cout << "[smoketest]   findRigidBodyByName('" << name << "') = "
                      << (body ? "non-null" : "NULL") << std::endl;
            if (!body) overallFailures++;
        }
    }

    std::cout << "\n[smoketest] " << (overallFailures == 0 ? "ALL OK" : "FAILURES PRESENT") << std::endl;
    delete root;
    delete bufferManager;
    return overallFailures == 0 ? 0 : 1;
}
