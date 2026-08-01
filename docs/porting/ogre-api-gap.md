# Ogre 1.7.0RC1 -> Ogre 14.5.2 (vcpkg) porting gap analysis

Scope: HovercraftUniverse, CoreEngine, GUI, Networking, Sound, Scripting, OgreMax, Utils,
Exceptions. Old headers referenced at `HovercraftUniverse/dependencies/OGRE/include`
(read-only). vcpkg `ogre` port inspected at `C:\Users\dirk\vcpkg\ports\ogre\`.

## Summary table

| Issue | Occurrences | Affected projects | Severity | Fix pattern |
|---|---|---|---|---|
| `Ogre::vector<T>::type` STLAllocator typedefs | 6 (4 files) | HovercraftUniverse (game) | Blocker (compile) | Replace with `std::vector<T>` etc. Mechanical, `sed`-able. |
| `Ogre::ST_GENERIC` / `SceneType` enum passed to `createSceneManager` | 2 direct + OgreMax's own `SceneType`/`ParseSceneManager` indirection | CoreEngine, OgreMax | Blocker (compile) | `createSceneManager(Ogre::ST_GENERIC, name)` -> `createSceneManager("DefaultSceneManager", name)` (string-typed API). OgreMax's `ParseSceneManager`/`SceneType` (OgreMax\OgreMaxUtilities.*) needs its own enum->string map updated, since PCZ/BSP/Octree/Terrain scene-manager type strings no longer exist in mainline Ogre. |
| Non-default plugins loaded in `plugins_*.cfg`: `Plugin_CgProgramManager`, `Plugin_PCZSceneManager`, `Plugin_OctreeZone`, `Plugin_BSPSceneManager`, `Plugin_OctreeSceneManager` | 6 lines across 3 files (`data/plugins_debug.cfg`, `data/plugins_release.cfg`, `setup.iss`) | data/, installer | Blocker (missing plugins) | None of these plugins are built by the vcpkg `ogre` port (only core + optional assimp/freeimage/overlay/zip components, no PCZSceneManager/OctreeZone/BSP/Cg plugins exist upstream anymore). No direct C++ usage of PCZ/BSP/Octree APIs was found in game code (`grep` for `PCZSceneManager\|PCZone\|BspSceneManager` = 0 hits outside `dependencies/`), so these lines are very likely dead/vestigial and can simply be deleted from the cfg files. `Plugin_CgProgramManager` **is** actually used (see Cg row below) and has no replacement — must be re-authored. |
| Cg shader program (`data/levels/materials/programs/asteroid.program`, `asteroidVS.cg`/`asteroidPS.cg`) | 1 material's vertex+fragment program (2 program defs, `profiles vs_1_1`/`ps_2_0`) | data/, HovercraftUniverse (Asteroid.cpp) | Blocker (compile/runtime — Cg plugin does not exist in modern Ogre; NVIDIA discontinued Cg SDK in 2012) | Small blast radius: only one asteroid material uses Cg. Rewrite `asteroid.program` as HLSL (`language hlsl`, D3D9/11 profiles) and/or GLSL, or drop the shader and use a fixed-function/RTSS-generated pass if the effect is simple enough. |
| Fixed-function (no `program_ref`) material passes | 13 of 16 `.material` files have zero `program_ref` (`SkyX.material` is the one exception with 12 program refs; only `asteroid.program`'s material has a Cg program) | data/ (all level/hovercraft/character materials), CoreEngine (MovableTextOverlay-generated materials) | Behavioral-risk (not a hard compile blocker, but needs RTSS) | Ogre 14 with GL3+/D3D11 render systems has **no** fixed-function pipeline; it requires `Ogre::RTShaderSystem` (RTSS) to synthesize shaders for these passes at runtime. Must instantiate `Ogre::RTShaderSystem::initialize()` + generate shaders for all resource groups, or D3D9 render system (which still has FFP, see below) must be kept as the target, or every material must be hand-converted to explicit programs. |
| `Ogre::Overlay`, `OverlayManager`, `OverlayElement`, `OverlayContainer`, custom `MovableTextOverlay` | 73 occurrences across 16 files (GUI: `GUIManager`, `OverlayManager.cpp/h`, `MouseVisualisation`, `OverlayContainer`, `ServerMenu`; HovercraftUniverse: `HUD.h`, `LobbyGUI.h`, `MainMenu.h`; CoreEngine: `MovableTextOverlay.*`, `ObjectTextDisplay.*`) | GUI, CoreEngine, HovercraftUniverse | Mechanical (build-config) + Behavioral-risk | Overlay moved to the separate **Overlay component** (`OgreOverlaySystem.h`, needs `new Ogre::OverlaySystem()` registered with the SceneManager, `#include <OgreOverlaySystem.h>`/`OgreOverlayManager.h`/`OgreOverlayContainer.h` instead of core Ogre headers). Good news: the vcpkg port builds `overlay` **by default** (`default-features: ["assimp","freeimage","overlay","zip"]`), so the component is available; code just needs the new include paths + explicit `OverlaySystem` instantiation/registration with each `SceneManager` (this didn't exist as a manual step in 1.7). |
| `Ogre::Font`, `Ogre::FontManager` | 20 occurrences across 5 files (`Application.cpp`, `ClientLoader.cpp`, `OgreMaxScene.cpp`, `OgreMaxUtilities.*`) | CoreEngine, HovercraftUniverse, OgreMax | Mechanical | Also moved into the Overlay component headers (`OgreFont.h` now lives under `Components/Overlay/include`). Same default-feature availability as above; just needs include-path/link fixes. |
| `SharedPtr` idiom: `isNull()`, `setNull()`, `getPointer()` | 14 occurrences across 5 files (`OgreMaxScene.cpp` x4, `OgreMaxModel.cpp` x4, `OgreMaxUtilities.cpp` x2, `CustomOgreMaxScene.cpp` x1, `MovableTextOverlay.cpp` x3) | OgreMax, HovercraftUniverse, CoreEngine | Blocker (compile) | Ogre 14's `SharedPtr` is now a thin wrapper around `std::shared_ptr` (or is `std::shared_ptr` directly depending on config) and does **not** have `isNull()/setNull()/getPointer()`. Replace: `p.isNull()` -> `!p`, `p.setNull()` -> `p.reset()` or `p = nullptr`, `p.getPointer()` -> `p.get()`. No `staticCast<>()`/`dynamicCast<>()` calls were found in game code (0 hits), so no `static_pointer_cast` migration needed there — but re-check OgreMax's helper templates since they wrap the same pointer types heavily (117 hits of general `Ptr`/pointer-ish String/Utilities code in OgreMaxUtilities.cpp — worth a manual pass). |
| `StringConverter::`/`StringUtil::` calls | 61 occurrences across 13 files (`Application.*`, `BasicView.*`, `OgreWindowListener.*`, `InputManager.*`, `GameStateManager.h`, `OgreMaxScene.*`, `OgreMaxRenderWindowIterator.hpp`) | CoreEngine, OgreMax | Mechanical, low-risk | Core `StringConverter`/`StringUtil` API is largely stable 1.7->14; main risk is `Ogre::vector<String>::type` return values from `StringUtil::split` (see row 1) and minor signature narrowing (some `toString`/`parseX` overloads changed default args). Needs a compile-and-fix pass, not a rewrite. |
| `WindowEventUtilities`, `FrameListener`, `RenderWindow` | 117 occurrences across 21 files (heaviest: `OgreMaxUtilities.cpp` 47, `OgreMaxScene.cpp` 17, `CustomOgreMaxScene.cpp` 10) | Nearly every project | Behavioral-risk | `Ogre::FrameListener::frameStarted/frameEnded` signatures changed to take `const Ogre::FrameEvent&` still, but return-value semantics and some renamed events (`frameRenderingQueued`) should be diffed against `Application.cpp`/`OgreWindowListener.cpp`. `WindowEventUtilities` API is mostly unchanged. Needs targeted review rather than a mechanical rewrite; flagged as the largest "long tail" of touch points. |
| `HardwareBuffer`/`VertexDeclaration`/lock-flag usage | 18 occurrences across 3 files, all in **OgreMax** (`OgreMaxTypes.hpp`, `OgreMaxUtilities.*`) | OgreMax only | Mechanical | `HardwareBuffer::LockOptions` enum and lock-flag names are stable; the risk is `HardwareVertexBufferSharedPtr`/`VertexDeclaration` element-type enums (`VET_*`) that got a couple of new members (double-precision types) — additive change, should just compile. Confirm with a build attempt. |
| RenderSystem_Direct3D9 availability | referenced in `setup.iss`, `plugins_debug.cfg`, `plugins_release.cfg` | data/, installer | **Critical / strategic** | See dedicated section below — D3D9 **is** still buildable in Ogre 14 / the vcpkg port, but is opt-in, not a default feature. |
</br>

