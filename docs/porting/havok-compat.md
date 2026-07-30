# Havok Physics 6.6 compatibility shim (Phase A)

## Why this exists

The game (`HovercraftUniverse/**`) is built against **Havok Physics 6.6**,
a closed-source physics SDK. The vendor program is long gone; only
prebuilt VC9 (`_MSC_VER` 1500-era) headers and libraries survive under
`HovercraftUniverse/dependencies/Havoc/`, and those cannot link against
the MSVC v143 (Win32/x86) toolchain this porting effort targets. There is
no source to port, no vendor to ask, and no newer binary to fetch. Havok
is the second and last hard blocker for the modern build (the other,
already solved, being ZoidCom -- see `docs/porting/zoidcom-compat.md`,
whose conventions this document and the shim follow).

The fix is the same shape as ZoidCom's: a **drop-in, API-compatible
replacement written from scratch**, `compat/havok/**`, owned by this
porting effort (not a port of anything under `HovercraftUniverse/`). It
matches the real Havok 6.6 headers (`HovercraftUniverse/dependencies/
Havoc/Source/`, used here strictly as an API reference) closely enough
that the game's `#include <Physics/...>` / `<Common/...>` style includes
resolve unchanged, with the same class names, method signatures and
enums, but is backed by **Bullet Physics 3.25** (vcpkg, `x86-windows`)
instead of Havok's own solver.

## Phase A scope

Get every Havok-dependent file to **compile and link**, wire real
Bullet-backed behavior wherever the mapping is clean (math, world/body/
shape creation and stepping, phantoms, actions, collision events, ray
casting, closest-points queries), and stub the rest honestly (a
`// TODO(phaseB):` comment plus a runtime log line -- see
`havok_compat::todoPhaseBOnce()` in `compat/havok/src/HavokAll.cpp`,
mirroring `zshim::todoPhaseBOnce()` from the ZoidCom shim). It is
explicitly **not** Phase A's job to reproduce Havok's exact numerical
behavior (solver iterations, damping curves, character-controller spring
dynamics) or to parse `.hkx` files -- see the dedicated sections below.

## Shim structure

Unlike ZoidCom's shim (one class per real header, ~1:1), this one uses a
single umbrella implementation header/source pair,
`compat/havok/include/havok_compat/HavokAll.h` +
`compat/havok/src/HavokAll.cpp`, plus ~47 one-line **thin forwarding
headers** under `compat/havok/include/<RealHavokPath>` that each just do
`#include "havok_compat/HavokAll.h"`. This was a deliberate simplification
given how many tiny headers real Havok splits its API across (a single
`hkpAabbPhantom` needs its own path, its base `hkpPhantom`'s own path,
etc.) -- centralizing the actual class bodies avoids keeping ~47 files in
sync by hand, at the cost of the shim's internal file layout not mirroring
Havok's own (its *external*, game-visible `#include` paths still do,
which is what actually matters for the "drop-in" property). Three `.cxx`
files the game directly `#include`s as source
(`Common/Base/keycode.cxx`, `Common/Compat/hkCompat_None.cxx`,
`Common/Serialize/Util/hkBuiltinTypeRegistry.cxx`) are empty stubs --
real Havok's versions do license-keycode checks and reflection/
compatibility registration that have no equivalent need in this shim.

## Havok -> Bullet mapping design

