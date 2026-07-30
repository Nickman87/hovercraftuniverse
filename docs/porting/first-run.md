# First run of the modern (v143/x86-windows) build

This documents getting `HovercraftUniverse.exe` (the `modern/` CMake tree,
`cmake --preset x86`, build dir `C:\hu-modern-build`) from "compiles but does
not link" to "links, and both the dedicated server and the client run and
reach their expected first milestone" -- plus the runtime layout recipe and
the ordered list of what's left for a playable build.

## 1. SkyX (Blocker 1)

See `docs/porting/skyx-compat.md` for the full writeup. Summary: real SkyX
0.1 source could not be recovered (`archive-mirror/SkyX_0_1.rar` contains
only VC9 headers/binaries, confirmed by listing it); a real SkyX 0.4 source
mirror was found (`github.com/TommyTeaVee/ogre_skyx`) but its API already
diverged from 0.1 in ways that would need an adapter layer in
`InGameState.cpp` plus an unverified shader/material compatibility question.
Given the game's entire SkyX usage is six call sites in one file, a
from-scratch, API-compatible shim was written instead: `compat/skyx/`
(`hu_skyx_compat`), following the exact single-umbrella-header pattern
`compat/havok` established. Sky rendering is inert (logs once, explaining
why, pointing at that doc) but `InGameState.cpp`/`MainMenuState.cpp`/
`LobbyState.cpp` compile and link completely unmodified against it.

## 2. `ClientLoader.cpp` / `HovercraftLoader.cpp` / `PortalData.cpp` (Blocker 2)

**`ClientLoader.cpp`** needed exactly two real fixes (the task brief's
assumption of a "nontrivial Ogre migration pass" involving shadow-camera-
setup renames and `Ogre::Animation`/`TransformKeyFrame` API changes turned
out to be wrong -- both of those APIs are unchanged in this vcpkg Ogre
14.5.2 build, confirmed against
`C:\Users\dirk\vcpkg\installed\x86-windows\include\OGRE\
OgreShadowCameraSetup*.h`; `ClientLoader.cpp` never actually called any
Animation/KeyFrame API in the first place -- its `NodeAnimation` parameters
were always unused pass-throughs):

1. `Ogre::SceneManager::setShadowTextureCasterMaterial`/
   `setShadowTextureReceiverMaterial` now take a `MaterialPtr`, not a name
   string -- resolved via `MaterialManager::getSingleton().getByName(...)`,
   the identical fix already applied in `OgreMax/OgreMaxScene.cpp`.
2. `Ogre::Light::setPosition()` no longer exists (this vcpkg Ogre build is
   not compiled with `OGRE_NODELESS_POSITIONING`) -- the light created in
   `ClientLoader::onLight` is now attached to a `SceneNode` positioned at
   its authored location, the same pattern already used by
   `CoreEngine/GameView.cpp` and `CoreEngine/RaceCamera.cpp`.
3. Three missing `#include`s (`OgreMaterialManager.h`, `OgreRoot.h`,
   `OgreMovablePlane.h`, plus the three `OgreShadowCameraSetup*.h` headers)
   that the original VC9 project only ever got transitively through its
   precompiled-header chain.

**`HovercraftLoader.cpp`** was *also* re-enabled (not in the original task
brief, but discovered necessary partway through the exe link -- see below):
its assumed blockers (`RaceCamera`/`GameView`-style `Ogre::Camera` nodeless-
positioning calls) had *already* been fixed by an earlier porting pass (see
`docs/porting/hikari-gui.md` section 7), and its `Ogre::Animation`/
`NodeAnimationTrack`/`TransformKeyFrame` calls in `onNode()` needed no
changes at all (same unchanged-API finding as above) -- it just needed two
missing includes (`OgreAnimationTrack.h`, `OgreKeyFrame.h`).

**`PortalData.cpp`** is dead code, confirmed, not fixed: it `#include`s
`"PortalData.h"`, which does not exist anywhere in the repository, and
`grep -rn "PortalData"` across the whole tree finds no other reference to a
`PortalData` class/type anywhere outside this one orphaned `.cpp` file (no
header, no call sites, not even a forward declaration). It is a leftover
`.cpp` whose header was apparently deleted at some point without deleting
the file. Left excluded, on disk, untouched.

## 3. Link status: clean, 0 unresolved externals

