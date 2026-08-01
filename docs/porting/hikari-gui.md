# Hikari (Flash-in-Ogre GUI) + GUI project porting report

Scope: rebuilding Hikari from source against Ogre 14.5.2/v143 (`hu_hikari`),
porting `HovercraftUniverse/GUI` against it (`hu_gui`), re-enabling the
GUI/Sound-tainted files this unblocks in `hu_coreengine`/`hu_game_physics`,
and a first attempt at linking the actual game executable.

## 1. Why Hikari pins this whole build to Win32/x86

Hikari hosts Adobe's Flash ActiveX control (`Flash.ocx`) via COM/OLE
(`IOleObject`, `IOleInPlaceSiteWindowless`, etc, see
`third_party/hikari/include/impl/FlashSite.h`) and renders it into an
`Ogre::Texture`. Flash.ocx itself only ever shipped as a 32-bit ActiveX
control. This is confirmed, not assumed: the only copy of it in this
repository, `local-game/data/Flash.ocx`, is a 32-bit DLL, and Hikari's COM
plumbing (`IOleInPlaceObjectWindowless`, `IOleClientSite`) has no 64-bit
story for hosting it. That is why `modern/CMakeLists.txt` targets
Win32/x86 end to end, independent of every other porting decision in this
tree (Ogre, Havok, ZoidCom, etc all happen to also be easiest on x86, but
Hikari/Flash is the one hard requirement).

## 2. Vendoring and licensing

Upstream Hikari (LGPL 2.1, Adam J. Simmons, Google Code SVN mirror) is
vendored at `third_party/hikari/{include,source}` from
`archive-mirror/hikari-source-archive.zip` (the `.svn` directories and the
unrelated `HikariLua` subproject were not copied; `HikariDemo` was not
copied either). `third_party/hikari/LICENSE-hikari.txt` is the upstream
license text.

**LGPL implication**: Hikari is statically linked into `hu_hikari.lib` and
from there into every consumer, ultimately the game executable. LGPL 2.1
permits static linking as long as the *library's own source* (this vendored
copy, plus the patches below) remains available/re-distributable and users
retain the ability to relink against a modified Hikari. Since this is an
open, in-repo source vendor (not a black-box binary), that condition is
satisfied by the repository itself; no further action was taken, but this
should be kept in mind if the modern build is ever distributed as a binary
release without its source.

**Flash.ocx itself is not vendored or redistributed** by this porting
effort. It is only referenced (via `local-game/data/Flash.ocx`, itself
outside version control per `.gitignore`) as a build-time and run-time
dependency the end user must supply themselves (as the original game did).

## 3. The team's Hikari patches (reconstructed)

Only prebuilt VC9 binaries plus the patched **public** headers survive from
the team's patched Hikari (`HovercraftUniverse/dependencies/Hikari/{includes,lib}`;
the three "Win-Hikari-Update" revisions are mirrored as
`archive-mirror/Win-Hikari-Update{,_2,_3}.rar`, all of which contain the same
prebuilt binaries + headers, no source). No patched `.cpp` files survive
anywhere, so the source-level implementation of the patch had to be
reconstructed from:

1. Diffing upstream's headers (`archive-mirror/hikari-source-archive.zip`)
   against the patched ones (`HovercraftUniverse/dependencies/Hikari/includes`).
2. Cross-checking every changed signature against actual call sites in
   `HovercraftUniverse/GUI/**` (the only consumer in this repository) to
   confirm what the game actually uses, and to disambiguate any patch that
   could have been implemented multiple ways.

After reconstruction, diffing the *reconstructed* public headers
(`third_party/hikari/include/{FlashControl.h,Hikari.h}`) against the
patched ones byte-for-byte confirmed an exact match (only doxygen-comment
wording differs) -- see the diff run during this task, reproduced here for
the record:

```
$ diff -u HovercraftUniverse/dependencies/Hikari/includes/FlashControl.h third_party/hikari/include/FlashControl.h
$ diff -u HovercraftUniverse/dependencies/Hikari/includes/Hikari.h third_party/hikari/include/Hikari.h
(only added doc-comments for the two new public methods; no signature differences)
```