| Havok concept | Bullet concept | Notes |
|---|---|---|
| `hkVector4`/`hkQuaternion`/`hkRotation`/`hkTransform` | `btVector3`/`btQuaternion`/`btMatrix3x3`/`btTransform` | Real, thin wrappers. `hkVector4` carries a separate `hkReal m_w` since Havok's SIMD vector has a real 4th lane the game reads/writes (`v(3)`), which `btVector3` doesn't have. |
| `hkpWorld` | `btDiscreteDynamicsWorld` (+ `btDbvtBroadphase`, `btSequentialImpulseConstraintSolver`, `btCollisionDispatcher`, a `btGhostPairCallback`) | One Bullet world per Havok world. `markForWrite`/`unmarkForWrite`/`registerWithJobQueue` are no-ops (see threading note below). |
| `hkpRigidBody` | `btRigidBody` | `hkpRigidBody` owns/wraps a `btRigidBody*`; `hkpShape` owns/wraps a `btCollisionShape*`. |
| `hkpBoxShape`/`hkpSphereShape`/`hkpCapsuleShape` | `btBoxShape`/`btSphereShape`/`btCapsuleShape` | Real. Capsule note: Havok's capsule is an arbitrary line segment (`vertexA`, `vertexB`) + radius; Bullet's `btCapsuleShape` is always Y-axis-aligned through its local origin. The shim approximates by using the segment length as Bullet's half-height -- correct for the axis-aligned probe capsules this game actually constructs (`AdvancedTest`'s forward-facing collision probe in `HavokHovercraft.cpp`), not a general fix for an arbitrarily-oriented/offset capsule. |
| `hkpConvexVerticesShape` / `hkpMeshShape` / `hkpBvTreeShape` | `btConvexHullShape` / `btBvhTriangleMeshShape` | Declared and real, but **unused by any current game code path** -- provided as a working target for whoever implements the Phase B mesh-from-scene loader (see below). |
| `hkpPhantom` family (`hkpAabbPhantom`, `hkpShapePhantom`/`hkpSimpleShapePhantom`) | `btPairCachingGhostObject` | **This is the mapping called out in the task brief.** Each phantom owns a ghost object added to the dynamics world with `CF_NO_CONTACT_RESPONSE` + the `SensorTrigger` collision group. Each `hkpWorld::stepMultithreaded()` call, `hkpWorld::_updatePhantomOverlaps()` reads each ghost's current `getOverlappingObjects()` list, diffs it against the previous step's set, and calls the phantom's virtual `addOverlappingCollidable()`/`removeOverlappingCollidable()` for what changed -- reproducing Havok's overlap-listener behavior exactly as the game's `CheckpointPhantom`/`FinishPhantom`/`PortalPhantom`/`PowerupPhantom`/`SpeedBoostPhantom`/`StartPhantom`/`PlanetGravityPhantom` subclasses expect. |
| `hkpAction`/`hkpUnaryAction` | *(no direct Bullet equivalent wired)* | **Also called out in the brief, with a deliberate deviation.** Bullet has `btActionInterface::updateAction()`, registered per-object on the dynamics world. This shim does **not** use it: instead, `hkpWorld` keeps its own `std::vector<hkpAction*>` and calls `applyAction(stepInfo)` on each, once per step, right after `m_bullet->stepSimulation()`. Behaviorally equivalent for this game (every action here is a `hkpUnaryAction` bound to exactly one entity, exactly what the game already assumes) and much simpler than adapting Havok's `hkpAction`/`clone()`/multi-entity/multi-phantom action model onto Bullet's differently-shaped interface. `HoverAction`, `BoostAction`, `PlanetGravityAction`, `UpdatePositionAction`, `PhantomTrackAction`, `HavokHovercraftCollisionEffect`, and the two `EntityCollisionPrevention.cpp` binder actions all work unmodified against this. |
| `hkpCollisionListener` + contact events | Bullet's persistent manifolds (`btCollisionDispatcher::getManifoldByIndexInternal`) | Real: after each step, `hkpWorld::_dispatchCollisionEvents()` walks every manifold with contacts, resolves both sides back to their owning `hkpEntity` via `btRigidBody::getUserPointer()`, and fires `contactProcessCallback()` on any registered listener -- this is what `HavokHovercraftCollisionEffect`'s bump effect rides on. `contactPointAdded/Confirmed/RemovedCallback` are declared (for API completeness) but never fired -- no current game listener overrides them (only `contactProcessCallback` is overridden, in `HovercraftCollisionListener`). |
| `hkpSimpleWorldRayCaster` / `hkpWorldRayCastInput`/`Output` | `btCollisionWorld::rayTestSingle` | Real. `HoverAction`'s hover-height probe (`PlanetRayCastCallback`) subclasses this to filter for planet-type entities before accepting a hit; the base `addBroadPhaseHandle()` does a genuine single-shape raycast via Bullet. |
| `hkpCollisionDispatcher::getGetClosestPointsFunc()` + `hkpClosestCdPointCollector` | `btGjkPairDetector` (GJK + EPA) | Real, but **generic**: real Havok dispatches by `(shapeTypeA, shapeTypeB)` into a large function-pointer table (box-vs-sphere, sphere-vs-capsule, ...); this shim ignores both shape-type arguments and always returns one GJK-based closest-points implementation that works for any pair of convex shapes. `PlanetGravityAction`'s gravity-direction calculation (satellite vs. planet shape) is the only caller, and both shapes involved are always convex, so this is behaviorally sufficient here -- it would not be for concave/mesh-vs-mesh queries, which this game never performs through this path. |
| `hkpCharacterRigidBody` + state machine (`hkpCharacterContext`, `hkpCharacterStateOnGround/InAir/Jumping/Climbing`) | A `btRigidBody` owned directly by `hkpCharacterRigidBody`, plus a from-scratch simplified state update | **Not a faithful port -- see the dedicated section below.** |
| `hkJobQueue`/`hkCpuJobThreadPool`/`hkSpuJobThreadPool` | *(none -- single-threaded)* | `// TODO(phaseB)`. See threading note below. |
| `hkVisualDebugger`/`hkpPhysicsContext` | *(none)* | `// TODO(phaseB)` no-ops. Dev-time-only tooling; no viewer exists to connect to post-port, and the game never queries these for gameplay state. |
| `hkpHavokSnapshot::load()` / `.hkx` loading | *(none)* | `// TODO(phaseB)`. See the dedicated `.hkx` section below -- the single biggest Phase A gap. |