## D3D9 / Cg / fixed-function findings (as requested)

**D3D9 RenderSystem:** Ogre 14.5.2 (OGRECave/ogre mainline) **still ships and builds
`RenderSystem_Direct3D9`** — confirmed in `vcpkg/ports/ogre/portfile.cmake`
(`vcpkg_check_features` maps `d3d9` -> `OGRE_BUILD_RENDERSYSTEM_D3D9`) and in
`vcpkg.json`'s `features.d3d9` (`"description": "Build Direct3D9 RenderSystem"`,
`"supports": "windows"`). It is **not** in `default-features` (only
`assimp, freeimage, overlay, zip` are default) — the port must be consumed as
`ogre[d3d9]` to get it. The portfile hard-codes
`OGRE_BUILD_RENDERSYSTEM_D3D11=ON`, `OGRE_BUILD_RENDERSYSTEM_GL=ON`,
`OGRE_BUILD_RENDERSYSTEM_GL3PLUS=ON`, and `GLES/GLES2=OFF`; D3D9 is the only
render system gated behind an explicit feature flag. Practically: D3D9 is available
and still has the fixed-function pipeline (FFP) — a real option for "least
functionality change."

**Cg plugin:** `Plugin_CgProgramManager` is **not** an available feature/component
of the vcpkg `ogre` port at all (no `cg` feature exists in `vcpkg.json`; upstream
Ogre dropped the Cg plugin years ago following NVIDIA's discontinuation of the Cg
toolchain in 2012). Blast radius in this codebase is small: exactly one asset,
`data/levels/materials/programs/asteroid.program` (2 program defs, vs_1_1/ps_2_0
profiles), consumed by the asteroid material used from `Asteroid.cpp`. This is a
one-file rewrite (to HLSL or GLSL), not a systemic problem.