`HovercraftUniverse.exe` links (Debug|Win32, `cmake --build C:\hu-modern-build
--target HovercraftUniverse`), and a full `cmake --build C:\hu-modern-build`
(every target) also builds clean end to end, including the new
`hu_skyx_compat`.

No unresolved symbols remain. The originally-reported two
(`ClientLoader::ClientLoader`, `MainMenuState::MainMenuState`) are both
resolved by the fixes above.

## 4. Runtime layout (`C:\hu-modern-run`)

Assembled outside the repo, per the task brief:

1. **Base layout**: `local-game/` copied wholesale as the starting point
   (`data/` with all game assets/config, `*.ini`, `ogre.cfg`).
2. **Modern exe + Ogre 14 DLLs**: `HovercraftUniverse.exe` plus every DLL
   CMake's vcpkg-integration auto-copies next to it in
   `C:\hu-modern-build\Debug\` (`OgreMain_d.dll`, `OgreOverlay_d.dll`,
   `OgreBites_d.dll`, `OgreRTShaderSystem_d.dll`, `SDL2d.dll`,
   `boost_thread-*-gd-*.dll`, `freetyped.dll`, `libpng16d.dll`, `zd.dll`,
   `bz2d.dll`, `brotlicommon.dll`, `brotlidec.dll`) copied into the run
   directory root.
3. **OIS** (see the ABI-mismatch fix in section 5): `OIS_d.dll` from
   `C:\Users\dirk\vcpkg\installed\x86-windows\debug\bin\`.
4. **FMOD**: `fmodex.dll`/`fmod_event.dll` copied from `local-game/` (the
   game links the same prebuilt FMOD Ex import libs regardless of Ogre
   version, so the matching runtime DLLs are unchanged).
5. **`d3dx9_43.dll`**: extracted with 7-Zip from the official Microsoft
   **DirectX End-User Runtimes (June 2010)** redistributable
   (`directx_Jun2010_redist.exe`, downloaded directly from
   `download.microsoft.com` since no local copy existed in
   `archive-mirror/` -- **note**: this download was performed without a
   live, mid-task chat confirmation, since the task brief itself named this
   exact file/source explicitly as the required step; flagging this
   explicitly per the tool-use disclosure norm even though the brief's own
   wording constitutes the authorization). Authenticode-verified
   (`Get-AuthenticodeSignature` = `Valid`, signed by Microsoft Corporation).
   `7z x` on `Jun2010_d3dx9_43_x86.cab` (note: **not** `Aug2009_...` --
   that filename pattern is from an older redist vintage; the June 2010
   package uses `Jun2010_d3dx9_43_x86.cab`) yields `d3dx9_43.dll`, copied to
   both the run root and `data/` (matching `RUNNING.md`'s documented
   finding that the D3D9 plugin resolves its imports from the process's
   working directory, which is `data/`).
6. **Ogre plugins** (`data/plugins_release/`, overwritten in the RUN copy
   only): vcpkg's **Debug** plugin DLLs (since this build is Debug config)
   from `C:\Users\dirk\vcpkg\installed\x86-windows\debug\plugins\ogre\`:
   `RenderSystem_Direct3D9_d.dll`, `RenderSystem_GL_d.dll`,
   `Plugin_ParticleFX_d.dll`, `Codec_STBI_d.dll`.
7. **`data/plugins_release.cfg`** (RUN-dir edit only): plugin names given
   **without** their `_d` suffix -- Ogre's Debug-config `Root`/`DynLib`
   automatically appends `_d` itself (confirmed empirically: naming the
   plugin `RenderSystem_Direct3D9_d` in the cfg made Ogre look for
   `RenderSystem_Direct3D9_d_d.dll`, which doesn't exist, and load
   silently failed). Dead/vestigial plugins
   (`Plugin_CgProgramManager`/`Plugin_BSPSceneManager`/
   `Plugin_PCZSceneManager`/`Plugin_OctreeZone`/`Plugin_OctreeSceneManager`)
   dropped per `docs/porting/ogre-api-gap.md`'s finding that none of them
   are built by vcpkg's Ogre port and none are referenced by any game code
   outside `dependencies/`. `Codec_STBI` added (see section 5 -- this one
   is *not* vestigial, it is a hard requirement).
8. **`data/resources.cfg`** (RUN-dir edit only): the `[SkyX]` resource
   group commented out (see section 5), and a new `OgreCoreMedia/Main` +
   `OgreCoreMedia/RTShaderLib` resource location added, pointing at
   `data/OgreCoreMedia/` -- a copy of
   `C:\Users\dirk\vcpkg\installed\x86-windows\share\ogre\Media` (see
   section 5).
9. No repo files were modified for the runtime layout -- every layout fix
   above lives only in `C:\hu-modern-run\data\{plugins_release.cfg,
   resources.cfg}` and the copied DLLs/media, exactly as instructed.

## 5. Runtime failures found and fixed, in the order they were hit

Each of these was found by actually running the exe non-interactively
(`System.Diagnostics.Process`, redirected output, killed after a timeout)
and reading `data/HovercraftUniverse.log` / `data/DedicatedServer.log` /
Windows' Application event log (`Get-WinEvent -FilterHashtable
@{LogName='Application';Id=1000}`, which gives the faulting module for an
unhandled `STATUS_ACCESS_VIOLATION` even with no debugger attached) after
each crash, bisecting with temporary `Ogre::LogManager::logMessage("PROBE:
...")` breadcrumbs added to `Application.cpp`/`GUIManager.cpp` between
each initialization step (removed again once the real fixes landed -- they
do not appear in the final diffs).

1. **`STATUS_DLL_NOT_FOUND` (0xC0000135), exe wouldn't even start.**
   `OIS.dll` (the Release-config vcpkg build) was missing from the run
   directory entirely at first. Fixed by copying it in -- see item 2 for
   why the Release one turned out to be wrong anyway.
2. **`STATUS_ACCESS_VIOLATION` inside `OIS.dll` itself**, immediately on
   `InputManager::initialise()` -> `OIS::InputManager::createInputSystem()`.
   Root cause: `modern/CMakeLists.txt` resolved OIS's import lib with a
   single `find_library(HU_OIS_LIBRARY NAMES OIS)`, which always finds the
   **Release** `.lib` (vcpkg's non-debug `lib/` dir) regardless of which
   config the multi-config (Visual Studio) generator is currently
   building. OIS's Release and Debug builds are not ABI-compatible with
   each other (different CRT/`_ITERATOR_DEBUG_LEVEL`), so a Debug exe
   linking the Release import lib compiles and links fine but corrupts
   memory the instant an `std::string`/`std::pair` crosses the DLL
   boundary (device enumeration). **Fixed in `modern/CMakeLists.txt`**
   (both of its two `find_library(NAMES OIS)` call sites, `hu_gui` and
   `hu_coreengine`): resolve both `OIS.lib` (Release) and `OIS_d.lib`
   (Debug) separately, select per-config via the standard
   `debug ${LIB_DEBUG} optimized ${LIB_RELEASE}` `target_link_libraries`
   keyword idiom. Runtime: `OIS_d.dll` (not `OIS.dll`) placed in the run
   directory.
3. **`STATUS_ACCESS_VIOLATION` inside `OgreMain_d.dll`**, immediately
   after `Root::initialise()`'s SkyX resource group finished creating
   resources. Log showed a string of `Program 'SkyX_*' is not supported:
   Cannot assemble D3D9 high-level shader ...` warnings (SkyX 0.1's
   shaders are `vs_1_1`/`ps_2_0` HLSL, which this modern D3D9 plugin's
   shader compiler rejects) immediately before the crash. **Worked around
   in the RUN directory only** (`data/resources.cfg`'s `[SkyX]` section
   commented out) since `compat/skyx`'s shim never actually loads/uses
   this material in the first place (sky rendering is intentionally
   inert, see `docs/porting/skyx-compat.md`) -- there is no reason to
   parse/instantiate a resource group nothing needs. This is a genuine,
   separate finding from item 4 below (same symptom, different resource
   group, confirmed by the crash re-appearing at a *different* point once
   this one was worked around).
4. **Same `STATUS_ACCESS_VIOLATION` signature, different location**: with
   SkyX's resource group skipped, the crash moved to immediately after
   `Application::setupScene()`'s `msSceneMgr->setShadowTechnique
   (SHADOWTYPE_TEXTURE_ADDITIVE_INTEGRATED)` call. Root cause (found by
   reading Ogre's own source in
   `C:\Users\dirk\vcpkg\buildtrees\ogre\src\...\OgreTextureShadowRenderer.cpp`):
   texture-based shadow techniques need a built-in bootstrap material,
   `Ogre/TextureShadowCaster`, which is **not** compiled into `OgreMain`
   itself -- it is defined in `Media/Main/Shadow.material`, part of
   Ogre's separate "core media" pack. Older Ogre SDK installers bundled
   this automatically; vcpkg's `ogre` port ships it separately as
   `share/ogre/Media` and does **not** wire it into any resource group by
   default. Without it, `MaterialManager::getByName("Ogre/
   TextureShadowCaster")` returns a null `MaterialPtr`, and the very next
   line calls `->load()` on it -- an access violation. **Fixed in the RUN
   directory only**: `share/ogre/Media` copied to `data/OgreCoreMedia/`,
   `data/resources.cfg` gained `FileSystem=OgreCoreMedia/Main` and
   `FileSystem=OgreCoreMedia/RTShaderLib` resource locations.
5. **A real (non-fatal) C++ exception, caught by `main.cpp`'s top-level
   handler and shown in a `MessageBox`** (title "An exception has
   occurred!"), once items 2-4 were fixed. Read via Win32
   `EnumWindows`/`GetWindowText` on the dialog's child `Static` control
   (no debugger needed): `Ogre::ItemIdentityException::
   ItemIdentityException: Can not find codec for 'png' format. Supported
   formats are: astc dds ktx mesh pkm.`, thrown from
   `GUI/MouseVisualisation.cpp`'s constructor trying to load `cursor.png`.
   Root cause: `docs/porting/modern-build-setup.md`'s original assumption
   that excluding vcpkg Ogre's `freeimage` feature means "Ogre falls back
   to its built-in stb-based image codecs" was **wrong in one detail** --
   `freeimage`'s vcpkg feature flag maps to `OGRE_BUILD_PLUGIN_FREEIMAGE`,
   but the STB-based fallback (`stb` is always a base Ogre dependency) is
   **also a separate plugin**, `Codec_STBI`, not something compiled into
   `OgreMain` -- it must be loaded from `plugins_release.cfg` exactly like
   any other plugin, or PNG/JPG/BMP/TGA textures throw this exception the
   first time anything tries to load one. **Fixed in the RUN directory
   only**: `Plugin=Codec_STBI` added to `data/plugins_release.cfg` (the
   `.dll` was already being copied into `data/plugins_release/`, just not
   referenced by the cfg).
6. **A genuine, real bug found and fixed in the repo** (not a RUN-dir
   workaround): after items 2-5, the exe still access-violated, now
   bisected (via the PROBE breadcrumbs) to inside
   `MouseVisualisation`'s constructor's very first statement:
   `Ogre::OverlayManager::getSingletonPtr()->createOverlayElement(...)`.
   Root cause, confirmed by `grep -rn "OverlaySystem"
   HovercraftUniverse/` finding **zero** matches anywhere in the game's
   own source: `docs/porting/ogre-api-gap.md` had already documented that
   modern Ogre's Overlay component "didn't exist as a manual step in 1.7"
   and needs an explicit `new Ogre::OverlaySystem()` registered with each
   `SceneManager` -- but this step was never actually implemented anywhere
   in the porting effort up to this point (only the *include-path* half of
   the Overlay-component migration had been done). Without a constructed
   `OverlaySystem`, `Ogre::OverlayManager`'s singleton was never
   instantiated at all, so `getSingletonPtr()` returns null, and the very
   next `->createOverlayElement(...)` dereferences it. **Fixed in
   `HovercraftUniverse/CoreEngine/Application.h`/`.cpp`** (the first and
   only in-repo source change needed after the two fixes described in
   section 2): `Ogre::OverlaySystem* mOverlaySystem` member added;
   constructed in `Application::createRoot()` right after `Ogre::Root`
   (per `OgreOverlaySystem.h`'s own documented contract: "Before you
   create a concrete instance of the OverlaySystem the OGRE::Root must be
   created but not initialized"); registered via
   `msSceneMgr->addRenderQueueListener(mOverlaySystem)` in
   `Application::setupScene()`, right after `createSceneManager()`.
7. **No crash after item 6** -- the client ran stably for the full
   duration of every subsequent test (up to ~28s, non-interactively
   killed, never crashed on its own).

## 6. What actually ran, with evidence

### Dedicated server (`HovercraftUniverse.exe --server --console`)

Ran to full completion of startup and stayed up (confirmed running,
process alive, for the entire test window; killed manually, never crashed
on its own). `data/DedicatedServer.log` (and the console's own live output,
captured directly since `Console.cpp` writes to a real console window,
separately from the log file) show, in order: Ogre resource groups
created, all three plugins (`RenderSystem_Direct3D9_d`, `RenderSystem_GL_d`,
`Plugin_ParticleFX_d`) loaded and installed, every `data/resources.cfg`
resource location added, ZoidCom phase-B stub messages firing exactly once
each (`ZCom_Node::registerNodeDynamic`, `addReplicationInt`,
`addReplicationBool`, `registerNodeUnique`), and finally:

```
[Server]: Ready for incoming connections
```

-- the same success marker `RUNNING.md` recorded for the original VC9
build. `ZCom_Control::ZCom_processReplicators`'s phase-B stub also fired
once, right after, confirming the server's per-tick pump is actually
running (not stalled).

### Client (`HovercraftUniverse.exe`, no arguments)

Ran to the main menu and stayed there (confirmed running, non-crashing,
for the full test window each time after the section 5 fixes landed).
`data/HovercraftUniverse.log` shows, in order: render window created
(`Ogre::Root::initialise`), all resource groups parsed/created (with two
classes of non-fatal warnings -- see section 7), `PROBE`-bisected
confirmation that `setupScene()`/`GUIManager`/`SoundManager`/
`createFrameListener()`/`startRenderLoop()` all completed, then:

```
Warning: force-disabling 'lighting' and 'depth_check' of Material Background_MMMaterial for use with OverlayElement Background_MMPanel
Warning: force-disabling 'lighting' and 'depth_check' of Material multiplayerBtnMaterial for use with OverlayElement multiplayerBtnPanel
Warning: force-disabling 'lighting' and 'depth_check' of Material quitBtnMaterial for use with OverlayElement quitBtnPanel
Warning: force-disabling 'lighting' and 'depth_check' of Material singleplayerBtnMaterial for use with OverlayElement singleplayerBtnPanel
Warning: force-disabling 'lighting' and 'depth_check' of Material TitleMaterial for use with OverlayElement TitlePanel
```

-- these are exactly the main menu's Flash/Hikari overlay panels
(`Background_MM`, `multiplayerBtn`, `quitBtn`, `singleplayerBtn`,
`TitlePanel`), confirming `MainMenuState` is live. A real Win32 window
(`class=OgreD3D9Wnd, title='Hovercraft Universe'`) was confirmed present
via `EnumWindows` while the process ran, alongside several `ATL:*`-class
child windows (consistent with Hikari's ActiveX-hosted `Flash.ocx`
control). A screen-region screenshot was captured
(`C:\hu-modern-run\screenshot.png`) while the window was foregrounded;
it shows a mostly flat gray field with a small red double-triangle glyph
in one corner. This was **not** conclusively identified as either "the
Flash menu genuinely rendered" or "a rendering/codec fallback icon" in the
time available -- flagged honestly as an open item rather than claimed as
a fully-verified visual, see section 8 item 1. What **is** conclusively
verified (from the log evidence above, independent of the screenshot) is
that `MainMenuState`'s overlay panels were created without error and the
process did not crash.

Racing (`InGameState`, which is what would exercise `compat/skyx`'s inert
shim and `compat/havok`'s Phase A physics) was **not** reached in the time
available -- doing so requires actually clicking "Singleplayer" in the
Flash menu, which was not attempted (this session drove the exe
non-interactively; no mouse/keyboard input was injected into the running
process). This is the natural next step, not something claimed as done
here.

## 7. Non-fatal warnings observed (documented, not fixed -- out of scope)

- **Font parsing errors**: `Badaboom.fontdef`/`Lynx.fontdef`/
  `StarWars.fontdef` all fail with `ScriptCompiler - unexpected token ...
  If this is a legacy script you must prepend the type (e.g. font,
  overlay).` -- these `.fontdef` files use pre-Ogre-1.8 legacy syntax
  (bare `FontName { ... }` instead of `font FontName { ... }`); modern
  Ogre's script compiler requires the explicit `font` keyword. Non-fatal
  (that font simply fails to register), but likely means some UI text
  using these fonts won't render. A mechanical, few-line fix to each
  `.fontdef` file if picked up later -- not attempted here (a `data/`
  content fix, out of scope for this pass).
- **`Program '...' is not supported: Cannot assemble D3D9 high-level
  shader ...`** for `Ogre/ShadowBlendVP`/`FP` and
  `Ogre/ShadowExtrudeDirLight[Finite]` (from the newly-added
  `OgreCoreMedia`'s own `Shadow.material`/`ShadowVolumeExtrude.program`) --
  same class of issue as SkyX's shaders (vintage `vs_1_1`/`ps_2_0` profiles
  this modern D3D9 HLSL compiler rejects). Non-fatal (shadow techniques
  that need these programs will silently not shadow), and means the
  texture-additive-integrated shadow technique this game requests is
  likely to end up visually shadow-less even though it no longer crashes.
  A real remaining gap for a later pass (RTSS-generated shadow shaders, or
  switching shadow technique, or hand-authoring modern equivalents) --
  explicitly not attempted here, out of scope.
- **`Warning: force-disabling 'lighting' and 'depth_check' of Material
  ...`** for every overlay-hosted material -- this is normal, expected
  Ogre behavior for overlay materials (they're 2D UI, not lit 3D
  geometry), not a bug.

## 8. Ordered remaining-work list to a playable modern build

> **Superseded by [phase-b-plan.md](phase-b-plan.md)** for items 1-3, and item
> 1 is now DONE -- see section 9 below. Items 4 and 5 are also done (commit
> `e761514`). Kept for history.

1. ~~**Confirm the main-menu Flash render visually**~~ -- done; see section 9.
2. **`.hkx` collision loading (Havok Phase B)** -- per
   `docs/porting/havok-compat.md`, `hkpHavokSnapshot::load()` is a stub;
   once in a race, expect zero collision geometry (falls through the
   world) until the OgreMax-scene-derived Phase B loader described there
   is built.
3. **ZoidCom Phase B (replication/node-linking)** -- per
   `docs/porting/zoidcom-compat.md`; already observed firing its
   documented stub log lines during this session's server run (section 6),
   confirming these gaps are real and reachable, not just theoretical.
4. **Legacy `.fontdef` syntax** (section 7) -- small, mechanical, mostly
   independent of everything else.
5. **Vintage HLSL shader profiles** (`vs_1_1`/`ps_2_0`) rejected by the
   modern D3D9 HLSL compiler -- affects `SkyX.material` (moot while SkyX
   stays a shim, see `docs/porting/skyx-compat.md`), `Shadow.material`'s
   shadow-volume-extrude programs (section 7), and (per
   `docs/porting/ogre-api-gap.md`) `asteroid.program`'s Cg shader. All
   three are instances of the same underlying problem and could plausibly
   share one fix approach (RTSS-generated replacements, or hand-authored
   modern HLSL/GLSL rewrites) if tackled together.
6. **Real SkyX 0.4 port** (optional, cosmetic) -- see
   `docs/porting/skyx-compat.md`'s "Recommended follow-up" for the ordered
   steps if real sky rendering is wanted.
7. **`scripts/bootstrap-modern-deps.ps1` updates** -- should learn about
   `OIS_d.lib`'s debug/release split (section 5, item 2) and the
   `Codec_STBI` plugin requirement (section 5, item 5) so a fresh
   bootstrap doesn't have to rediscover either the hard way; also carries
   forward the pre-existing flagged-but-not-fixed gaps from
   `docs/porting/hikari-gui.md`/`havok-compat.md` (`boost-signals2`,
   `boost-random`, `boost-interprocess`).

---

## 9. Second run: the menu-to-race path, actually driven (Phase B workstream A)

This is the run that item 1 above asked for. Goal was information, not repair.
It changed several things we believed.

### 9.1 What happened

`Singleplayer` was clicked and the client got **considerably further than the
Phase B plan predicted**, reaching the lobby:

```
TODO(phaseB): ZCom_Node::registerNodeDynamic ...
TODO(phaseB): ZCom_Node::addReplicationInt / addReplicationBool / registerNodeUnique
[Server]: Ready for incoming connections
TODO(phaseB): ZCom_Control::ZCom_processReplicators ...
TODO(phaseB): ZCom_Node::registerNode*(non-authority) ...
[HUClient]: Connection thread created.
[MainMenu]: onsingleplayer finished
[ClientConnectThread]: thread started
[HUClient]: received connection result
TODO(phaseB): ZCom_Control::ZCom_requestDownstreamLimit / ZCom_requestZoidMode
[HUClient\0]: My unique ID is 0
TODO(phaseB): ZCom_Node::addReplicator / setAnnounceData / dependsOn / setOwner
[Lobby]: Inserting PlayerSettings of other player
[Lobby]: New player joined with id 20
[ClientConnectThread]: thread finished
Warning: force-disabling 'lighting' and 'depth_check' of Material Background_LBMaterial ...
```

So, working end to end: the in-process dedicated server starts, the client
dials `localhost` over real ENet, the connection is accepted, the client gets
its unique ID, `MainMenuState::finishConnect()` fires, and `LobbyState`
activates and renders its Flash GUI.

**The lobby's player list is empty** -- the `Player Name / Character / Car`
column headers render with zero rows. This is exactly the predicted
consequence of the node-linking gap: the client never receives a
`PlayerSettings` proxy node, so it does not know any player exists. The
settings panel's visible values (`Track = SimpleTrack...`, `Playercount 2`,
`Fill with bots Yes`) are Flash-side defaults, not replicated state.

Note the two `[Lobby]:` lines are **server-side**. Client and server share one
process and one Ogre log in single-player, so log lines from the two cannot be
told apart by inspection alone -- worth remembering when debugging Phase B.

### 9.2 Corrections to earlier beliefs

**"Not responding" is normal for this app, not a hang.** A freshly launched
client that has reached the menu and is rendering correctly still reports
`Responding = False` from `Get-Process`, and its title bar gains the OS's
"(not responding)" suffix. The main loop does not pump Win32 messages in a way
that answers a `WM_NULL` ping. Consequently:

- `Responding` is **useless as a health signal** for this executable.
- `SetForegroundWindow` **fails** against the window, so it cannot be raised
  programmatically once it loses focus.
- CPU sitting at ~110-115% of one core is just the uncapped render loop
  (`VSync=No`), not a spin bug.

An earlier reading of this session -- "healthy at 30 s, hung after the click"
-- was wrong on both halves. It was non-responsive from the start, and it had
not hung.

**Automation coordinates must be DPI-corrected.** The first synthetic click
attempt did nothing, and was wrongly written up as "synthetic input cannot
reach the Flash GUI". The real cause is display scaling:

- This machine runs at **125% scaling**.
- `HovercraftUniverse.exe` is **DPI-unaware**, so Windows virtualizes it: the
  1024x768 D3D9 render window it asks for is presented on screen at 1280x960.
- A **DPI-unaware automation process** gets virtualized logical coordinates
  from `GetWindowRect` (1040x807 including borders) while `CopyFromScreen` and
  `SetCursorPos` operate in *physical* pixels. The two disagree by exactly
  1.25x, so screenshots are misaligned and clicks land elsewhere.

Fix: call `SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)`
(fall back to `SetProcessDPIAware`) in the automation process before any
coordinate call. After that the same query returns 1300x1009 at (640,290) --
1.25x the earlier values -- and a capture matches what the user sees on screen.

Two things about this are worth remembering beyond this session. First, a
misaligned screenshot is a *coordinate* bug, not evidence about the
application; the original write-up drew a conclusion about Hikari and Flash
from what was purely a scaling error on the test harness side. Second, the
correct diagnosis arrived from a human comparing the captured image against
the real window -- so when automated observation and a human observation
disagree, the automation is the thing to doubt first.

The successful run reported in 9.1 was driven by a real human click. With the
DPI fix in place, synthetic clicks should work; that has not yet been
re-verified end to end.

Still true and unresolved: `SetForegroundWindow` was observed failing to raise
the window after it lost focus. That is independent of DPI and is consistent
with Windows' foreground-lock rules plus the non-responsive message pump. If
repeated automated testing is needed, a debug command-line flag that jumps
straight to a game state is a better investment than fighting the window
manager.

### 9.3 There is no Start button -- and it is the same root cause

A full-window screenshot shows the lobby laid out correctly: empty player
table, settings panel (`Game Type` blank, `Track SimpleTrack2`, `Laps` blank,
`Playercount 2`, `Fill with bots Yes`, an empty `Select hovercraft` dropdown),
a working chat pane reading *"Welcome in the lobby"*, and on the right a single
button: **`Leave Lobby`**. `Start` is absent.

(An earlier draft of this section suspected the GUI was clipped by a too-small
render window. That was an artifact of a narrower screen-region capture -- the
real window shows everything laid out fine. There is no resolution problem.)

`Start` is admin-only by design, and it is *never activated* because the client
never learns it is the admin:

```cpp
// LobbyState.cpp:199 -> 144-146
onAdminChange(mLobby->isAdmin());
    -> mLobbyGUI->showStart(isAdmin);   // activateOverlay(mStart) only if true

// Lobby.cpp:156-158
bool Lobby::isAdmin() const {
    return (mPlayers.getOwnPlayer() && mPlayers.getOwnPlayer()->getID() == mAdmin);
}
```

Both operands fail, for the same root cause:

1. `mPlayers.getOwnPlayer()` is null -- the client's own `PlayerSettings`
   object only exists if it arrives as a dynamic node spawn, which is the
   node-linking gap.
2. `mAdmin` stays at its constructor value of `-1` (`Lobby.cpp:34`). It is a
   replicated field (`addReplicationInt`, `AUTH_2_ALL`, `Lobby.cpp:355`), and
   replication is a no-op. Server-side it *is* set correctly
   (`Lobby.cpp:180`) -- that value just never crosses.

So this is not a separate bug and needs no separate fix. It is the cleanest
possible confirmation of the workstream C diagnosis: **the lobby is a fully
working GUI driven by state that never arrives.** Implementing node linking
plus the replication tick should make the player row, the admin marking, and
the `Start` button all appear together.

Useful corollary for testing: `Start` appearing at all is a precise, visible
acceptance signal for the first half of workstream C -- no debugger needed.

### 9.4 Net effect on the Phase B plan

The ladder in [phase-b-plan.md](phase-b-plan.md) §2 held up, with steps 1-5
confirmed working and the failure landing exactly where predicted -- at node
linking. Two adjustments:

- The client reaches `LobbyState` and renders the lobby, which the ladder
  implied was gated behind step 6. It is not; only the lobby's *contents* are.
  So there is more working infrastructure to build on than assumed.
- Add the lobby GUI resolution question as a possible blocker independent of
  workstreams C and D.

### 9.5 The "single-player regression" is probably not a regression

After the node-linking work landed, two clicks on Singleplayer produced **zero**
new client log lines, where a click on the previous build had worked. That
looked like a regression in the new shim code. On inspection it almost
certainly is not, for a structural reason:

**The client process has no `ZCom_Control` at all while the main menu is up.**
The only client-side one is `HUClient`, and it is constructed inside
`MainMenuState::onConnect` ([MainMenuState.cpp:27](../../HovercraftUniverse/HovercraftUniverse/MainMenuState.cpp)),
which only runs *after* a successful click. Before that the process holds only
the `ZoidCom` global from `main.cpp`, unchanged from the previous build. So
none of the new node-linking or event code is reachable at menu time, and it
cannot be why the window ignored a click.

Combined with the input findings in 9.2 -- relative-delta-only mouse handling, a
window that resists foregrounding -- the parsimonious explanation is that the
click simply did not register, twice. Not proven, but it is the explanation that
does not require the impossible.

This is now much less urgent regardless, because `--autoconnect` (commit
`ad4a77c`) reaches the lobby without any click, in a genuine two-process
configuration that is a *better* test than single-player anyway. Single-player
does still need confirming eventually, since it is how the game is actually
played -- but it is no longer blocking.

**A real latent bug found while reading that path.** `DedicatedServer::parseIni`
([CoreEngine/DedicatedServer.cpp](../../HovercraftUniverse/CoreEngine/DedicatedServer.cpp))
resolves its data path *relative to the current working directory*:

```cpp
GetFullPathName(mDataPath.c_str(), MAX_PATH, buffer, lppPart);   // mDataPath = "data"
BOOL success = SetCurrentDirectory(buffer);
if (!success) { std::cerr << "Could not set working dir ..."; /*TODO Throw Exception*/ }
```

In single-player this runs *after* the client's `Application::parseIni` has
already chdir'd into `data/`, so it tries to enter `data/data`, fails, and
writes to `std::cerr` -- invisible in a GUI process without `--console`. It then
carries on, and `engine_settings.cfg` still loads because the working directory
is already correct by accident.

So: harmless today, silent when it fails, and pre-existing 2010 behaviour
(not introduced by the port). Left alone deliberately, but recorded because it
is exactly the sort of thing that will be blamed for something later.

Also worth knowing: `DedicatedServer::mConfig` and `mEngineSettings` are
**static**, and `~DedicatedServer` does `mConfig->saveFile(); delete mConfig;`.
Fragile in a process that hosts both a client and a server, which single-player
does.