### Threading

Real Havok fans physics work out across `hkJobQueue` + `hkCpuJobThreadPool`
(and optionally SPU job threads, irrelevant on PC). The game only ever
calls `hkpWorld::stepMultithreaded(jobQueue, threadPool, dt)` and never
inspects either argument's internals, so this shim accepts and stores
them as inert handles and always steps Bullet single-threaded
(`btDiscreteDynamicsWorld::stepSimulation`) on the calling thread (the
dedicated Havok thread the game already spins up in `HavokThread.cpp`).
For a 30 Hz hovercraft-racing game (`data/engine_settings.cfg`:
`[Havok] Framerate=30`) on modern hardware this is not a performance
concern; it is, however, not a faithful reproduction of Havok's job-based
scheduling, and any code that assumed multithreaded determinism/ordering
guarantees from Havok's scheduler would behave differently here. No such
assumption was found in the game code.

### Character controller -- simplified, not ported

Real Havok's `hkpCharacterRigidBody` + state machine implements
spring-based ground/air velocity projection, slope handling, and step
climbing, tuned via each state's `setSpeed`/`setGain`/
`setMaxLinearAcceleration`/`setDisableHorizontalProjection`. None of that
has a clean Bullet equivalent (Bullet's own `btKinematicCharacterController`
is a *kinematic*, not rigid-body-driven, controller with different
semantics entirely), so rather than stub it out completely (which would
leave the hovercraft unable to move under its own control at all -- too
severe for Phase A, since movement is the core gameplay loop), this shim
implements a **simplified reimplementation**:

- `hkpCharacterRigidBody` owns a real `hkpRigidBody`/`btRigidBody`
  (dynamic, angular factor zeroed since the game fully controls
  orientation itself in `HavokHovercraft::update()`, `DISABLE_DEACTIVATION`
  so it never sleeps mid-race).
- `checkSupport()` does a genuine downward raycast (3 units along
  `m_up`) against the owning `hkpWorld` via Bullet, and reports
  `m_supported`/`m_surfaceNormal`/`m_surfaceVelocity` from the hit.
