# SkyX compatibility shim

## Why this exists

The game (`HovercraftUniverse/HovercraftUniverse/InGameState.cpp`/`.h`) is
built against **SkyX 0.1**, a small third-party Ogre add-on for sky/cloud/
star rendering (LGPL 2.1, Xavier Verguin Gonzalez). Only prebuilt VC9
binaries and headers survive: `archive-mirror/SkyX_0_1.rar` was inspected
and confirmed to contain exactly 20 files -- 2 DLLs (`bin/debug/SkyX_d.dll`,
`bin/release/SkyX.dll`), 2 `.lib` files, and the public headers under
`dependencies/SkyX/include` -- **no `.cpp` sources at all**. Those VC9
binaries cannot be relinked against MSVC v143 (SkyX is a real C++-ABI
library: virtual calls, no C shim), so, same as Havok and ZoidCom, this is
a genuine "no source, no vendor, no newer binary" gap.

## Route taken: from-scratch API shim, not a real SkyX port

The task brief's preferred route was finding real SkyX source matching the
0.1 API and vendoring it. That was investigated:

- A real, full-source SkyX mirror **was found**:
  `github.com/TommyTeaVee/ogre_skyx` (cloned during this task, ~20 `.cpp`/
  `.h` files under `src/`, plus a `CMakeLists.txt`). This is **not
  abandonware** -- it clones and builds.
- However, it is **SkyX 0.4** (`SKYX_VERSION_MAJOR/MINOR/PATCH` = 0/4/0 in
  its `Prerequisites.h`), not 0.1, and its public API already diverged from
  the 0.1 headers this game was written against in a way that is not a
  drop-in match:
  - `SkyX::SkyX`'s constructor changed from `(Ogre::SceneManager*,
    Ogre::Camera*)` (0.1) to `(Ogre::SceneManager*, Controller*)` (0.4) --
    a `Controller`/`BasicController` abstraction was introduced in between.
  - Per-camera updates moved from being implicit (0.1 just took the camera
    at construction time) to an explicit, separately-invoked
    `SkyX::notifyCameraRender(Ogre::Camera*)` call (0.4), meant to be driven
    either manually or via `Ogre::RenderTargetListener` registration on the
    viewport's render target.
  - `data/levels/materials/programs/SkyX.material` (12 `program_ref`s, the
    game's actual shipped shader data) was authored against 0.1's
    `GPUManager` shader-parameter conventions; whether 0.4's GPUManager
    (which gained a lightning system and other 0.1->0.4 feature additions,
    per its extra `VClouds/Lightning*.h` files not present in the 0.1
    headers) reads the same material/parameter names was not verified and
    is a real, unquantified risk.
  - Porting 0.4 would additionally need its own Ogre 1.x->14 API-gap pass
    (a non-trivial fraction of its ~20 files reference `isNull()`/
    `setNull()`/`getPointer()`, `HardwareVertexBuffer`/`VertexDeclaration`
    lock-flag idioms, and `MaterialManager`/`GpuProgramParameters` calls --
    the same classes of break documented in `docs/porting/ogre-api-gap.md`
    for the rest of this codebase).
- Given the game's entire SkyX usage is **six call sites in one file**
  (`InGameState.cpp`: construct, `create()`, `getAtmosphereManager()`
  `getOptions()`/`setOptions()`, `remove()`, `update()`), reconstructing a
  from-scratch, 0.1-API-compatible shim is both lower-risk (no adapter
  layer needed in game code, no untested shader/material compatibility
  question) and faster than a real 0.4 port plus an Ogre-14 migration pass
  plus an InGameState.cpp adapter for the changed construction/update
  protocol. **This is the option actually implemented.**

This is a real trade-off, not a dead end: `third_party/skyx` real 0.4 source
could still be vendored properly in a follow-up pass if real sky rendering
is wanted later -- see "Recommended follow-up" below.

## What was built

`compat/skyx/` (owned by this porting effort, not a port of anything under
`HovercraftUniverse/`), following the exact structural precedent set by
`compat/havok`'s single-umbrella-header shim (see
`docs/porting/havok-compat.md`'s "Shim structure" section) rather than
ZoidCom's one-class-per-real-header layout, since SkyX's real API is small
enough that mirroring its actual multi-header layout would add nothing:

- `compat/skyx/include/SkyX.h` -- a one-line thin forwarding header
  (matches the real 0.1 include path: `#include <SkyX.h>`, flat, no
  subdirectory) that includes the umbrella header below.
