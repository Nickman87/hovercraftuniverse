// HavokAll.cpp -- out-of-line implementation for the Havok->Bullet shim.
// See compat/havok/include/havok_compat/HavokAll.h for the class bodies and
// docs/porting/havok-compat.md for the design rationale.
#include "havok_compat/HavokAll.h"
#include "havok_compat/CollisionProvider.h"
#include "havok_compat/PhysicsDiag.h"

#include <set>
#include <unordered_set>
#include <mutex>
#include <sstream>
#include <cmath>

#include <BulletCollision/NarrowPhaseCollision/btGjkEpaPenetrationDepthSolver.h>
#include <BulletCollision/NarrowPhaseCollision/btVoronoiSimplexSolver.h>
#include <BulletCollision/CollisionShapes/btConcaveShape.h>
#include <BulletCollision/CollisionShapes/btTriangleShape.h>
#include <BulletCollision/CollisionShapes/btTriangleCallback.h>

namespace havok_compat {

// Mirrors HovercraftUniverse\HovercraftUniverse\PlanetGravityAction.h's
// PlanetGravityAction::HK_SPHERE_ACTION_ID (0x28021978). The game itself
// scans a body's action list for this exact value to detect whether a
// PlanetGravityAction is currently attached (see
// PlanetGravityPhantom::addOverlappingCollidable/removeOverlappingCollidable
// in the game source). The shim mirrors that same scan in checkSupport()
// below, rather than including a game header from compat/ -- this constant
// is a Havok user-data convention the *game* defined, not something the shim
// owns, so it is deliberately duplicated here with this comment as the only
// link back to its source of truth.
static constexpr hkUlong kPlanetGravityActionUserData = 0x28021978;

void todoPhaseBOnce(const char* site, const std::string& msg) {
    static std::mutex mtx;
    static std::unordered_set<std::string> seen;
    std::lock_guard<std::mutex> lock(mtx);
    std::string key = site ? site : "";
    if (seen.count(key)) return;
    seen.insert(key);
    std::cerr << "[hu_havok_compat] TODO(phaseB): " << msg << std::endl;
}

// Non-owning registration point for the Phase B collision reconstruction
// provider -- see CollisionProvider.h. Deliberately a plain static pointer,
// not a singleton with lazy construction: exactly one real implementation
// (hu_collision's OgreCollisionProvider) is ever registered, once, from
// HUDedicatedServer::run() at process startup.
static CollisionProvider* g_collisionProvider = nullptr;

void setCollisionProvider(CollisionProvider* provider) {
    g_collisionProvider = provider;
}

CollisionProvider* collisionProvider() {
    return g_collisionProvider;
}

} // namespace havok_compat

// ---------------------------------------------------------------------------
// hkBaseSystem / hkThreadMemory / hkMonitorStream
// ---------------------------------------------------------------------------

void hkBaseSystem::init(hkPoolMemory*, hkThreadMemory*, hkErrorReportFunction) {
    // Real Havok wires the global allocators + error reporter here. Bullet
    // uses plain new/delete (btAlignedAlloc under the hood), so there is
    // nothing to wire up. Kept as a real, if trivial, function so the
    // call site (AbstractHavokWorld.cpp) needs no changes.
}
void hkBaseSystem::quit() {}

hkThreadMemory& hkThreadMemory::getInstance() {
    static hkThreadMemory instance(nullptr);
    return instance;
}

hkMonitorStream& hkMonitorStream::getInstance() {
    static hkMonitorStream instance;
    return instance;
}

// ---------------------------------------------------------------------------
// hkpWorld
// ---------------------------------------------------------------------------

void hkpWorld::addEntity(hkpRigidBody* body) {
    body->m_world = this;
    body->addReference();
    m_entities.push_back(body);
    m_allCollidables.push_back(body->getCollidable());
    m_bullet->addRigidBody(body->m_bulletBody);
}

void hkpWorld::removeEntity(hkpRigidBody* body) {
    m_bullet->removeRigidBody(body->m_bulletBody);
    m_entities.erase(std::remove(m_entities.begin(), m_entities.end(), body), m_entities.end());
    m_allCollidables.erase(std::remove(m_allCollidables.begin(), m_allCollidables.end(), body->getCollidable()), m_allCollidables.end());
    body->removeReference();
}