- `hkpCharacterContext::update()` picks `HK_CHARACTER_ON_GROUND` or
  `HK_CHARACTER_IN_AIR` based on that support result (real Havok's state
  transitions are considerably more involved, including a dedicated
  jumping state this shim's `update()` never actually selects -- `
  HK_CHARACTER_JUMPING`/`HK_CHARACTER_CLIMBING` states are registered and
  constructible for API completeness, since the game does register them,
  but are otherwise inert here), then exponentially drives velocity from
  the current velocity toward `forward * inputUD * speed`, clamped by
  `maxLinearAcceleration * dt`, plus the state's configured gravity along
  `-up`.

This is real, working, tunable behavior (the hovercraft accelerates,
brakes, and turns under the same `Height`/`CharacterGravity`/`TurnAngle`/
`Damping` knobs from `data/engine_settings.cfg` the game already reads),
but it will **feel** different from real Havok -- see "gameplay-feel
divergences" below.

## `.hkx` collision data -- Phase B

This is the single biggest Phase A stub. Level and hovercraft collision
geometry ships as proprietary Havok `.hkx` files:

| File | Paired `.scene` | Used by |
|---|---|---|
| `data/levels/Bavaraf.hkx` | `Bavaraf.scene` | Bavaraf track |
| `data/levels/Junkyard.hkx` | `Junkyard.scene` | Junkyard track |
| `data/levels/Ramp.hkx` | `Ramp.scene` | Ramp track |
| `data/levels/SimpleTrack2.hkx` | `SimpleTrack2.scene` | SimpleTrack2 track |
| `data/levels/SoccerField.hkx` | `SoccerField.scene` | SoccerField track |
| `data/hovercraft/HippyCraft.hkx` | `HippyCraft.scene` | HippyCraft vehicle |
| `data/hovercraft/hover1.hkx` | `hover1.scene` | hover1 vehicle |
| `data/hovercraft/HoverJeep.hkx` | `HoverJeep.scene` | HoverJeep vehicle |
| `data/hovercraft/USAHovercraft.hkx` | `USAHovercraft.scene` | USAHovercraft vehicle |

`AbstractHavokWorld::load()` reads a level `.hkx` via
`hkpHavokSnapshot::load()`; `HavokHovercraft::load()` reads a hovercraft
`.hkx` the same way. Both go through this shim's
`hkpHavokSnapshot::load()`, which **does not parse the binary format at
all** -- it logs
`TODO(phaseB): .hkx snapshot loading is not implemented (path: ...)`
once per distinct file path and returns an empty `hkpPhysicsData` (whose
`createWorld()` still returns a valid, empty `hkpWorld`, and whose
`findRigidBodyByName()` always misses, logging its own
`TODO(phaseB)` once per distinct name). Net effect right now: the game
boots, the physics world exists and steps, but **no static collision
geometry or named rigid bodies exist** -- planets/track surfaces/
hovercraft hulls found by name (`createAsteroid`, `createStaticBody`,
`HavokHovercraft::load`'s `findRigidBodyByName`) will all currently
throw `ParseException` / hit `HK_NULL`. This is called out explicitly so
it reads as a known, documented gap rather than a silent one.

**Recommended Phase B approach: do not reverse-engineer the `.hkx`
binary format.** Havok's tagfile/packfile serialization format is
complex, versioned, and undocumented at the binary level in the surviving
headers (only the C++ object model is documented, not the wire format).
Instead, since every `.hkx` has a matching `.scene` (loaded by
`CustomOgreMaxScene`/OgreMax for rendering) that already describes the
same geometry's meshes, transforms and names:

1. Extend the OgreMax scene loader (or a small Phase-B-only utility next
   to it) to, for each named mesh object in the `.scene`, pull its
   `Ogre::Mesh` vertex/index data (after OgreMax has loaded it for
   rendering anyway) and build a `btTriangleMesh` from it.
2. Wrap that in this shim's already-real `hkpMeshShape`
   (`btBvhTriangleMeshShape` under the hood -- see the mapping table
   above) and construct a static (`MOTION_FIXED`) `hkpRigidBody` at the
   node's transform, named identically to what `findRigidBodyByName()`
   is asked to look up (matching the names `HoverCraftUniverseWorld.cpp`
   already searches for: the asteroid/static-body's `getOgreEntity()`
   string, and the hovercraft's `mEntityName`).
3. Populate `hkpPhysicsData::m_namedBodies` with the result so
   `findRigidBodyByName()` starts finding real geometry, and give
   `hkpPhysicsData::createWorld()` a way to add all of them as entities
   at world-creation time (mirroring what real Havok's snapshot loader
   would have done).
4. For the hovercraft hull specifically (`HavokHovercraft::load()` pulls
   `body->getCollidable()->getShape()` off the loaded rigid body and
   reuses it as-is for the character rigid body), a convex hull
   (`hkpConvexVerticesShape`/`btConvexHullShape`, also already real in
   this shim) computed from the hovercraft mesh's vertices is likely a
   better fit than a full triangle mesh (character controllers generally
   want convex collision volumes).

This sidesteps the undocumented `.hkx` format entirely and reuses
geometry the game already has to load for rendering, at the cost of only
approximating whatever simplified/decimated collision proxy the original
`.hkx` actually contained (the render mesh may be higher-poly than the
original hand-authored Havok collision shapes) -- an acceptable tradeoff
for Phase B, and something to revisit per-track if a particular level's
collision feels too expensive or too different from the original.

## Build status

- **`hu_havok_compat`** (`compat/havok/CMakeLists.txt`, links Bullet
  3.25 via `find_package(Bullet CONFIG)`): builds clean, 0 errors.
- **`hu_game_physics`** (`modern/CMakeLists.txt`): builds clean, 0
  errors, ~2m50s from a clean rebuild. **71 of 91** `.cpp` files under
  `HovercraftUniverse/HovercraftUniverse/` compile into it; **20** are
  excluded, in two categories:

  **(A) GUI / Sound / Scripting-tainted (13 files)** -- transitively
  `#include` `GUIManager.h`, `SoundManager.h`, or a Scripting header
  (`lua.h`, `luabind.hpp`, `OgreLuaBindings.h`), none of which are ported
  yet: `ClientPreparationLoader.cpp`, `HUApplication.cpp`, `HUClient.cpp`,
  `HUD.cpp`, `HovercraftAIController.cpp`, `HovercraftRepresentation.cpp`,
  `InGameState.cpp`, `LobbyGUI.cpp`, `LobbyState.cpp`, `MainMenu.cpp`,
  `MainMenuState.cpp`, `ServerLoader.cpp`, `main.cpp` (the last is the
  WinMain entry point itself, GUI/Sound-tainted directly) -- plus, found
  only once actual compilation was attempted (their taint is via an
  angle-bracket `#include <...>` or an unflagged intermediate header, not
  caught by the same-style grep-based scan used for the initial split):
  `ClientLoader.cpp` and `HovercraftLoader.cpp` (both pull in
  `HovercraftRepresentation.h` -> `<Moveable3DEmitter.h>`, which only
  exists under `HovercraftUniverse/Sound/`) and `ClientConnectThread.cpp`
  (pulls in `"ConnectListener.h"`, which only exists under
  `HovercraftUniverse/GUI/`).

  **(B) Genuine, pre-existing, non-Havok bugs (4 files)** -- confirmed by
  actually attempting to compile each against `hu_havok_compat` (same
  "document rather than hack" rule the ZoidCom porting pass established
  for its own RaceCamera/GameView/EntityPropertySystem findings):
  - `PortalData.cpp` -- `#include`s `"PortalData.h"`, which does not
    exist anywhere in the repository.
  - `Portal.cpp` -- uses the Ogre 1.x `Ogre::vector<Ogre::String>::type`
    + `Ogre::StringUtil::split()` idiom, removed in Ogre 14.5.2 (an
    ogre-api-gap issue, same class as `docs/porting/ogre-api-gap.md`,
    just not caught by that document's original pass because this file
    was Havok-blocked at the time).
  - `PathRecorder.cpp`, `HUServerThread.cpp` -- both call
    `boost::xtime`/`boost::TIME_UTC` (removed from modern Boost) to pace
    a sleep loop; a boost-version-api-gap issue, unrelated to Havok.
    (`HavokThread.cpp`'s own physics-step loop uses plain Win32 `Sleep()`
    instead and is unaffected.)

  All 20 excluded files remain on disk, untouched, ready to be re-added
  once their blocking project is ported or their pre-existing bug is
  fixed elsewhere.

  Two build-configuration adjustments were needed for `hu_game_physics`
  specifically (both are the same kind of "build-config difference, not
  a game-code edit" precedent `hu_ogremax`/`hu_coreengine` already
  established for `UNICODE`/`_UNICODE`):
  - `/permissive` (re-enabling non-conformant parsing, overriding the
    top-level `/permissive-`): `HovercraftUniverse/Utils/PlayerMapImpl.h`
    omits `typename` on several dependent type names, and
    `CustomOgreMaxScene.h` declares
    `Ogre::String CustomOgreMaxScene::getUniqueName();` *inside* the
    class body it belongs to (an invalid redundant qualifier) -- both
    tolerated under classic/permissive parsing, both rejected under
    strict `/permissive-` conformance. Both are genuine, pre-existing
    bugs in code this effort cannot edit.
  - `/FI windows.h` (forced include): `Havok.h`/`HavokThread.h` use
    `DWORD`/`WINAPI`/`LPVOID` without ever `#include`-ing `<windows.h>`
    themselves -- in the original VC9 `.vcproj` this worked because some
    earlier header in that project's precompiled-header chain already
    pulled it in; this target has no precompiled header, so forcing
    `<windows.h>` ahead of every translation unit reproduces the old
    implicit behavior.
  - Two vcpkg packages were missing and added:
    `boost-random` (`RaceState.h`'s Mersenne-twister RNG) and
    `boost-interprocess` (`HUClient.h`'s semaphore) -- both narrow,
    legitimate dependency gaps, fixed the same way `boost-thread` was
    added for `hu_coreengine`; not game-code edits.

## API surface: implemented vs. mapped vs. stubbed

Enumerated by grepping every `hk`-prefixed symbol across
`HovercraftUniverse/HovercraftUniverse/{AbstractHavokWorld,Havok,
HavokEntity,HavokEntityType,HavokThread,HavokHovercraft,
HavokHovercraftCollisionEffect,HoverAction,BoostAction,
PlanetGravityAction,PlanetGravityPhantom,UpdatePositionAction,
PhantomTrackAction,BasePhantom,CheckpointPhantom,FinishPhantom,
PortalPhantom,PowerupPhantom,SpeedBoostPhantom,StartPhantom,
HoverCraftUniverseWorld,StaticBody,StaticBodyEntity,CollisionEvent,
CharacterContextContainer,EntityCollisionPrevention}.{h,cpp}`, plus
`ServerLoader.cpp`'s few direct Havok call sites.

Of roughly 120 distinct Havok symbols/classes/methods this surface
touches:

- **~70 real, Bullet-backed** -- math (`hkVector4`/`hkQuaternion`/
  `hkRotation`/`hkTransform`/`hkAabb`), `hkpWorld` (creation, gravity,
  stepping, entity/phantom/action registration), `hkpRigidBody` (full
  position/rotation/velocity/force/impulse/mass surface), the box/
  sphere/capsule/convex-vertices/mesh shape family, the full
  `hkpPhantom`/`hkpAabbPhantom`/`hkpShapePhantom`/`hkpSimpleShapePhantom`
  overlap-listener surface, `hkpAction`/`hkpUnaryAction`, collision
  listener contact dispatch, ray casting, closest-points queries,
  `hkpWorldObject`'s property bag (used by `HavokEntityType`).
- **~15 simplified reimplementations, not faithful ports** -- the
  character-rigid-body state machine (see dedicated section above).
- **~35 honest `TODO(phaseB)` stubs** -- `.hkx`/`hkpHavokSnapshot`
  loading, `hkVisualDebugger`/`hkpPhysicsContext`, the job-queue/thread-
  pool classes (accepted but inert), `hkpGroupFilterSetup` (no group
  rules exist in this game's data to enforce), a handful of
  `hkpCollisionListener` callbacks the game never overrides
  (`contactPointAdded/Confirmed/RemovedCallback`).

## Gameplay-feel divergences (Havok vs. this shim)

Called out explicitly, as requested, in order of expected impact:

1. **No level/vehicle collision geometry yet (Phase B).** Until the
   Phase B loader above exists, there is nothing for anything to collide
   with -- this dwarfs every other divergence below in practice.
2. **Character control is a simplified reimplementation, not a port**
   (see above): expect noticeably different acceleration/turning feel,
   no real slope handling, no step-climbing, and no functional jump
   state (the `HK_CHARACTER_JUMPING` state exists but nothing currently
   drives a transition into it).
3. **Gravity application**: real Havok applies `hkpWorldCinfo::m_gravity`
   globally to every dynamic body every step *in addition to* whatever
   custom actions (like `PlanetGravityAction`) apply; this game's actual
   gravity comes entirely from the custom planet-gravity phantom/action
   pair, and `hkpWorldCinfo::m_gravity` defaults to a flat
   `(0, -9.8, 0)` in this shim (never actually read from the `.hkx`,
   since that's Phase B) -- currently harmless since nothing in the
   loaded-empty world falls anywhere, but will need re-checking once
   Phase B populates real geometry.
4. **Restitution/friction model**: Bullet's contact solver (Sequential
   Impulse) computes restitution/friction differently from Havok's
   solver internals; this shim never attempts to translate Havok
   friction/restitution *coefficients* into equivalent Bullet material
   values beyond passing the same numeric fields through
   (`hkpRigidBodyCinfo::m_friction`/`m_restitution` -> Bullet's
   `btRigidBody` friction/restitution), so a given numeric value will
   not necessarily *feel* the same between the two engines even where
   game data supplies it identically.
5. **Damping**: same story as friction/restitution -- numeric fields are
   passed through 1:1 where they exist, but the two solvers apply
   damping differently internally.
6. **Solver iteration count**: Bullet's `btSequentialImpulseConstraintSolver`
   uses its own default iteration count, not tuned to match Havok's
   default solver behavior; not currently exposed as a knob anywhere in
   this shim.
7. **Fixed-timestep behavior**: both engines are driven at a fixed
   30 Hz timestep from `data/engine_settings.cfg`'s `[Havok] Framerate`,
   and `hkpWorld::stepMultithreaded()` passes that same fixed `dt`
   straight through to `btDiscreteDynamicsWorld::stepSimulation(dt, 0,
   dt)` (zero substeps beyond the one given, matching the game's own
   single-step-per-tick usage) -- so timestep *cadence* matches; the two
   engines' internal integrators still differ numerically.
8. **Closest-points queries are generic, not shape-type-dispatched** (see
   the mapping table above) -- correct for convex-vs-convex (the only
   case this game exercises via `PlanetGravityAction`), not a general
   substitute for Havok's full dispatch table.

## Files

- `compat/havok/include/havok_compat/HavokAll.h` -- all class bodies.
- `compat/havok/src/HavokAll.cpp` -- out-of-line implementation
  (`hkpWorld` stepping/dispatch, `hkpCharacterRigidBody`, ray casting,
  closest-points).
- `compat/havok/include/**` -- ~47 thin forwarding headers matching every
  real Havok include path the game uses.
- `compat/havok/CMakeLists.txt` -- `hu_havok_compat` static library.
- `modern/CMakeLists.txt` -- `hu_game_physics` target (game-logic
  sources) plus the `add_subdirectory` wiring for `hu_havok_compat`.