- `compat/skyx/include/skyx_compat/SkyXAll.h` -- every class body:
  `SkyX::SkyX` (real constructor signature, `create()`/`remove()`/
  `update()`/accessors, all inert no-ops except bookkeeping), `SkyX::
  AtmosphereManager` (its `Options` struct reproduced field-for-field with
  the same defaults as the real 0.1 header, since `InGameState.cpp`
  round-trips an `Options` value through `getOptions()`/mutate/
  `setOptions()`), and stub `CloudsManager`/`VCloudsManager`/`MoonManager`/
  `ColorGradient` classes (declared, for API completeness per the task
  brief, even though `InGameState.cpp` never actually calls into them --
  its one `getCloudsManager()` call site is commented out in the original
  game source).
- `compat/skyx/src/SkyXAll.cpp` -- out-of-line implementation plus
  `skyx_compat::todoPhaseBOnce()`, an exact mirror of
  `havok_compat::todoPhaseBOnce()`/`zshim::todoPhaseBOnce()`: logs
  `[hu_skyx_compat] TODO(phaseB): <message>` once per distinct call site.
- `compat/skyx/CMakeLists.txt` -- `hu_skyx_compat` static library, links
  `OgreMain` only (the public API surface this game touches is just
  `Ogre::SceneManager`/`Camera`/`Vector2`/`Vector3`).
- `modern/CMakeLists.txt` -- `add_subdirectory` for `compat/skyx`; linked
  into `hu_game_physics` (`PUBLIC`, so its include dir propagates to
  `InGameState.cpp`); re-enabled `InGameState.cpp`, `MainMenuState.cpp`
  (which `#include`s `InGameState.h` to transition into it), and
  `LobbyState.cpp` (which does the same, plus directly instantiates `new
  InGameState(...)`) -- all three were previously excluded purely for the
  missing `<SkyX.h>`.

## Behavior: sky rendering is genuinely absent, not faked

`SkyX::create()` logs, once, that no sky dome/sun/moon/stars/clouds will be
rendered and why (pointing at this document). `update()` is a real no-op
(nothing was created, so there is nothing to advance). `getAtmosphereManager
()->setOptions(...)` stores the given `Options` value (so `InGameState.cpp`'s
call sequence doesn't need any special-casing) but there is no shader/
material to push the values to, and logs the same TODO(phaseB) once. This
was verified end-to-end: `InGameState.cpp` compiles and links completely
unmodified against this shim.

Practical effect at runtime: races run with whatever the level's own skybox/
background (if any) provides, and no atmospheric scattering, sun disc, moon,
star field, or volumetric clouds. This was not itself observed during this
task's client run (the client reached the main menu but racing/InGameState
was not entered in the available time -- see `docs/porting/first-run.md`),
so it is a design-level consequence of the shim, not yet an empirically
confirmed screen.

## Recommended follow-up (real sky rendering)

If real sky rendering is wanted later, in priority order:

1. Vendor `github.com/TommyTeaVee/ogre_skyx` (SkyX 0.4) into
   `third_party/skyx/` as its own `hu_skyx` target (same shape as
   `hu_hikari`), run it through the same Ogre 1.x->14 API-gap fixes as
   every other vendored dependency in this tree.
2. Adapt `InGameState.cpp`'s construction/update call sites for 0.4's
   `Controller`-based constructor (`new SkyX::BasicController()` in place
   of the bare camera pointer) and its `notifyCameraRender()` step (easiest
   via `viewport->getTarget()->addListener(mSkyX)`, matching 0.4's own
   documented `RenderTargetListener` integration pattern -- no manual
   per-frame call needed).
3. Verify (or re-author) `data/levels/materials/programs/SkyX.material`
   against 0.4's `GPUManager` shader-parameter conventions -- this is the
   one genuinely open question 0.4 introduces that 0.1 wouldn't have had.
4. Swap `compat/skyx_compat` out of `hu_game_physics`'s link line for the
   new `hu_skyx` target; the `#include <SkyX.h>` call sites in
   `InGameState.cpp`/`.h` do not need to change (0.4's own top-level
   `SkyX.h` header still exists at the same include path).
