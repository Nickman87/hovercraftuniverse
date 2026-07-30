// HavokAll.cpp -- out-of-line implementation for the Havok->Bullet shim.
// See compat/havok/include/havok_compat/HavokAll.h for the class bodies and
// docs/porting/havok-compat.md for the design rationale.
#include "havok_compat/HavokAll.h"
#include "havok_compat/CollisionProvider.h"

#include <set>
#include <unordered_set>
#include <mutex>

#include <BulletCollision/NarrowPhaseCollision/btGjkEpaPenetrationDepthSolver.h>
#include <BulletCollision/NarrowPhaseCollision/btVoronoiSimplexSolver.h>

namespace havok_compat {

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

    if (!phantom->m_ghost) {
        auto* ghost = new btPairCachingGhostObject();
        phantom->m_ghost = ghost;
        // hkpAabbPhantom / hkpShapePhantom set the actual shape/transform
        // right after construction in their own ctors/setAabb/setTransform;
        // give the ghost a small placeholder box shape until then so it's
        // never null inside Bullet's broadphase.
        btCollisionShape* shape = phantom->m_collidable && phantom->m_collidable->m_shape
            ? phantom->m_collidable->m_shape->bullet()
            : nullptr;
        if (!shape) shape = new btBoxShape(btVector3(0.001f, 0.001f, 0.001f));
        ghost->setCollisionShape(shape);
        ghost->setCollisionFlags(ghost->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
        ghost->setUserPointer(phantom);

        if (auto* aabbPhantom = dynamic_cast<hkpAabbPhantom*>(phantom)) {
            aabbPhantom->setAabb(aabbPhantom->getAabb());
        } else if (auto* shapePhantom = dynamic_cast<hkpShapePhantom*>(phantom)) {
            shapePhantom->setTransform(shapePhantom->getTransform());
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

void hkpWorld::_updatePhantomOverlaps() {
    for (auto* phantom : m_phantoms) {
        if (!phantom->m_ghost) continue;
        std::vector<hkpCollidable*> nowOverlapping;
        int n = phantom->m_ghost->getNumOverlappingObjects();
        for (int i = 0; i < n; ++i) {
            btCollisionObject* obj = phantom->m_ghost->getOverlappingObject(i);
            btRigidBody* rb = btRigidBody::upcast(obj);
            if (!rb) continue; // phantoms only report overlaps with rigid bodies, matching game usage
            auto* entity = static_cast<hkpEntity*>(rb->getUserPointer());
            if (!entity) continue;
            nowOverlapping.push_back(entity->getCollidable());
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
    for (auto* a : actionsCopy) a->applyAction(stepInfo);
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

    m_rigidBody->attachBullet(body, info.m_shape);
}

void hkpCharacterRigidBody::checkSupport(const hkStepInfo&, hkpSurfaceInfo& surfaceInfo) {
    surfaceInfo.m_supported = false;
    hkpWorld* world = m_rigidBody->getWorld();
    if (!world) return;

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
// hkpCollisionDispatcher::GenericGetClosestPoints -- generic GJK closest
// point query between any two convex collidables. Used by
// PlanetGravityAction to find the surface direction from a satellite to the
// planet. See docs/porting/havok-compat.md for why this ignores the
// (shapeTypeA, shapeTypeB) dispatch table real Havok uses.
// ---------------------------------------------------------------------------

void hkpCollisionDispatcher::GenericGetClosestPoints(const hkpCollidable& a, const hkpCollidable& b, const hkpCollisionInput& input, hkpClosestCdPointCollector& collector) {
    collector.m_hasHit = false;

    auto* shapeA = dynamic_cast<btConvexShape*>(a.getShape() ? a.getShape()->bullet() : nullptr);
    auto* shapeB = dynamic_cast<btConvexShape*>(b.getShape() ? b.getShape()->bullet() : nullptr);
    if (!shapeA || !shapeB) return;

    hkpRigidBody* rbA = hkGetRigidBody(&a);
    hkpRigidBody* rbB = hkGetRigidBody(&b);
    btTransform xa = rbA ? rbA->m_bulletBody->getWorldTransform() : btTransform::getIdentity();
    btTransform xb = rbB ? rbB->m_bulletBody->getWorldTransform() : btTransform::getIdentity();

    btGjkEpaPenetrationDepthSolver epaSolver;
    btVoronoiSimplexSolver simplexSolver;
    btGjkPairDetector detector(shapeA, shapeB, &simplexSolver, &epaSolver);

    btGjkPairDetector::ClosestPointInput cpInput;
    cpInput.m_transformA = xa;
    cpInput.m_transformB = xb;
    cpInput.m_maximumDistanceSquared = (input.m_tolerance * input.m_tolerance) + 1e6f; // effectively unbounded; tolerance only grows the caller's retry loop

    btPointCollector result;
    detector.getClosestPoints(cpInput, result, nullptr);

    if (result.m_hasResult) {
        collector.m_hasHit = true;
        collector.m_contact.m_position = hkVector4(result.m_pointInWorld);
        collector.m_contact.m_normal = hkVector4(result.m_normalOnBInWorld);
        collector.m_contact.m_distance = result.m_distance;
    }
}