**Fixed-function materials:** Of 16 `.material` files in `data/`, 13 contain zero
`program_ref` (pure fixed-function texture/pass blocks) — essentially all
level/track/hovercraft/character materials. Only `SkyX.material` (12 program refs)
and the one asteroid material are shader-driven. This is the crux of the strategy
decision: modern GL3+/D3D11 in Ogre 14 have no FFP and require RTSS
(`Ogre::RTShaderSystem`) to synthesize per-material shaders for all 13
fixed-function materials — a nontrivial integration (RTSS init, shader-generation
scheme wiring, per-viewport material scheme, subtleties with texture blend modes
not perfectly reproduced by RTSS). D3D9 in Ogre 14 still has native FFP, so keeping
D3D9 as the render system sidesteps this whole category of behavioral risk.

**Hikari/Flash GUI:** `HovercraftUniverse/dependencies/Hikari` is a **prebuilt
binary** (Win32 `Hikari.dll`/`Hikari.lib`, MFC/OLE/ActiveX Flash-hosting code,
`#include <windows.h>` in `FlashControl.h`) — this locks the whole build to
Win32/x86 regardless of Ogre version, confirming the stated constraint
independent of the Ogre port choice. `FlashControl` renders to an `Ogre::TexturePtr`
+ a dynamically created `Ogre::Material`/`TextureUnitState` (`createMaterial()`),
and wraps it in an internal `Impl::ViewportOverlay` when used as a screen overlay —
i.e. it depends on the Overlay component (confirmed present as a default vcpkg
feature) plus ordinary `Ogre::Material`/`Texture` APIs (not raw FFP passes), so it
is not itself a blocker for moving off FFP — but every place code builds its own
Ogre overlay UI around the Hikari texture (`GUI/OverlayManager.cpp`, `GUI/OverlayContainer.cpp`,
`GUI/MouseVisualisation.cpp`) is in the 73-occurrence Overlay-component list above
and needs the include/registration fixes.

## Strategy recommendation

**Recommendation: target a mid-point release — Ogre 1.9 or 1.12 — rather than
modern Ogre 14, if minimizing functionality change is the primary goal. If
long-term maintainability/toolchain currency matters more, target Ogre 14 with
D3D9 explicitly enabled (`ogre[d3d9]`) and budget real time for RTSS migration.**

Reasoning, based on what was measured:

1. **D3D9 is available in both options** (Ogre 14's vcpkg port supports it via the
   `d3d9` feature, and obviously 1.9/1.12 has it too), so "modern Ogre lacks D3D9"
   is *not* a blocking argument for going all the way to 14 — this removes what
   would otherwise be the strongest reason to pick 14 over a mid-point.