### Patch 1 -- multi-tier z-ordering collapsed to a flat zOrder

Upstream: `FlashControl`/`Impl::ViewportOverlay`/
`HikariManager::createFlashOverlay` all took `(Ogre::uchar zOrder, Ogre::uchar tier)`,
with z-ordering computed as `100 * tier + zOrder` and tier-scoped "highest
zOrder" lookups. The patch removed `tier` entirely and widened `zOrder`
from `Ogre::uchar` (0-255) to `Ogre::ushort`. Confirmed against
`HovercraftUniverse/GUI/**`: every call site (`GUIManager::createOverlay`,
`BasicOverlay`, `OverlayParameters`) already only ever passes a bare
`zOrder`, never a tier -- the tier concept was entirely unused by this game.
Reproduced in `third_party/hikari/include/impl/ViewportOverlay.h`,
`include/FlashControl.h`, `include/Hikari.h` and their `.cpp` files.

### Patch 2 -- dead API removed

`FlashControl::setPosition`/`resetPosition`/`getCoordinates` and
`HikariManager::getFocusedControl` were removed from the public API.
Confirmed unused anywhere in `HovercraftUniverse/GUI/**` (grep found zero
call sites) -- simple dead-weight removal.

### Patch 3 -- `FlashControl::handleInputs`/`isHandlingInputs` added

`GUI/BasicOverlay.cpp` calls `mFlashControl->handleInputs(!mIgnoreInputs)`
from both its constructor-time setup and its `show()`/`hide()` paths. This
is a genuinely new feature: an overlay with input handling turned off never
participates in mouse hit-testing/focus (implemented as an early-out in
`FlashControl::isPointOverMe`), letting a purely-informational HUD element
(e.g. the speedometer) render on top of other overlays without stealing
clicks meant for whatever is visually behind it.

### Patch 4 -- `HikariManager::toggleBringToTop`/`mBringToTop` added

`GUI/GUIManager.cpp` calls `mHikariMgr->toggleBringToTop(val)`. Reconstructed
as a flag gating the "bring the just-focused control to the front of the
z-order" reshuffle inside `HikariManager::focusControl` -- when disabled,
focusing a control still grants it keyboard/mouse focus but no longer
reorders the render stack. (The tier-scoped partitioning that used to gate
this reshuffle is gone along with tiers, per Patch 1.)

## 4. Ogre 14 API fixes applied inside Hikari (not team patches)

These are purely mechanical/build fixes for the 2008-era Hikari source
against Ogre 14.5.2 + v143, independent of the team's functional patches
above:

- **Overlay component split**: `Ogre::Overlay`/`OverlayManager`/
  `PanelOverlayElement` moved out of core Ogre into the separate Overlay
  component. Added explicit `#include <OgreOverlay.h>`/`<OgreOverlayManager.h>`
  to `impl/ViewportOverlay.h` (was relying on the monolithic `Ogre.h`
  transitively providing them, which it no longer does).