void hkpWorld::addPhantom(hkpPhantom* phantom) {
    phantom->m_world = this;
    phantom->addReference();
    m_phantoms.push_back(phantom);
    m_allCollidables.push_back(phantom->getCollidable());

    // hkpAabbPhantom is a broadphase-only query in real Havok -- see the
    // class comment on hkpAabbPhantom in HavokAll.h. It gets no ghost object
    // and is never added to the Bullet world at all: _updatePhantomOverlaps()
    // queries its AABB directly against m_broadphasePair instead, which does
    // zero narrowphase work. (Putting even a well-sized ghost box into the
    // Bullet world -- the previous approach -- made btDiscreteDynamicsWorld
    // run full box-vs-shape narrowphase between that box and everything it
    // enclosed, which is what caused the single-player hang/crash this
    // change fixes; see the task's regression writeup.)
    if (dynamic_cast<hkpAabbPhantom*>(phantom)) {
        return;
    }

    if (!phantom->m_ghost) {
        auto* ghost = new btPairCachingGhostObject();
        phantom->m_ghost = ghost;
        ghost->setCollisionFlags(ghost->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
        ghost->setUserPointer(phantom);

        // Ask the phantom for its real shape instead of guessing from
        // m_collidable->m_shape. hkpShapePhantom/hkpSimpleShapePhantom
        // (SpeedBoostPhantom, portals, ...) are the only other phantom kinds
        // reaching this point -- they carry a real trigger-volume shape and
        // keep the ghost path unchanged.
        if (auto* shapePhantom = dynamic_cast<hkpShapePhantom*>(phantom)) {
            btCollisionShape* shape = phantom->getCollidable()->getShape()
                ? phantom->getCollidable()->getShape()->bullet()
                : nullptr;
            ghost->setCollisionShape(shape ? shape : new btBoxShape(btVector3(0.001f, 0.001f, 0.001f)));
            shapePhantom->setTransform(shapePhantom->getTransform());
        } else {
            // No known phantom subclass reaches this branch in this codebase
            // (only hkpAabbPhantom -- handled above -- and
            // hkpShapePhantom/hkpSimpleShapePhantom are ever instantiated) --
            // kept as a safety net so the ghost is never null inside
            // Bullet's broadphase.
            ghost->setCollisionShape(new btBoxShape(btVector3(0.001f, 0.001f, 0.001f)));
        }
    }
    m_bullet->addCollisionObject(phantom->m_ghost, btBroadphaseProxy::SensorTrigger,
        btBroadphaseProxy::AllFilter);
}

void hkpWorld::removePhantom(hkpPhantom* phantom) {
    if (phantom->m_ghost) m_bullet->removeCollisionObject(phantom->m_ghost);
    m_phantoms.erase(std::remove(m_phantoms.begin(), m_phantoms.end(), phantom), m_phantoms.end());
    m_allCollidables.erase(std::remove(m_allCollidables.begin(), m_allCollidables.end(), phantom->getCollidable()), m_allCollidables.end());
    phantom->removeReference();
}

namespace {

// Real hkpAabbPhantom semantics: a pure broadphase AABB-vs-AABB query, no
// narrowphase. btBroadphaseInterface::aabbTest() walks the broadphase's own
// AABB tree (btDbvtBroadphase here) and calls process() once per proxy whose
// AABB overlaps -- exactly the cheap query real Havok performs, and nothing
// like the box-vs-shape narrowphase the previous ghost-object approach did.
struct AabbPhantomOverlapCallback : public btBroadphaseAabbCallback {
    std::vector<hkpCollidable*>* m_out;
    explicit AabbPhantomOverlapCallback(std::vector<hkpCollidable*>* out) : m_out(out) {}

    bool process(const btBroadphaseProxy* proxy) override {
        // A proxy's m_clientObject is the btCollisionObject* it was created
        // for (set by btCollisionWorld::addCollisionObject/addRigidBody) --
        // this mirrors what getOverlappingObject() returns on the
        // ghost-object path below, just reached via the broadphase directly.
        auto* obj = static_cast<btCollisionObject*>(proxy->m_clientObject);
        btRigidBody* rb = btRigidBody::upcast(obj);
        if (!rb) return true; // phantoms only report overlaps with rigid bodies, matching game usage (also filters out other phantoms' ghost/sensor objects)
        auto* entity = static_cast<hkpEntity*>(rb->getUserPointer());
        if (!entity) return true;
        m_out->push_back(entity->getCollidable());
        return true; // keep walking -- collect every overlapping proxy, not just the first
    }
};

} // namespace

void hkpWorld::_updatePhantomOverlaps() {
    for (auto* phantom : m_phantoms) {
        std::vector<hkpCollidable*> nowOverlapping;

        if (auto* aabbPhantom = dynamic_cast<hkpAabbPhantom*>(phantom)) {
            // Broadphase-only path: hkpAabbPhantom has no ghost object (see
            // hkpWorld::addPhantom) and contributes zero narrowphase work.
            const hkAabb& aabb = aabbPhantom->getAabb();
            AabbPhantomOverlapCallback callback(&nowOverlapping);
            m_broadphasePair->aabbTest(aabb.m_min.v, aabb.m_max.v, callback);
        } else if (phantom->m_ghost) {
            // hkpShapePhantom/hkpSimpleShapePhantom: unchanged ghost-object
            // path -- these carry a real trigger-volume shape and Bullet's
            // ghost pair cache already gives cheap, precise overlap tests
            // against it.
            int n = phantom->m_ghost->getNumOverlappingObjects();
            for (int i = 0; i < n; ++i) {
                btCollisionObject* obj = phantom->m_ghost->getOverlappingObject(i);
                btRigidBody* rb = btRigidBody::upcast(obj);
                if (!rb) continue; // phantoms only report overlaps with rigid bodies, matching game usage
                auto* entity = static_cast<hkpEntity*>(rb->getUserPointer());
                if (!entity) continue;
                nowOverlapping.push_back(entity->getCollidable());
            }
        } else {
            continue;
        }

        // Diff against last step's set, firing add/remove exactly like a
        // real hkpPhantom overlap listener would.
        for (auto* c : nowOverlapping) {
            if (std::find(phantom->m_currentOverlaps.begin(), phantom->m_currentOverlaps.end(), c) == phantom->m_currentOverlaps.end()) {
                phantom->addOverlappingCollidable(c);
            }
        }
        auto prevCopy = phantom->m_currentOverlaps;
        for (auto* c : prevCopy) {
            if (std::find(nowOverlapping.begin(), nowOverlapping.end(), c) == nowOverlapping.end()) {
                phantom->removeOverlappingCollidable(c);
            }
        }
    }
}

void hkpWorld::_dispatchCollisionEvents() {
    int numManifolds = m_dispatcher->getNumManifolds();
    for (int i = 0; i < numManifolds; ++i) {
        btPersistentManifold* manifold = m_dispatcher->getManifoldByIndexInternal(i);
        if (manifold->getNumContacts() == 0) continue;

        // Bullet's dispatcher builds a manifold for ANY overlapping pair the
        // broadphase reports, including a btPairCachingGhostObject (the
        // Bullet object backing every hkpPhantom -- see hkpWorld::addPhantom)
        // overlapping a rigid body: CF_NO_CONTACT_RESPONSE only tells the
        // constraint solver to ignore the contact, it does not stop manifold
        // creation. A blind static_cast<const btRigidBody*> here would accept
        // that ghost object's btCollisionObject* and then reinterpret its
        // setUserPointer(phantom) (an hkpPhantom*, sibling of hkpEntity under
        // hkpWorldObject, NOT the same layout) as an hkpEntity* -- silently
        // reading entA->m_collisionListeners out of whatever bytes happen to
        // sit at that offset in the real hkpPhantom object. That is exactly
        // the access violation this function is known to crash with (see
        // docs/porting/phase-b-collision.md's dispatch-crash note): the
        // "vector" decoded from misaligned hkpPhantom fields looks non-empty
        // but its data pointer is garbage.
        //
        // btRigidBody::upcast() is Bullet's own type-checked cast (it tests
        // btCollisionObject::getInternalType() before casting) and returns
        // null for a ghost object -- exactly the same guard
        // _updatePhantomOverlaps() and hkGetRigidBody() already use elsewhere
        // in this file. Phantom overlap notifications are delivered
        // separately and correctly via _updatePhantomOverlaps() /
        // hkpPhantom::addOverlappingCollidable(); nothing is lost by
        // excluding ghost objects here.
        //
        // Require BOTH sides to be real rigid bodies, not just "at least
        // one" -- real Havok's hkpContactProcessEvent is only ever raised
        // for a genuine rigid-body/rigid-body manifold, so m_collidableA and
        // m_collidableB are always valid there. Listener code written
        // against that contract (e.g. HovercraftCollisionListener::
        // contactProcessCallback in HavokHovercraftCollisionEffect.cpp,
        // which picks "the other side" and unconditionally calls
        // other->getOwner()) rightly never null-checks the non-self side.
        // Dispatching a one-sided event (one real entity, one null because
        // the other body was a ghost) would hand that code a null
        // hkpCollidable* and crash it -- a second, more subtle version of
        // the same phantom/rigid-body type confusion this function used to
        // have, just moved one call deeper. So a manifold with either side
        // not a rigid body is skipped entirely, exactly as real Havok would
        // never have produced it in the first place.
        auto* bodyA = btRigidBody::upcast(manifold->getBody0());
        auto* bodyB = btRigidBody::upcast(manifold->getBody1());
        if (!bodyA || !bodyB) {
            // Only true the first time a phantom/ghost object actually
            // shows up in a contact manifold -- make the (correct, by
            // design) drop visible rather than a silent no-op, per the
            // "don't mask a real gap invisibly" rule.
            havok_compat::todoPhaseBOnce("hkpWorld::_dispatchCollisionEvents:ghost",
                "skipping a contact manifold involving a non-rigid-body (phantom/ghost) "
                "collision object -- phantom overlaps are reported via "
                "hkpWorld::_updatePhantomOverlaps()/hkpPhantom::addOverlappingCollidable() instead");
            continue;
        }
        auto* entA = static_cast<hkpEntity*>(bodyA->getUserPointer());
        auto* entB = static_cast<hkpEntity*>(bodyB->getUserPointer());
        if (!entA && !entB) continue; // shouldn't happen -- attachBullet() always sets both

        hkpProcessCollisionData data;
        for (int c = 0; c < manifold->getNumContacts(); ++c) {
            const btManifoldPoint& pt = manifold->getContactPoint(c);
            hkpProcessCdPoint p;
            p.m_contact.m_position = hkVector4(pt.getPositionWorldOnA());
            p.m_contact.m_normal = hkVector4(pt.m_normalWorldOnB);
            p.m_contact.m_distance = pt.getDistance();
            data.m_points.push_back(p);
        }

        hkpContactProcessEvent ev;
        ev.m_collidableA = entA ? entA->getCollidable() : nullptr;
        ev.m_collidableB = entB ? entB->getCollidable() : nullptr;
        ev.m_collisionData = &data;

        if (entA) for (auto* l : entA->m_collisionListeners) l->contactProcessCallback(ev);
        if (entB) for (auto* l : entB->m_collisionListeners) l->contactProcessCallback(ev);
    }
}

void hkpWorld::stepMultithreaded(hkJobQueue*, hkJobThreadPool*, hkReal deltaTime) {
    // See the job-queue/thread-pool note in HavokAll.h: this shim always
    // steps single-threaded, ignoring the queue/pool arguments.
    m_bullet->stepSimulation(deltaTime, 0, deltaTime);

    _updatePhantomOverlaps();
    _dispatchCollisionEvents();

    hkStepInfo stepInfo;
    stepInfo.m_deltaTime = deltaTime;
    stepInfo.m_invDeltaTime = deltaTime > 0 ? 1.0f / deltaTime : 0.0f;
    // Copy: an action's applyAction() may add/remove actions from the world.
    std::vector<hkpAction*> actionsCopy = m_actions;
    for (auto* a : actionsCopy) {
        a->applyAction(stepInfo);
    }
}

// ---------------------------------------------------------------------------
// hkpPhysicsData::createWorld()
// ---------------------------------------------------------------------------

hkpWorld* hkpPhysicsData::createWorld() {
    hkpWorld* world = new hkpWorld(m_cinfo);
    // Phase B: add every reconstructed named body (see hkpHavokSnapshot::load()
    // in HavokAll.h) to the freshly-created world, mirroring what real
    // Havok's snapshot loader would have done for bodies deserialized out of
    // the .hkx -- see docs/porting/phase-b-collision.md section 3.1. A no-op
    // when m_namedBodies is empty (no CollisionProvider registered, or this
    // hkpPhysicsData belongs to a hovercraft hull snapshot -- HavokHovercraft
    // ::load() never calls createWorld() on its own hkpPhysicsData, it only
    // ever reuses the body's *shape*).
    for (auto& kv : m_namedBodies) {
        if (kv.second) world->addEntity(kv.second);
    }
    return world;
}

// ---------------------------------------------------------------------------
// hkpCharacterRigidBody
// ---------------------------------------------------------------------------

hkpCharacterRigidBody::hkpCharacterRigidBody(const hkpCharacterRigidBodyCinfo& info) {
    m_rigidBody = new hkpRigidBody();
    btVector3 inertia(0, 0, 0);
    btCollisionShape* shape = info.m_shape ? info.m_shape->bullet() : new btSphereShape(0.5f);
    if (info.m_mass > 0.0f) shape->calculateLocalInertia(info.m_mass, inertia);

    btTransform startTransform;
    startTransform.setIdentity();
    startTransform.setOrigin(info.m_position.v);
    startTransform.setRotation(info.m_rotation.q);

    auto* motionState = new btDefaultMotionState(startTransform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(info.m_mass, motionState, shape, inertia);
    rbInfo.m_friction = info.m_friction;
    rbInfo.m_linearDamping = 0.05f;
    rbInfo.m_angularDamping = 0.6f;
    auto* body = new btRigidBody(rbInfo);
    // A hovercraft character controls its own orientation directly (see
    // HavokHovercraft::update()); stop Bullet's solver from fighting that.
    body->setAngularFactor(btVector3(0, 0, 0));
    body->setActivationState(DISABLE_DEACTIVATION);

    // Exempt the character from the Bullet world's gravity.
    //
    // This game has no global "down" (see docs/porting/physics-model.md): a
    // hovercraft's gravity comes from PlanetGravityAction, aimed at whichever
    // asteroid it is near, plus Havok/CharacterGravity applied along -mUp by
    // the character state machine. Real Havok's hkpCharacterRigidBody works
    // the same way -- the world's gravity is not applied to it on top.
    //
    // The shim was letting Bullet add the world's (0,-9.8,0) as well, so every
    // craft carried a constant world-DOWNWARD pull regardless of which surface
    // it was flying over. On flat ground that just reads as "gravity feels
    // heavy"; in flight between two asteroids it bends the trajectory toward
    // world -Y instead of toward the destination asteroid, which is what a
    // human tester saw as getting "lost halfway between the asteroids".
    //
    // BT_DISABLE_WORLD_GRAVITY must be set BEFORE the body is added to the
    // world: btDiscreteDynamicsWorld::addRigidBody() overwrites a body's
    // gravity with the world's unless this flag is already present.
    body->setFlags(body->getFlags() | BT_DISABLE_WORLD_GRAVITY);
    body->setGravity(btVector3(0, 0, 0));

    // Continuous collision detection. The track is reconstructed as a
    // btBvhTriangleMeshShape (OgreCollisionProvider::buildStaticShape), which
    // is a zero-thickness surface: a hovercraft moving faster than its own
    // extents in one step can pass straight through it between discrete
    // steps, ending up underneath the level with no way back. Observed with a
    // bot at ~48 units/s dropping below the track and tumbling there, and it
    // is the same "clipping through the earth" a human tester reported.
    //
    // Real Havok's character controller does its own swept/continuous
    // handling; this is the Bullet equivalent and has no direct Havok
    // counterpart. Thresholds are derived from the character's own shape so
    // they scale with whatever hull the hovercraft happens to use: sweep once
    // the per-step motion exceeds roughly the smallest half-extent.
    if (shape) {
        btVector3 aabbMin, aabbMax;
        btTransform ident;
        ident.setIdentity();
        shape->getAabb(ident, aabbMin, aabbMax);
        const btVector3 halfExtents = (aabbMax - aabbMin) * 0.5f;
        const btScalar smallestHalfExtent =
            (std::min)(halfExtents.x(), (std::min)(halfExtents.y(), halfExtents.z()));
        if (smallestHalfExtent > 0.01f) {
            body->setCcdMotionThreshold(smallestHalfExtent);
            // Bullet's swept sphere must be smaller than the shape or the
            // sweep reports contacts before the shape actually touches.
            body->setCcdSweptSphereRadius(smallestHalfExtent * 0.5f);
        }
    }

    m_rigidBody->attachBullet(body, info.m_shape);
}

void hkpCharacterRigidBody::checkSupport(const hkStepInfo&, hkpSurfaceInfo& surfaceInfo) {
    // Character trajectory trace, off unless HU_PHYSICS_DIAG is set (see
    // PhysicsDiag.h). This runs once per character per physics step, which
    // makes it the cheapest place in the shim to observe where the server
    // thinks each hovercraft actually is -- useful for telling a physics
    // problem ("the craft really did fly into space") apart from a
    // replication/rendering one ("the server had it on the track all along").
    // Logged sparsely; the counter is per-character.
    if (havok_compat::physicsDiagEnabled() && m_rigidBody && m_rigidBody->m_bulletBody) {
        if ((++m_diagStepCounter % 60) == 0) {
            const btVector3& p = m_rigidBody->m_bulletBody->getWorldTransform().getOrigin();
            const btVector3& v = m_rigidBody->m_bulletBody->getLinearVelocity();
            char buf[256];
            snprintf(buf, sizeof(buf),
                "character[%p]: pos=(%.2f,%.2f,%.2f) vel=(%.2f,%.2f,%.2f) |v|=%.2f up=(%.3f,%.3f,%.3f)",
                (void*)this, p.x(), p.y(), p.z(), v.x(), v.y(), v.z(), v.length(),
                m_up.v.x(), m_up.v.y(), m_up.v.z());
            havok_compat::diagLog(buf);
        }
    }

    surfaceInfo.m_supported = false;

    // Planet gravity supersedes character gravity (docs/porting/
    // physics-model.md section 4): a craft in a PlanetGravityAction's
    // overlap already receives mass * asteroidGravity * dir as a force every
    // step. Stacking Havok/CharacterGravity on top of that measurably made
    // SimpleTrack2's asteroid-to-asteroid jump unmakeable (bot furthest
    // progress: x ~ -190 with both sources vs. x = -876 with only planet
    // gravity). Detected the same way the game itself detects it --
    // scanning the body's actions for PlanetGravityAction's well-known
    // user-data tag -- rather than duplicating any planet-tracking state.
    surfaceInfo.m_planetGravityActive = false;
    const int numActions = m_rigidBody->getNumActions();
    for (int i = 0; i < numActions; ++i) {
        hkpAction* action = m_rigidBody->getAction(i);
        if (action && action->getUserData() == havok_compat::kPlanetGravityActionUserData) {
            surfaceInfo.m_planetGravityActive = true;
            break;
        }
    }

    hkpWorld* world = m_rigidBody->getWorld();
    if (!world) {
        return;
    }

    hkVector4 pos = m_rigidBody->getPosition();
    btVector3 from = pos.v;
    btVector3 to = from - m_up.v * 3.0f; // probe 3 units "down" relative to current up

    btCollisionWorld::ClosestRayResultCallback cb(from, to);
    world->m_bullet->rayTest(from, to, cb);
    if (cb.hasHit()) {
        surfaceInfo.m_supported = true;
        surfaceInfo.m_surfaceNormal = hkVector4(cb.m_hitNormalWorld);
        auto* rb = const_cast<btRigidBody*>(btRigidBody::upcast(cb.m_collisionObject));
        surfaceInfo.m_surfaceVelocity = rb ? hkVector4(rb->getLinearVelocity()) : hkVector4(0, 0, 0);
    }
}

// ---------------------------------------------------------------------------
// hkpSimpleWorldRayCaster -- default addBroadPhaseHandle(): a genuine
// raycast against the one collidable's Bullet shape, at its current world
// transform (rigid bodies only; phantoms have no meaningful "hit").
// ---------------------------------------------------------------------------

hkReal hkpSimpleWorldRayCaster::addBroadPhaseHandle(const hkpBroadPhaseHandle* handle, int /*castIndex*/) {
    hkpCollidable* col = handle->m_collidable;
    hkpRigidBody* rb = hkGetRigidBody(col);
    if (!rb || !rb->m_bulletBody || !rb->m_bulletBody->getCollisionShape()) return 1.0f;

    btCollisionWorld::ClosestRayResultCallback cb(m_input.m_from.v, m_input.m_to.v);
    // Single-shape raycast: build a tiny transient collision world query via
    // Bullet's low-level convex-cast utility against just this one object's
    // world transform + shape (rather than re-querying the whole broadphase,
    // since the caller is already iterating the broadphase one handle at a
    // time).
    btTransform xform = rb->m_bulletBody->getWorldTransform();
    btCollisionObject tmpObj;
    tmpObj.setCollisionShape(rb->m_bulletBody->getCollisionShape());
    tmpObj.setWorldTransform(xform);
    btCollisionWorld::rayTestSingle(
        btTransform(btQuaternion::getIdentity(), m_input.m_from.v),
        btTransform(btQuaternion::getIdentity(), m_input.m_to.v),
        &tmpObj, tmpObj.getCollisionShape(), xform, cb);

    if (cb.hasHit()) {
        m_output->m_hit = true;
        m_output->m_hitFraction = cb.m_closestHitFraction;
        m_output->m_normal = hkVector4(cb.m_hitNormalWorld);
        return 0.0f; // stop -- matches Havok convention of returning <=0 to end the cast
    }
    return 1.0f;
}

// ---------------------------------------------------------------------------
// Convex-vs-concave closest-point support for GenericGetClosestPoints below.
// The planet/track is reconstructed as a btBvhTriangleMeshShape (concave --
// see OgreCollisionProvider::buildStaticShape), so the plain GJK
// convex-vs-convex call the game relies on (PlanetGravityAction's gravity/
// up-vector query) would otherwise always report "no hit" against it.
//
// processAllTriangles() walks the shape's BVH and only visits triangles
// overlapping the AABB we pass in, so restricting that AABB to the convex
// side's world AABB (expanded by the query tolerance) keeps this cheap. The
// triangle count cap below is a second, independent bound for pathological
// AABBs/meshes: the callback still gets invoked by the BVH walk, but skips
// the actual GJK work once the cap is hit.
// ---------------------------------------------------------------------------

namespace {

// True iff every component of v is finite (not NaN/Inf) and v is long
// enough to be a meaningful direction. A degenerate GJK/EPA result (which
// does happen -- coplanar/degenerate triangles, exact vertex touches) can
// hand back a zero or NaN normal; letting that win the "best triangle"
// comparison would propagate into a NaN up-vector and fling the craft.
static bool isUsableNormal(const btVector3& n) {
    if (!std::isfinite((float)n.x()) || !std::isfinite((float)n.y()) || !std::isfinite((float)n.z())) {
        return false;
    }
    return n.length2() > btScalar(1e-12);
}

// Force `normal` (located at `contactPoint`) to point toward `convexCentre`
// -- the convex/craft rigid body's centre of mass -- instead of trusting
// whichever sign btGjkPairDetector/EPA happened to produce. GJK's separating
// -axis sign is only well-defined for cleanly separated shapes; the moment
// two shapes touch or interpenetrate (exactly the case for a hovercraft
// resting on or clipping into the track) the query falls through to EPA and
// the sign can flip per triangle, per step.
//
// `convexIsArgA` says whether the convex/craft body is the caller's "a"
// argument. The collector's stored normal must keep meaning "normal on b,
// pointing toward a" (see PlanetGravityAction's contract, quoted in
// GenericGetClosestPoints below) -- and since the contact point sits right
// on the boundary between the two bodies, "toward the convex body" and
// "toward a" are the same vector exactly when the convex body IS a, and
// exact opposites when the convex body is b (a is then the concave/planet
// side, and there's no single "centre" of a triangle mesh to aim at instead
// -- but we don't need one, because the opposite-of-toward-convex vector
// already points the right way). So: orient toward convex, then flip once
// more if the convex side is b.
static void orientNormalTowardConvex(btVector3& normal, const btVector3& contactPoint,
    const btVector3& convexCentre, bool convexIsArgA) {
    const btVector3 towardConvex = convexCentre - contactPoint;
    const btScalar d = normal.dot(towardConvex);
    const bool pointsTowardConvex = d >= btScalar(0);
    if (pointsTowardConvex != convexIsArgA) {
        normal = -normal;
    }
}

struct ConcaveClosestPointCallback : public btTriangleCallback {
    btConvexShape* m_convex;
    btTransform m_convexTransform;
    btTransform m_concaveTransform;
    btScalar m_maximumDistanceSquared;
    int m_maxTriangles;
    // True if, in the caller's (a, b) query, the convex side is "a" and this
    // triangle proxy stands in for "b" -- i.e. the pair should be evaluated
    // as detector(convex, triangle) so the result's m_normalOnBInWorld means
    // "normal on b, in world", exactly like the convex-vs-convex path below.
    // False mirrors the opposite assignment (concave is "a", convex is "b").
    bool m_convexIsArgA;
    // The convex/craft rigid body's centre of mass in world space --
    // btRigidBody::getWorldTransform()'s translation IS the centre-of-mass
    // frame origin for a Bullet rigid body, so this needs no extra AABB
    // computation; see orientNormalTowardConvex above.
    btVector3 m_convexCentre;
    int m_triangleCount = 0;
    bool m_hasResult = false;
    btScalar m_bestDistance = SIMD_INFINITY;
    btVector3 m_bestPointInWorld = btVector3(0, 0, 0);
    btVector3 m_bestNormalOnB = btVector3(0, 1, 0);

    ConcaveClosestPointCallback(btConvexShape* convex, const btTransform& convexTransform,
        const btTransform& concaveTransform, btScalar maximumDistanceSquared, int maxTriangles, bool convexIsArgA)
        : m_convex(convex), m_convexTransform(convexTransform), m_concaveTransform(concaveTransform),
          m_maximumDistanceSquared(maximumDistanceSquared), m_maxTriangles(maxTriangles), m_convexIsArgA(convexIsArgA),
          m_convexCentre(convexTransform.getOrigin()) {}

    void processTriangle(btVector3* triangle, int /*partId*/, int /*triangleIndex*/) override {
        if (m_triangleCount >= m_maxTriangles) return;
        ++m_triangleCount;

        // Triangle vertices arrive in the concave shape's local space; a
        // btTriangleShape wraps them as-is, so whichever detector slot it
        // occupies must use m_concaveTransform to place it in world.
        btTriangleShape triShape(triangle[0], triangle[1], triangle[2]);

        btVoronoiSimplexSolver simplexSolver;
        btGjkEpaPenetrationDepthSolver epaSolver;
        btGjkPairDetector::ClosestPointInput cpInput;
        cpInput.m_maximumDistanceSquared = m_maximumDistanceSquared;
        btPointCollector result;

        if (m_convexIsArgA) {
            btGjkPairDetector detector(m_convex, &triShape, &simplexSolver, &epaSolver);
            cpInput.m_transformA = m_convexTransform;
            cpInput.m_transformB = m_concaveTransform;
            detector.getClosestPoints(cpInput, result, nullptr);
        } else {
            btGjkPairDetector detector(&triShape, m_convex, &simplexSolver, &epaSolver);
            cpInput.m_transformA = m_concaveTransform;
            cpInput.m_transformB = m_convexTransform;
            detector.getClosestPoints(cpInput, result, nullptr);
        }

        if (!result.m_hasResult) {
            return;
        }

        // Orient explicitly before either guarding or comparing -- a
        // degenerate normal is degenerate regardless of sign, but we want
        // the stored/reported normal (once it does win) to already be in
        // the game's expected convention.
        btVector3 normal = result.m_normalOnBInWorld;
        orientNormalTowardConvex(normal, result.m_pointInWorld, m_convexCentre, m_convexIsArgA);

        if (!isUsableNormal(normal)) {
            return; // degenerate (NaN/zero-length) -- never let this win "best"
        }

        // Smallest ABSOLUTE distance, not smallest signed distance: the
        // signed distance is negative while penetrating, so comparing
        // signed values prefers the most-deeply-penetrated triangle, which
        // for a body resting on a mesh can be one that's nowhere near
        // directly underneath it. We want the nearest surface, whether the
        // craft is above it or has clipped slightly into it.
        if (btFabs(result.m_distance) < btFabs(m_bestDistance)) {
            m_bestDistance = result.m_distance; // keep the signed value in the report
            m_bestPointInWorld = result.m_pointInWorld;
            m_bestNormalOnB = normal;
            m_hasResult = true;
        }
    }
};

} // namespace

// ---------------------------------------------------------------------------
// hkpCollisionDispatcher::GenericGetClosestPoints -- generic GJK closest
// point query between any two collidables, one of which may be concave.
// Used by PlanetGravityAction to find the surface direction from a satellite
// to the planet. See docs/porting/havok-compat.md for why this ignores the
// (shapeTypeA, shapeTypeB) dispatch table real Havok uses.
// ---------------------------------------------------------------------------

void hkpCollisionDispatcher::GenericGetClosestPoints(const hkpCollidable& a, const hkpCollidable& b, const hkpCollisionInput& input, hkpClosestCdPointCollector& collector) {
    collector.m_hasHit = false;

    btCollisionShape* bulletA = a.getShape() ? a.getShape()->bullet() : nullptr;
    btCollisionShape* bulletB = b.getShape() ? b.getShape()->bullet() : nullptr;

    // Classify with Bullet's own isConvex()/isConcave() rather than
    // dynamic_cast. Both are non-virtual inline tests on btCollisionShape's
    // m_shapeType tag, which is exactly what Bullet provides them for -- and
    // unlike dynamic_cast they need no RTTI in Bullet's vtables. Whether a
    // prebuilt Bullet exposes RTTI at all is outside this project's control
    // (vcpkg's port sets its own flags), and this function -- the only place
    // in the shim that ever cast a Bullet type -- was dead code until the
    // hkpAabbPhantom ghost fix made PlanetGravityAction reachable, so the
    // hazard had never been exercised.
    auto* shapeA = (bulletA && bulletA->isConvex()) ? static_cast<btConvexShape*>(bulletA) : nullptr;
    auto* shapeB = (bulletB && bulletB->isConvex()) ? static_cast<btConvexShape*>(bulletB) : nullptr;
    auto* concaveA = (bulletA && bulletA->isConcave()) ? static_cast<btConcaveShape*>(bulletA) : nullptr;
    auto* concaveB = (bulletB && bulletB->isConcave()) ? static_cast<btConcaveShape*>(bulletB) : nullptr;

    bool castFailed = !((shapeA || concaveA) && (shapeB || concaveB));
    if (castFailed) {
        return;
    }

    hkpRigidBody* rbA = hkGetRigidBody(&a);
    hkpRigidBody* rbB = hkGetRigidBody(&b);
    btTransform xa = rbA ? rbA->m_bulletBody->getWorldTransform() : btTransform::getIdentity();
    btTransform xb = rbB ? rbB->m_bulletBody->getWorldTransform() : btTransform::getIdentity();

    // Effectively unbounded from Bullet's point of view -- input.m_tolerance
    // is enforced explicitly below, once, on whichever path ran.
    const btScalar maximumDistanceSquared = (input.m_tolerance * input.m_tolerance) + 1e6f;

    bool hasResult = false;
    btVector3 bestPoint(0, 0, 0);
    btVector3 bestNormalOnB(0, 1, 0);
    btScalar bestDistance = SIMD_INFINITY;
    int trianglesExamined = 0; // stays 0 on the convex-vs-convex path

    if (shapeA && shapeB) {
        // Both convex: the original GJK path, unchanged in behaviour.
        btGjkEpaPenetrationDepthSolver epaSolver;
        btVoronoiSimplexSolver simplexSolver;
        btGjkPairDetector detector(shapeA, shapeB, &simplexSolver, &epaSolver);

        btGjkPairDetector::ClosestPointInput cpInput;
        cpInput.m_transformA = xa;
        cpInput.m_transformB = xb;
        cpInput.m_maximumDistanceSquared = maximumDistanceSquared;

        btPointCollector result;
        detector.getClosestPoints(cpInput, result, nullptr);

        if (result.m_hasResult) {
            btVector3 normal = result.m_normalOnBInWorld;
            // Both sides are convex here, so there's no shape-based way to
            // pick out "the craft" the way the concave branch does -- but
            // every real caller (PlanetGravityAction) always passes the
            // craft as argument a, and the contract this function must
            // honour is "normal on b, pointing toward a" regardless. So we
            // orient toward a's centre unconditionally (convexIsArgA=true
            // below is not a classification, it's just this branch's fixed
            // "orient toward a" rule), keeping this path consistent with
            // the concave one and independent of GJK/EPA's sign.
            if (isUsableNormal(normal)) {
                orientNormalTowardConvex(normal, result.m_pointInWorld, xa.getOrigin(), /*convexIsArgA=*/true);
                hasResult = true;
                bestPoint = result.m_pointInWorld;
                bestNormalOnB = normal;
                bestDistance = result.m_distance;
            }
        }
    } else {
        // One side is concave (in practice always the planet/track, a
        // btBvhTriangleMeshShape) -- walk its BVH-culled triangles within
        // (convex AABB + tolerance) and run GJK per triangle, keeping the
        // closest result. convexIsArgA tracks which of (a, b) the convex
        // shape actually is, so the callback can reproduce the same (A, B)
        // slot assignment the convex-vs-convex path above uses -- that is
        // what keeps m_normalOnBInWorld meaning "normal on b" either way,
        // with no sign flip needed regardless of which side is concave.
        bool convexIsArgA = (shapeA != nullptr);
        btConvexShape* convex = convexIsArgA ? shapeA : shapeB;
        btConcaveShape* concave = convexIsArgA ? concaveB : concaveA;
        const btTransform& convexTransform = convexIsArgA ? xa : xb;
        const btTransform& concaveTransform = convexIsArgA ? xb : xa;

        btVector3 convexAabbMin, convexAabbMax;
        convex->getAabb(convexTransform, convexAabbMin, convexAabbMax);
        btVector3 tol(input.m_tolerance, input.m_tolerance, input.m_tolerance);
        convexAabbMin -= tol;
        convexAabbMax += tol;

        // Transform the (expanded) convex AABB into the concave shape's
        // local frame by transforming its 8 corners through the concave
        // body's inverse world transform and re-enclosing them.
        btTransform concaveInverse = concaveTransform.inverse();
        btVector3 localAabbMin(SIMD_INFINITY, SIMD_INFINITY, SIMD_INFINITY);
        btVector3 localAabbMax(-SIMD_INFINITY, -SIMD_INFINITY, -SIMD_INFINITY);
        for (int i = 0; i < 8; ++i) {
            btVector3 corner(
                (i & 1) ? convexAabbMax.x() : convexAabbMin.x(),
                (i & 2) ? convexAabbMax.y() : convexAabbMin.y(),
                (i & 4) ? convexAabbMax.z() : convexAabbMin.z());
            btVector3 local = concaveInverse(corner);
            localAabbMin.setMin(local);
            localAabbMax.setMax(local);
        }

        const int kMaxTrianglesPerQuery = 4096; // bounds worst-case per-step cost against a pathological AABB/mesh
        ConcaveClosestPointCallback callback(convex, convexTransform, concaveTransform,
            maximumDistanceSquared, kMaxTrianglesPerQuery, convexIsArgA);
        concave->processAllTriangles(&callback, localAabbMin, localAabbMax);

        trianglesExamined = callback.m_triangleCount;
        if (callback.m_hasResult) {
            hasResult = true;
            bestPoint = callback.m_bestPointInWorld;
            bestNormalOnB = callback.m_bestNormalOnB;
            bestDistance = callback.m_bestDistance;
        }
    }

    // Report a hit only within tolerance -- this is what makes the caller's
    // do { ...; tolerance *= 2; } while (!hasHit()) loop
    // (PlanetGravityAction::applyAction) terminate against a real track: the
    // convex-vs-convex path above can, in principle, return an
    // arbitrarily-far closest pair (GJK doesn't refuse just because two
    // convex hulls are far apart), and the concave path's per-triangle GJK
    // calls likewise return whatever the nearest triangle in the queried
    // AABB is, which can be farther than the AABB inflation implies. Gating
    // on input.m_tolerance here means every doubling of the caller's
    // tolerance strictly grows the query volume/acceptance radius, so the
    // loop is guaranteed to hit as soon as the tolerance reaches (at most)
    // the true distance to the planet mesh -- which is always finite for a
    // craft inside PlanetGravityPhantom's AABB.
    if (hasResult && bestDistance <= input.m_tolerance) {
        collector.m_hasHit = true;
        collector.m_contact.m_position = hkVector4(bestPoint);
        collector.m_contact.m_normal = hkVector4(bestNormalOnB);
        collector.m_contact.m_distance = bestDistance;
    }

    // Diagnostic logging (see havok_compat/PhysicsDiag.h) -- no-op unless
    // HU_PHYSICS_DIAG is set. This function runs every physics step for
    // every craft inside a PlanetGravityPhantom, so logging every call would
    // flood the log; once every 60 calls (roughly once every 2 seconds at a
    // 30 Hz step) is enough to see the normal settle or start flipping.
    if (havok_compat::physicsDiagEnabled()) {
        static std::atomic<long long> callCounter{0};
        if ((++callCounter % 60) == 0) {
            std::ostringstream oss;
            oss << "GenericGetClosestPoints: trianglesExamined=" << trianglesExamined
                << " bestAbsDistance=" << (hasResult ? btFabs(bestDistance) : SIMD_INFINITY)
                << " hasHit=" << (collector.m_hasHit ? 1 : 0)
                << " normal=(" << bestNormalOnB.x() << ", " << bestNormalOnB.y() << ", " << bestNormalOnB.z() << ")";
            havok_compat::diagLog(oss.str());
        }
    }
}
