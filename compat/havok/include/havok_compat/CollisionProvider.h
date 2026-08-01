// CollisionProvider.h -- Phase B hook: lets an Ogre-aware module (hu_collision)
// supply real Bullet collision geometry to this Havok shim without the shim
// itself depending on Ogre. See docs/porting/phase-b-collision.md section 3.1
// for the full design rationale.
//
// compat/havok deliberately has zero Ogre dependency (it's a physics shim,
// not a scene loader). hkpHavokSnapshot::load() is, however, the one place
// that needs to turn a ".hkx path" into real collision shapes -- and doing
// that requires reading the matching .scene + .mesh files, which is Ogre's
// job. This header is the seam: hu_collision implements CollisionProvider
// and registers exactly one instance at game startup (see
// HUDedicatedServer::run(), the single game-source change this workstream
// makes); HavokAll.h's hkpHavokSnapshot::load() asks for it by calling
// collisionProvider() and, if one is registered, calls build() instead of
// logging the Phase-A "not implemented" stub.
//
// If no provider is ever registered (e.g. a unit test linking only
// hu_havok_compat), behavior is exactly Phase A's: hkpHavokSnapshot::load()
// logs once and returns an empty hkpPhysicsData. That keeps this shim
// standalone and unit-testable, per the design doc.
#pragma once

#include <string>
#include <vector>

class btCollisionShape;
class btTransform;

// btTransform's default constructor leaves it uninitialized (real Bullet
// behavior) -- ReconstructedBody below always has its worldTransform field
// explicitly assigned by the provider before use, never read uninitialized.
#include <LinearMath/btTransform.h>

namespace havok_compat {

// One reconstructed rigid body, named identically to whatever
// hkpPhysicsData::findRigidBodyByName() will be asked to look up (the
// .scene node name -- see phase-b-collision.md section 3.4/4).
struct ReconstructedBody {
    std::string name;
    // Owned by the provider's caller once returned from build() -- ownership
    // transfers to the hkpShape wrapper hkpHavokSnapshot::load() constructs
    // around it (see HavokAll.cpp), which deletes it in ~hkpShape().
    btCollisionShape* shape = nullptr;
    // Carries position + rotation only; any non-uniform scale from the
    // .scene node hierarchy is baked directly into the shape's vertex data
    // by the provider (see phase-b-collision.md section 3.5) rather than
    // carried here, since btTransform cannot represent scale.
    btTransform worldTransform = btTransform::getIdentity();
    // true for static level/asteroid geometry (mass 0, btBvhTriangleMeshShape);
    // false is currently unused (the hovercraft hull is also constructed as
    // a mass-0 snapshot body here -- HavokHovercraft::load only ever copies
    // its *shape* back out, it never adds this particular body to a world,
    // so the mass value on this snapshot body itself is inert either way).
    // Kept as a field (not inferred from shape type) so a future provider
    // can distinguish without the shim needing to guess from RTTI.
    bool isStatic = true;
};

// Implemented by hu_collision (Ogre + TinyXML), which knows how to resolve
// a ".hkx" path to its paired ".scene" and rebuild collision geometry from
// the render meshes it describes. See docs/porting/phase-b-collision.md.
class CollisionProvider {
public:
    virtual ~CollisionProvider() = default;

    // hkxPath is exactly what AbstractHavokWorld::load()/HavokHovercraft::load()
    // handed to hkpHavokSnapshot::load(), e.g. ".\\levels\\junkyard.hkx" or
    // ".\\hovercraft\\hover1.hkx" -- see phase-b-collision.md section 3.2 for
    // how the provider maps that to a .scene file.
    // Returns false (and leaves `out` untouched) on any failure to resolve
    // or parse the scene; hkpHavokSnapshot::load() falls back to logging the
    // Phase-A "not implemented" stub in that case so a miss is never silent.
    virtual bool build(const std::string& hkxPath, std::vector<ReconstructedBody>& out) = 0;
};

// Non-owning: the registered provider must outlive every hkpHavokSnapshot::load()
// call that might use it (in practice, the lifetime of the whole process --
// see HUDedicatedServer::run(), the one call site that registers this).
void setCollisionProvider(CollisionProvider* provider);
CollisionProvider* collisionProvider();

}