2. **The FFP/RTSS problem only exists if you also switch away from D3D9.** 13 of 16
   materials are fixed-function; as long as D3D9 stays the render system (matching
   "least functionality change" and matching Hikari's Win32/x86 constraint), the
   FFP question is moot on Ogre 14 too — D3D9's FFP support hasn't been removed in
   mainline Ogre, only GL3+/D3D11/GLES2 dropped it. So if you're going to stay on
   D3D9 regardless, this specific risk doesn't differentiate 1.9/1.12 vs 14.
3. **What does differentiate them is the STL-wrapper / SharedPtr / Overlay-component
   API churn**, all of which is present in Ogre 14 but was **already removed/changed
   starting around the 1.9/1.10 timeframe** (STLAllocator wrappers dropped, Overlay
   split into a component, SharedPtr modernized). A mid-point Ogre (1.9/1.12) still
   requires essentially the *same* mechanical fixes found here (the `Ogre::vector<T>::type`,
   `isNull()/getPointer()`, Overlay-component include changes) — there is no smaller
   Ogre version still on vcpkg's radar that predates these changes and is still
   buildable on a modern toolchain, so the porting labor measured above (roughly:
   6 STL-typedef fixes, 14 SharedPtr fixes, 73 Overlay-component touch points, 20
   Font touch points, 2 SceneType fixes, 1 Cg shader) is required **regardless of
   which post-1.7 target is chosen**.
4. Where 1.9/1.12 *would* genuinely reduce risk is the truly dead-weight items:
   `Plugin_CgProgramManager`/PCZSceneManager/OctreeZone/BSPSceneManager are all
   still buildable on 1.9/1.12 (unlike on 14, where none of them exist upstream at
   all) — but the codebase doesn't actually call any PCZ/BSP/Octree scene-manager
   APIs directly (0 hits), so those cfg lines look like dead legacy config from the
   original 2010 project, not a real dependency. The only genuine loss is the Cg
   plugin, and that's a single small shader asset to rewrite either way.
5. **Trade-off that favors 14 despite equal mechanical cost:** 1.9/1.12 are
   unmaintained and **not in vcpkg** — meaning the "smaller, less-changed" option
   actually costs *more* engineering effort up front (you'd have to hand-roll the
   whole dependency/build story that vcpkg's `ogre` port already solves for 14:
   FreeType/pugixml/zlib/SDL2/stb wiring, CMake config, D3D9 SDK discovery, MSVC
   toolchain compatibility patches — the port's own patch set, e.g. `cmake4.patch`,
   `fix-dependencies.patch`, shows how much of that plumbing already had to be
   redone for modern CMake/MSVC). Given the project already migrated to vcpkg +
   CMake (see recent commits "Add CMake build system and dependency bootstrap
   script"), building 1.9/1.12 from source outside vcpkg reintroduces exactly the
   dependency-hell problem the revival effort is trying to escape.

**Net call:** go with vcpkg `ogre` 14.5.2, request the `d3d9` feature explicitly
(`ogre[d3d9,overlay]` — overlay is already default), and keep D3D9 as the shipped
render system. This gets FFP for free (no RTSS work needed), keeps GL as a
secondary/debug render system if desired, and confines the "hard" porting work to
the already-enumerated mechanical items (STL typedefs, SharedPtr idiom, Overlay/Font
include paths + `OverlaySystem` registration, `ST_GENERIC`->string SceneManager
creation, and the one Cg shader). Do not attempt to source-build 1.9/1.12 — it does
not reduce the measured porting effort and adds a maintenance burden the vcpkg
migration was meant to remove.

## SimpleTrack2 distance-dependent rendering artifact -- mesh LOD

**Symptom:** on SimpleTrack2, stable, geometry-shaped black patches appear on
the asteroids at moderate camera distance and resolve correctly as the
camera approaches. Not shimmering or flickering -- the same patches, same
shape, every frame, at a given distance.

**Hypotheses eliminated by experiment before the real cause was found:**

1. **Shadows.** Forcing `SHADOWTYPE_NONE` changed nothing. Ruled out.
2. **Mipmaps.** Forcing `TextureManager::setDefaultNumMipmaps(0)` changed
   nothing. Ruled out.
3. **Far-plane clipping.** `RaceCamera` uses a far plane of 30000, far beyond
   the distance at which the artifact appears. Ruled out.
4. **Material LOD.** None of the shipped `.material` scripts use
   `lod_distances`/`lod_strategy`. Ruled out.
5. **Z-fighting.** The artifact is perfectly stable at a given camera
   distance, not shimmering -- inconsistent with a depth-precision race.
   Ruled out.

**Confirmed cause: mesh LOD.** Of the 24 meshes SimpleTrack2 loads, 17
(including `Asteroid01.mesh` and `Asteroid02.mesh` -- exactly the meshes
showing the artifact) carry 3 LOD levels each. Ogre logs these meshes as
using "an old format [MeshSerializer_v1.41]" on load. A temporary
experiment (a per-300-frame sweep in `Application::startRenderLoop()` that
logged each loaded mesh's LOD level count and called
`Mesh::removeLodLevels()`) made the artifacts disappear within a second of
starting the game, verified by a human tester. That experiment has been
reverted; the permanent fix is `LegacyMeshLodListener`
(`HovercraftUniverse/CoreEngine/LegacyMeshLodListener.h/.cpp`), installed
from `Application::createRoot()` next to
`DuplicateMaterialScriptCompilerListener::install()`. It implements
`Ogre::MeshSerializerListener::processMeshCompleted()` and acts on any mesh
with more than one LOD level as that mesh finishes loading. (What it *does*
there changed once the cause was fully understood -- see "...but that was only
half of it" below.)

**Root cause: `value` is never derived from `userValue` for legacy-format
meshes.** An `Ogre::MeshLodUsage` carries both. `userValue` is the switch
distance as authored; `value` is what the LOD strategy actually compares
against the camera every frame, normally produced by
`LodStrategy::transformUserValue(userValue)` (the default distance strategy
squares it, so it can compare against squared distances and skip a square
root per object per frame). Loading a `MeshSerializer_v1.41` mesh populates
`userValue` but leaves `value` at zero. Measured on every LOD-bearing mesh in
SimpleTrack2 -- `Asteroid01`, `Asteroid02`, `Rock01`, `Check01`, and the rest
are all identical:

```
Before: [0: value=0 userValue=0] [1: value=0 userValue=200] [2: value=0 userValue=400]
```

With every level's threshold at 0 the level selection is meaningless, and the
renderer shows heavily decimated geometry at distances where it should be
showing full detail -- exactly "black patches at range that resolve as you
approach".

**The fix** is one call: `Mesh::setLodStrategy()` re-derives `value` for every
level (level 0 gets the strategy's base value, the rest get
`transformUserValue(userValue)`). Re-applying the current default strategy
after load reconstructs precisely the data Ogre needs:

```
After:  [0: value=0 userValue=0] [1: value=40000 userValue=200] [2: value=160000 userValue=400]
```

40000 = 200^2 and 160000 = 400^2, confirming both the squaring convention and
that LOD now switches at the authored distances.

### ...but that was only half of it

Re-deriving `value` was reported as the fix. **It was not** -- it repaired the
switch distances, which were genuinely broken, but it only moved the artifact
out of view. The artifact came back on the next play session.

The tell is *where* it was looked for. `Asteroid01`/`Asteroid02` are where the
player spends nearly all their time, comfortably inside the 200-unit LOD 0
band, so those meshes' reduced levels are almost never on screen. `Planet01` --
the third asteroid, at `(-1278, 281, -454)`, approached from 700+ units away --
renders at **LOD 2 for most of the approach**, and still showed the artifact,
with exactly the distance-dependent signature the original report described:
fixed in shape, resolving as the camera closes and crosses into LOD 1 then 0.

So Ogre 14's legacy read path gets **two** things wrong about a
`MeshSerializer_v1.41` mesh: the `value`/`userValue` derivation above, *and*
the reduced levels' geometry itself.

Confirmed by a decisive experiment: `LegacyMeshLodListener` was switched to
`Mesh::removeLodLevels()`, discarding the reduced levels entirely. That
removes the artifact on `Planet01` completely (human-verified, 31/07/2026).
Correct switch distances into broken geometry is still broken geometry.

**The permanent fix is therefore `Mesh::removeLodLevels()`** -- discard the
levels rather than trust them. These meshes are small by modern-GPU standards,
so always rendering full detail costs essentially nothing. It touches nothing
on disk: the assets under `HovercraftUniverse/data/` stay byte-for-byte as
recovered (a project deliverable) and the authored LOD data is simply ignored
at load time.

*(If the LOD levels are ever actually wanted back -- they buy nothing on
current hardware -- the way to do it is to regenerate them from LOD 0 with
Ogre 14's own `MeshLodGenerator` and re-apply the authored `userValue`
distances, rather than trusting what the legacy deserializer produced. The
`setLodStrategy()` call is preserved, commented out, next to the
`removeLodLevels()` call for whoever picks that up.)*

**Process note.** The first fix was reported as verified on the strength of a
mechanism that explained the data (`value=0` really was wrong) plus a check in
the place the symptom was first reported. Neither established that the symptom
was gone everywhere it occurred. A distance-dependent artifact needs to be
checked at the *worst-case distance in the level*, which here was a different
object entirely.
