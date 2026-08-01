# Phase B — Collision Geometry Reconstruction

**Workstream D of [phase-b-plan.md](phase-b-plan.md).**
Goal: the hovercraft rests on the track instead of falling through it, and can be driven.

---

## 1. The problem

The game's collision geometry lives in 9 `.hkx` files — Havok's proprietary serialized
packfile format:

| File | Size |
|------|------|
| `data/levels/Junkyard.hkx` | 3.62 MB |
| `data/levels/Bavaraf.hkx` | 2.07 MB |
| `data/levels/SimpleTrack2.hkx` | 1.59 MB |
| `data/levels/Ramp.hkx` | 359 KB |
| `data/levels/SoccerField.hkx` | 157 KB |
| `data/hovercraft/HoverJeep.hkx` | 1.32 MB |
| `data/hovercraft/hover1.hkx` | 172 KB |
| `data/hovercraft/HippyCraft.hkx` | 172 KB |
| `data/hovercraft/USAHovercraft.hkx` | 172 KB |

Havok 6.6 is gone and we are not writing a `.hkx` parser. In the shim, the loader is a stub:

```cpp
// compat/havok/include/havok_compat/HavokAll.h:1012
static hkpPhysicsData* load(hkStreamReader*, hkPackfileReader::AllocatedData** out) {
    havok_compat::todoPhaseBOnce("hkpHavokSnapshot::load");
    ...  // returns an empty hkpPhysicsData
}
```

so `findRigidBodyByName` always returns `nullptr`, and every level-geometry code path is dead.

## 2. Why reconstruction is viable

Three facts, established by reading the data and the source:

**The game only ever asks for bodies by name.** `findRigidBodyByName` is called from exactly
three sites — `createAsteroid`, `createStaticBody`
([HoverCraftUniverseWorld.cpp:182-230](../../HovercraftUniverse/HovercraftUniverse/HoverCraftUniverseWorld.cpp))
and `HavokHovercraft::load`
([HavokHovercraft.cpp:398-419](../../HovercraftUniverse/HovercraftUniverse/HavokHovercraft.cpp)).
Nothing iterates the `.hkx`, queries its shapes, or depends on its internal structure.

**That name is a `.scene` node name.** It comes from the `<OgreEntity>` tag inside an
`<externals>` item's `userData`, and the same string names the scene node that carries the
render mesh:

```xml
<node name="Asteroid01">
  <entity name="Asteroid01" meshFile="Asteroid01.mesh"/>
  <position x="..." y="..." z="..."/><rotation .../><scale .../>
</node>
...
<externals>
  <item name="Asteroid01_ent">
    <userData><![CDATA[<Asteroid><OgreEntity>Asteroid01</OgreEntity>...]]></userData>
  </item>
</externals>
```

So render mesh and missing collision body are joined by a shared key, and the `.scene`
carries the transform.

**No separate collision meshes ever shipped.** No `_collision` / `_phys` / `_col` /
`_lowpoly` naming exists anywhere in the asset tree; every mesh name maps 1:1 to a scene node.
This was a single-LOD pipeline — the original Havok shapes were baked from these same render
meshes by a content pipeline that no longer exists. Rebuilding from the render mesh is
therefore *the same input the original used*, just a different baker.

**And the trigger volumes need nothing.** Start, Finish, Checkpoint, Portal and SpeedBoost are
constructed in C++ as `hkpAabbPhantom` / `hkpBoxShape` from `.scene` transforms
([HoverCraftUniverseWorld.cpp:33-80](../../HovercraftUniverse/HovercraftUniverse/HoverCraftUniverseWorld.cpp))
and never touch the `.hkx` at all. They already work today.

## 3. Design

### 3.1 Where the code lives

`compat/havok` deliberately does not depend on Ogre, and it should stay that way — it's a
physics shim, not a scene loader. But the call site that needs scene data
(`hkpHavokSnapshot::load`) is inside it.

Resolved with a provider hook. The shim declares an interface and a registration point; a new
Ogre-aware module implements it.

```cpp
// compat/havok/include/havok_compat/CollisionProvider.h
namespace havok_compat {

struct ReconstructedBody {
    std::string   name;          // matches findRigidBodyByName's argument
    btCollisionShape* shape;     // owned by the provider
    btTransform   worldTransform;
    bool          isStatic;
};

class CollisionProvider {
public:
    virtual ~CollisionProvider() = default;
    // hkxPath is what AbstractHavokWorld::load was handed, e.g. ".\\levels\\junkyard.hkx"
    virtual bool build(const std::string& hkxPath,
                       std::vector<ReconstructedBody>& out) = 0;
};

void setCollisionProvider(CollisionProvider*);   // non-owning
CollisionProvider* collisionProvider();

}
```