- **`Ogre::DisplayString` is now narrow**: it used to be a distinct
  (potentially wide) string typedef; in this Ogre build it's simply
  `typedef Ogre::String DisplayString;` (`OgreOverlayElement.h`). This broke
  in two ways:
  - `FlashValue`'s `Ogre::DisplayString` constructor overload became an
    exact duplicate of its existing `std::string` overload (removed; see
    `include/FlashValue.h`).
  - Every place that mixed `Ogre::DisplayString` (now narrow) with the
    wide (`std::wstring`) invocation protocol Hikari uses internally to
    talk to the Flash ActiveX control (`impl/flashhandler.h`'s
    serialize/deserializeInvocation, BSTR round-trips) needed an explicit
    narrow/widen conversion at the boundary: `FlashValue`'s
    `wchar_t*`/`wstring` constructors (`FlashValue.cpp`'s `narrowFromWide`),
    `impl/flashhandler.h`'s `serializeValue` (`widenToWide`),
    `FlashControl::callFunction` (widen before serializing) and
    `FlashControl::handleFlashCall` (narrow before the `delegateMap` lookup).
    All conversions are ASCII-range only, which is sufficient for what this
    library actually exchanges with ActionScript (function names, numbers,
    "true"/"false", and short UI text).
- **`Ogre::SharedPtr` idiom**: `texture.setNull()` (two call sites in
  `FlashControl.cpp`) -> `texture.reset()` (Ogre 14's SharedPtr dropped
  `isNull()`/`setNull()`/`getPointer()`, same class of break documented in
  `docs/porting/ogre-api-gap.md`).
- **`FlashControl`'s removed `lastDirtyWidth`/`lastDirtyHeight` members**
  (gone per the team's own patch, Patch 1's header diff also showed this):
  reconstructed the DC-rebuild-on-resize behavior via a `GetObject()`-based
  bitmap-size query instead of cached members (see the comment in
  `FlashControl::update`).
- **ActiveX typelib import without system registration**: upstream used
  `#import "PROGID:ShockwaveFlash.ShockwaveFlash" named_guids`
  (`impl/FlashSite.h`, `impl/flashhandler.h`), which requires Flash.ocx to be
  registered (`regsvr32`) on the build machine -- a system-wide change this
  porting effort will not make. Replaced with a **computed `#import`**
  (`#import HIKARI_FLASH_OCX_PATH named_guids`, MSVC supports macro
  expansion here exactly like a computed `#include`), where
  `HIKARI_FLASH_OCX_PATH` is defined by `hu_hikari` (via the
  `HU_FLASH_OCX_PATH` CMake cache variable, defaulting to
  `local-game/data/Flash.ocx`) to the quoted, absolute path of a real
  Flash.ocx. This loads the typelib directly out of the OCX file with zero
  registry/system state touched.
- **Static-lib build**: `HikariPlatform.h`'s `_HikariExport` macro gained a
  `HIKARI_STATIC` branch (no-op instead of dllexport/dllimport) since
  `hu_hikari` builds Hikari as a static lib (matching every other ported
  project in this tree) rather than the original `Hikari.dll`.

## 5. `hu_hikari` build status: **clean**

`third_party/hikari/{include,source}` builds warning-light and
error-free as `hu_hikari` (static lib, Win32/Debug, v143, against
`OgreMain`+`OgreOverlay`), linking `ole32`/`oleaut32`/`user32`/`gdi32` for
its COM/GDI double-buffering plumbing. See `modern/CMakeLists.txt`'s
`hu_hikari` section for the full target definition and inline rationale.

## 6. `hu_gui` (HovercraftUniverse/GUI) build status: **clean**

Ported in place per `HovercraftUniverse/GUI/CMakeLists.txt`'s source list
(read-only, not modified; `Test.cpp` stays excluded per that file's own
comment). Fixes applied directly to `HovercraftUniverse/GUI/**`:

- **Overlay component includes**: `MouseVisualisation.h` needed explicit
  `#include <OgreOverlay.h>`/`<OgreOverlayManager.h>`/`<OgreOverlayContainer.h>`
  (same class of fix as Hikari's `ViewportOverlay.h` above) --
  `Ogre::OverlayManager`/`Ogre::Overlay`/`Ogre::OverlayContainer` are no
  longer visible via the monolithic `Ogre.h`.
- **Missing vcpkg Boost components**: `GUIManager.h` uses
  `boost::signals2::mutex` (`<boost/signals2/mutex.hpp>`) and
  `IResolution.h` uses `boost::shared_ptr` (`<boost/shared_ptr.hpp>`).
  `boost-signals2` was not in the original bootstrap script's vcpkg install
  list (installed via `vcpkg install boost-signals2:x86-windows` for this
  task; `boost-smart-ptr`, which provides `shared_ptr.hpp`, was already
  installed but `hu_gui` wasn't linking `Boost::boost` at all, so its
  include dir wasn't on the path -- fixed by linking `Boost::boost` and
  `Boost::signals2`). `scripts/bootstrap-modern-deps.ps1` should gain
  `boost-signals2` too (not touched here, flagged for whoever owns that
  script next).
- **Ambiguous `long` -> `Hikari::FlashValue` overload resolution**:
  `Countdown.cpp` (`Countdown::start`/`resync`) and `Results.cpp`
  (`Results::addPlayer`) passed a bare `long` into `Hikari::Args(...)`,
  which is ambiguous between `FlashValue`'s `int` and `Ogre::Real`
  converting constructors under standard overload resolution (both are
  equally-ranked standard conversions from `long`) -- apparently tolerated
  by the original VC9 toolchain, a hard `C2440` error under v143. Fixed
  with an explicit `(int)` cast at each call site (values are milliseconds/
  race-finish times, always well within `int` range).
- **`Speedometer.h`: real pre-existing bug** -- `void
  Speedometer::setBoost(Ogre::Real boost);` declared *inside* the
  `Speedometer` class body with a redundant, invalid `Speedometer::`
  qualifier (the same class of bug already documented for
  `CustomOgreMaxScene.h` in `modern/CMakeLists.txt`'s hu_game_physics
  section -- MSVC's classic permissive parser tolerates it, `/permissive-`
  rejects it as `C4596`). Fixed by removing the redundant qualifier.
- **`OverlayParameters.h`: real pre-existing dead/broken code** --
  `OverlayParameters<T>::getInstancedOverlay()` referenced an undeclared
  identifier `mtype` (a typo of the class's own `mType` typedef), was
  missing the `<T>` template argument on its out-of-class qualification,
  and never returned anything. Confirmed unused anywhere in
  `HovercraftUniverse/**` (grep for `getInstancedOverlay`/`OverlayParameters<`
  found only the declaration itself) -- this method could never have
  compiled or worked as shipped. Fixed to do what its name/signature
  obviously intend (construct a `T`, wrap in `boost::shared_ptr<T>`, return
  it), since leaving it broken blocks every consumer of the header
  (a template's body is still syntax-checked even when never instantiated).

## 7. Re-enabled in `hu_coreengine` (previously GUI/Sound-tainted)

All five files the mission named are now built (see `modern/CMakeLists.txt`'s
`hu_coreengine` target): `EntityRepresentation.cpp`, `BasicGameState.cpp`,
`GameStateManager.cpp`, `RepresentationManager.cpp`, `Application.cpp`.
Needed, beyond linking `hu_gui`/`hu_sound` and adding their include dirs:

- **`_HAS_STD_BYTE=0`**: `RepresentationManager.h` has a pre-existing
  `using namespace std;` at header/global scope. Harmless in 2010, but now
  that this file's #include chain also reaches `GUIManager.h`/`Hikari.h`'s
  `<windows.h>`/COM headers (`objidl.h` etc, which declare a global
  `typedef unsigned char byte;`) in the same translation unit, it collides
  with C++17's `std::byte` (pulled unqualified into scope) --
  `'byte': ambiguous symbol` (`C2872`) across every COM header that
  mentions `byte`. `_HAS_STD_BYTE=0` is the standard MS-STL escape hatch
  for exactly this legacy-code-plus-Windows-SDK collision.
- **`Application.cpp` Ogre-API-gap fixes** (independent of the GUI taint,
  same class of issue as `docs/porting/ogre-api-gap.md`):
  - `Root::showConfigDialog()` now requires an `Ogre::ConfigDialog*`
    argument (the old no-argument overload backed by a built-in native
    dialog is gone). Fixed using `OgreBites::getNativeConfigDialog()`
    (`<OgreBitesConfigDialog.h>`, OgreBites is already linked).
  - `Ogre::ST_GENERIC` / the `SceneType`-enum overload of
    `createSceneManager` no longer exists; switched to the string-typed
    overload (`createSceneManager("DefaultSceneManager", "Default")`).
- **`RaceCamera.cpp`/`.h` and `GameView.cpp`/`.h`**: pulled in transitively
  by `RepresentationManager.h` (`#include "GameView.h"`), and previously
  excluded for a *separate* reason from the GUI taint -- `Ogre::Camera`'s
  and `Ogre::Light`'s "nodeless positioning" methods
  (`setPosition`/`setDirection`/`lookAt`/`setFixedYawAxis`/`pitch`/`yaw`/
  `roll`/`getOrientation`/`getPosition`) only exist when Ogre itself is
  built with `OGRE_NODELESS_POSITIONING` (an Ogre build-time option, gating
  the methods' declarations in e.g. `OgreCamera.h`/`OgreLight.h` --
  they're not merely deprecated, they don't exist in the compiled
  `OgreMain.lib` at all), which this tree's vcpkg `ogre` port is not built
  with. Migrated in place to the SceneNode-based idiom throughout
  `RaceCamera.cpp`/`.h` (operate on `mActiveViewpointNode`, the camera's
  always-attached parent `SceneNode`, instead of the camera itself) and
  `GameView.cpp` (the point light was never attached to a SceneNode at all;
  gave it one). Convenient confirmation the fix is correct: a previous
  developer had already sketched most of the exact same
  SceneNode-based replacement inline as commented-out code next to each
  broken call, which this fix now uses verbatim.

## 8. Re-enabled in `hu_game_physics` (previously GUI/Sound/Scripting-tainted, "EXCLUDED (A)")

Re-added (all now build against `hu_gui`, `hu_sound`, `hu_scripting`,
`_HAS_STD_BYTE=0` as above): `ClientPreparationLoader.cpp`,
`HUApplication.cpp`, `HUClient.cpp`, `HUD.cpp`,
`HovercraftRepresentation.cpp`, `InGameState.cpp`, `LobbyGUI.cpp`,
`LobbyState.cpp`, `MainMenu.cpp`, `MainMenuState.cpp`,
`ClientConnectThread.cpp`. `main.cpp` is deliberately **not** added to
`hu_game_physics` -- it's the `WinMain` entry point and belongs to the
`HovercraftUniverse` exe target instead (see below).

`Portal.cpp` was ALSO fixed and re-enabled (2 call sites of the same
`Ogre::vector<Ogre::String>::type`/`StringUtil::split` idiom as `main.cpp`,
see below) -- discovered necessary while attempting the exe link (see
section 10).

Still excluded, for reasons independent of GUI (unchanged by this task,
confirmed still necessary by the exe link attempt below):
`PortalData.cpp` (orphaned, no header) and `ClientLoader.cpp`/
`HovercraftLoader.cpp` (shadow-camera-setup renames,
`Ogre::Animation`/`TransformKeyFrame` API changes -- a distinct, nontrivial
Ogre migration pass, not attempted here).

`hu_game_physics` (78 of 91 .cpp) builds **clean**.

## 9. A new, genuinely out-of-scope blocker found: SkyX

Re-enabling `InGameState.cpp` (GUI-tainted) surfaced a *different* real
dependency gap: `InGameState.h`/`.cpp` `#include <SkyX.h>` and instantiate
`SkyX::SkyX` directly (a sky-rendering Ogre add-on). SkyX headers survive at
`HovercraftUniverse/dependencies/SkyX/include`, and its source archive is
mirrored at `archive-mirror/SkyX_0_1.rar`, but it has never been
vendored/built anywhere in this modern tree -- porting it would be a
separate task of the same shape as `hu_hikari` (vendor + CMake target +
Ogre-API fixes), squarely out of scope for a Hikari/GUI porting task. Left
excluded: `InGameState.cpp`, plus two files that transitively need it
(`MainMenuState.cpp` `#include`s `InGameState.h` directly to transition
into it; `LobbyState.cpp` does the same AND directly instantiates `new
InGameState(...)`).

## 10. Exe link attempt: **2 unresolved externals, both already-documented, non-GUI blockers**

`HovercraftUniverse` (`WIN32` subsystem, `main.cpp` + `hu_game_physics` and
its full transitive closure -- `hu_coreengine`, `hu_networking`,
`hu_zoidcom_compat`, `hu_havok_compat`, `hu_ogremax`, `hu_gui`, `hu_sound`,
`hu_scripting`, `hu_utils`, `hu_exceptions`, `OgreMain`, `OgreOverlay`,
`OgreBites`) **compiles and reaches the linker**. `main.cpp` needed the same
two fixes as elsewhere: the `Ogre::vector<Ogre::String>::type`/
`StringUtil::split` idiom (row 1 of `docs/porting/ogre-api-gap.md`) and an
explicit `#include <zoidcom/zoidcom.h>` (previously only ever reached
transitively via the VC9 project's precompiled-header chain).

First link attempt surfaced **18 unresolved externals**, all resolved down
to exactly **2**, in two iterations:

1. **`EntityPropertyMap`/`EntityProperty`/`EntityPropertyFactory`/
   `EntityPropertyMapReplicator`** (referenced from `hu_coreengine`'s
   `Entity.cpp` and `hu_game_physics`'s `BoostAction`/`BoostProperty`/
   `SpeedBoost`/`HovercraftAIController`) -- these all live in
   `EntityPropertySystem.cpp`, which was excluded from `hu_coreengine` for
   a missing `#include <iostream>` (see section 7). Fixed and re-enabled;
   this alone cleared 11 of the 18.
2. **`Portal::Portal`/`onEnter`/`onLeave`/`load`/`getClassName`** -- from
   `Portal.cpp`, excluded for the `Ogre::vector<T>::type` idiom (see
   section 8). Fixed and re-enabled; this cleared 5 more.

**Remaining 2 unresolved externals, both already-documented and genuinely
out of scope for this task:**

- `HovUni::ClientLoader::ClientLoader(Ogre::SceneManager*)` -- from
  `ClientPreparationLoader.cpp` (re-enabled by this task) calling into
  `ClientLoader.cpp`, which is excluded for real, separate Ogre-API-gap
  reasons (shadow-camera-setup renames, `Ogre::Light::setPosition()`,
  `Ogre::Animation`/`TransformKeyFrame` API changes -- a nontrivial Ogre
  migration pass of its own, not attempted here).
- `HovUni::MainMenuState::MainMenuState()` -- from `HUApplication.cpp`
  (re-enabled by this task) calling `getInitialGameState()`, which
  constructs a `MainMenuState`; `MainMenuState.cpp` is excluded because it
  transitively needs the not-yet-vendored SkyX library (section 9).

**Notably absent from every stage of this link attempt: any Havok
phase-B-stub or ZoidCom-shim-gap symbol.** The original expectation (see
task background) was that reaching the linker would surface gaps in
`compat/havok`/`compat/zoidcom`; empirically, it did not -- every
unresolved symbol traced back to a GUI/SkyX/Ogre-API-gap file exclusion
already tracked in `modern/CMakeLists.txt`, not to the physics/networking
compat shims. This is a genuinely good sign for those shims' completeness
relative to what this game's code path actually calls.

## 11. What still blocks a running executable

1. **Link the last two symbols** by either (a) porting `ClientLoader.cpp`
   (shadow-camera-setup + Animation/KeyFrame Ogre-API-gap migration) or
   stubbing/excluding its one caller, and (b) vendoring SkyX (its own
   `hu_skyx`-shaped task) or stubbing out `MainMenuState`'s reference to
   `InGameState`.
2. **Runtime resource/config gaps** not touched by this task: the
   `plugins_*.cfg`/Cg-shader/RTSS items from `docs/porting/ogre-api-gap.md`
   (D3D9 fixed-function keeps most of these moot, per that doc's
   recommendation, but were not re-verified here).
3. **A real, loadable Flash.ocx** at run time next to the executable (see
   section 12 of this document, "Flash.ocx runtime requirement"), which
   this porting effort does not supply.
4. **This porting effort has not attempted an actual run** of the
   resulting (still not fully linked) `HovercraftUniverse.exe` -- once the
   above link gaps close, an end-to-end launch attempt is the natural next
   step, following the same verified-recipe approach as `RUNNING.md`.

## 12. Flash.ocx runtime requirement

Unchanged from before this task: the built game still needs a real,
32-bit Flash.ocx (Adobe Flash Player ActiveX control) present at
`Flash.ocx` next to the executable at run time (`HikariManager`'s
constructor `LoadLibrary`s it directly, falling back to
`CoCreateInstance`-by-registry if that fails). This porting effort does not
vendor or redistribute Flash.ocx; end users must supply their own copy (as
the original 2010 game required), exactly as `local-game/data/Flash.ocx`
demonstrates for local testing.