`hkpHavokSnapshot::load` then asks the provider, wraps each `ReconstructedBody` in a
`hkpRigidBody`, and populates `hkpPhysicsData::m_namedBodies` — the map that
`findRigidBodyByName` already reads and that currently is never filled. `createWorld()` adds
them all to the `hkpWorld`.

If no provider is registered, behaviour is exactly today's: log once, return empty. That keeps
the shim standalone and unit-testable.

New CMake target: **`hu_collision`** — depends on Ogre, `hu_ogremax` (for TinyXML) and
`compat/havok`. Registered from the game's startup path alongside the other subsystem
initialisation.

**Net game-source changes for this workstream: one registration call.** Everything else is new
code behind an existing API.

### 3.2 Resolving the scene from the `.hkx` path

`AbstractHavokWorld::load` receives `".\\levels\\" + track.getPhysicsFileName()`, where the
filename comes from the scene's own `<PhysicsFileName>` tag. Swap the extension:
`junkyard.hkx` → `junkyard.scene`.

Note the case mismatch in the shipped data — the tag says `junkyard.hkx` but the file on disk
is `Junkyard.scene`. Harmless on Windows (case-insensitive), but the resolver should fall back
to a case-insensitive directory scan so this isn't a latent trap.

For hovercraft, `HavokHovercraft::load` is handed `".\\hovercraft\\<name>.hkx"`; the same
extension swap finds the matching mesh via the hovercraft's own scene/mesh naming.

### 3.3 Parsing the scene independently

Timing matters here. `AbstractHavokWorld::load` is called from
`ServerLoader::parseTrackUserData`, which is an OgreMax callback fired on the scene's **root**
`userData` — i.e. before any nodes have been parsed. We cannot ask the in-flight OgreMax load
what nodes exist.

So the provider does its own lightweight pass over the `.scene` XML with TinyXML (already
vendored in `OgreMax/tinyxml`), collecting for every mesh-bearing node:

- node name
- `meshFile`
- accumulated **world** transform (walk the `<node>` hierarchy, composing
  position / rotation / scale)

This is a second parse of the same file. It is a few milliseconds and it buys complete
independence from OgreMax callback ordering — worth it.

### 3.4 Which nodes get collision

Create a static body for **every** mesh-bearing node, minus an exclusion set.

Creating bodies for everything (not just the nodes the game looks up by name) matches what the
original `.hkx` almost certainly contained — a full collision world, not just three named
objects — and it means `findRigidBodyByName` succeeds for whatever the game asks.

**Exclusion rule:** skip any node whose name is referenced by an `<externals>` item of a
trigger type — `Start`, `Finish`, `Checkpoint`, `Portal`, `SpeedBoost`, `PowerupSpawn`,
`ResetSpawn`, `StartPosition`. Those already have phantom volumes built in C++, and making
them solid would put an invisible wall across the start line.

This rule is not cosmetic. `Bavaraf` ships `Checkpoint0Geom.mesh`, `StartGeom.mesh` and
`PlanetGeom.mesh` — geometry that is trigger-shaped by name and goes through the ordinary
render-entity pipeline. Without the exclusion they become solid obstacles.

### 3.5 Shape selection

| Object kind | Shape | Rationale |
|-------------|-------|-----------|
| Track surfaces, walls, props (`StaticBody`) | `btBvhTriangleMeshShape`, mass 0 | Static concave geometry; BVH gives cheap broad rejection |
| Asteroids / planets (`Asteroid`) | `btBvhTriangleMeshShape`, mass 0 | Static in this game — they carry a gravity phantom, they don't move |
| Hovercraft hull | `btConvexHullShape`, decimated | Feeds `hkpCharacterRigidBody`, which needs a convex shape |

Scale is baked into the triangle data rather than carried on the shape. `btScaledBvhTriangleMeshShape`
exists and avoids duplicating mesh data for repeated props, but the game reuses relatively few
meshes at differing scales; start simple, revisit if load time or memory says otherwise.

The hovercraft hull is the risky one. A convex hull of a full-detail visual hull can be spiky
and oversized, and a character body that catches on geometry ruins the game. Plan:

1. `btConvexHullShape` from the mesh vertices, then `btShapeHull` decimation.
2. Compare the result's extents against the mesh AABB and log both.
3. Keep an auto-fitted **capsule** fallback behind a config switch — the original code already
   constructs a throwaway `hkpSphereShape(5)` before swapping in the `.hkx` shape
   ([HavokHovercraft.cpp:422](../../HovercraftUniverse/HovercraftUniverse/HavokHovercraft.cpp)),
   so a primitive is not obviously wrong for this game's character controller.

### 3.6 Reading mesh data back — the one real technical risk

Ogre uploads mesh buffers to the GPU and by default they are `HBU_STATIC_WRITE_ONLY` with no
shadow buffer, so `lock(HBL_READ_ONLY)` will not give us the vertices. This is the single
thing most likely to sink the naive implementation.

Use the `MeshManager::load` overload that requests shadowed buffers explicitly:

```cpp
Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().load(
    meshFile, group,
    Ogre::HardwareBuffer::HBU_STATIC,   // vertex usage
    Ogre::HardwareBuffer::HBU_STATIC,   // index usage
    true,                                // vertex buffers shadowed
    true);                               // index buffers shadowed
```

Two follow-on concerns to verify during implementation:

- **Load order.** If the render path already loaded the mesh with default (unshadowed)
  buffers, `load()` returns that existing instance and our flags are ignored. On the
  dedicated server this shouldn't arise — `ServerLoader` handles gameplay, not rendering — but
  in single-player, client and server share one process. Mitigation: load collision meshes
  into a **separate, dedicated resource group** so we get our own instance, and unload it
  after the shapes are built.
- **Memory.** Shadowed buffers double the footprint for the duration of the build. Unload the
  collision group as soon as the Bullet shapes exist; Bullet holds its own copy of the
  triangle data.

## 4. Implementation steps

1. `CollisionProvider` interface + registration in `compat/havok`; wire
   `hkpHavokSnapshot::load` and `hkpPhysicsData::m_namedBodies` to it. Behaviour unchanged
   when no provider is set.
2. New `hu_collision` target. TinyXML pass over a `.scene` producing
   `(nodeName, meshFile, worldTransform)` plus the trigger-exclusion set. **Verify against
   `SoccerField.scene` and `Junkyard.scene` by logging the collected set before building any
   shapes.**
3. Mesh readback into `btTriangleMesh`, in a dedicated resource group with shadowed buffers.
   Prove readback works on one mesh before going wide.
4. `btBvhTriangleMeshShape` construction for statics; register bodies; verify
   `findRigidBodyByName("Asteroid01")` returns non-null on `Junkyard`.
5. Hovercraft `btConvexHullShape` + decimation + extent logging.
6. Register the provider from game startup.
7. Runtime validation — see below.

Steps 1–5 are independently verifiable without reaching a race, which matters because
workstream C gates actual racing. Step 2 in particular is checkable by log inspection alone.

## 5. Acceptance criteria

- `findRigidBodyByName` returns a non-null body for every name the game requests, on all 5
  tracks. Any miss is logged loudly with the name and the scene it was sought in.
- The hovercraft rests on the track surface at its start position instead of falling.
- A full lap is drivable on `SoccerField` (smallest) and `Junkyard` (largest, 3.6 MB of
  original collision data — the stress case).
- Trigger volumes still fire: crossing the start line and each checkpoint registers, and no
  invisible wall exists where a `*Geom` mesh sits.
- Physics step cost is measured and recorded, so workstream E has a baseline.

## 6. What this will not reproduce

Stated plainly because it is a real divergence from the original, not a bug to be fixed later:

- **Contact behaviour will differ.** Bullet's BVH triangle mesh is not Havok's MOPP. Seam
  handling in particular is a known Bullet weak spot — a craft can snag on the shared edge
  between two triangles in ways it would not have in Havok. Mitigation belongs in workstream E
  (contact-processing threshold, collision margin).
- **Collision will likely be higher-poly than the original.** The shipped `.hkx` may well have
  contained simplified proxies rather than full render geometry. We cannot know, and we cannot
  recover them.
- **Anything authored only in Havok's tooling is gone.** Per-shape material properties,
  welding info, custom collision filters — if the original artists set any of it in the Havok
  content pipeline rather than in the `.scene`, it did not survive. Notably
  `hkpGroupFilterSetup::setupCollisionFilter` is already a no-op in the shim because no group
  rules exist anywhere in the game's data, which is weak evidence that little was authored
  there.

This is reconstruction, not reproduction. The measure of success is that it plays right —
validated against `local-game/` in workstream E — not that it matches byte-for-byte.
