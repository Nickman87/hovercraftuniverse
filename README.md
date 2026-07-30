# Hovercraft Universe

A multiplayer 3D hovercraft racing game set in space, where players race between planets and asteroids — each with its own atmosphere, gravity level and physical properties. Built in C++ on Ogre3D and Havok Physics.

Hovercraft Universe was developed in 2009–2010 by a student team at Hasselt University (Belgium) — Dirk Delahaye, Kristof Overdulve, Nick De Frangh, Olivier Berghmans, Pieter-Jan Pintens and Tobias Van Bladel — as project `uhasseltaacgua` on Google Code (SVN), for the course *Architectuur en Algoritmes van Computer Games*. This repository is a migration of that SVN trunk to GitHub. The game reached a working **v1.0** with an installer in June 2010 ([trailer](https://www.youtube.com/watch?v=rhufn_-8xO8)); development stopped after that. The original project was published under **GPL v3**.

- **▶ Want to just play it?** See [RUNNING.md](RUNNING.md) — the unmodified 2010 v1.0 binaries run on Windows 11 (verified July 2026), including the Flash GUI.
- **Wiki / documentation:** https://nickman87.github.io/hovercraftuniverse/ (migrated wiki, mostly Dutch — source in [`docs/wiki/`](docs/wiki/))
- **Original project (archived):** https://code.google.com/archive/p/uhasseltaacgua/
- **Development screenshots:** [`dev-screens/`](dev-screens/) (March–June 2010)

## The game

- Racing with hovercrafts across space tracks: checkpoints, laps, a countdown/racing/finished race state machine, position tracking and a results screen.
- **Planet gravity**: gravity phantoms pull craft toward planet surfaces; each world can have different gravity/atmosphere (configured per server in `engine_settings.cfg`).
- Track gimmicks: speed boosts, jumps, portals, power-up spawns, reset spawns, moving asteroids, a collision-prevention assist system.
- **Multiplayer** (up to 6 players by default): dedicated server or client-hosted, with lobby, in-game chat and server browser. Single player runs an embedded server.
- **AI bots** scripted in Lua (`data/scripts/AI/PathFollowing.lua`), with a path recorder tool to author racing lines. Servers can auto-fill open slots with bots.
- Shipped tracks in [`HovercraftUniverse/data/levels/`](HovercraftUniverse/data/levels/): SimpleTrack2, Bavaraf, Junkyard, Ramp, SoccerField (bots even play football — see `dev-screens/20100602-soccerbots.png`).
- Flash-based HUD and menus (speedometer, lap timer, position, direction arrow, chat, lobby, main menu) rendered in-engine.

## Repository layout

| Path | Contents |
|---|---|
| `HovercraftUniverse/` | The actual game: Visual Studio 2008 solution (`HovercraftUniverse.sln`) with 9 projects, game data, Doxygen config, Inno Setup installer script (`setup.iss`) |
| `HovercraftUniverse/data/` | Runtime game data: levels, entities, controls, GUI config, Lua scripts, sound project, SkyX resources |
| `HovercraftUniverse/bin/` | Working directories for debug/release runs (`.ini` configs, start scripts) and installer staging |
| `GUI/` | Flash source files (`.fla`, ActionScript) for all HUD/menu components, plus exported `.swf` files |
| `art/` | 3ds Max source scenes (hovercrafts, planets, tracks), textures, MaxScript exporter tools (`art/exporter/`), box art |
| `demos/` | Standalone experiments: Lua/LuaBind demo, ZoidCom networking demo (own solution) |
| `particle/` | Particle effect experiments (Ogre `.particle`, materials, textures) |
| `dev-screens/` | Dated development screenshots (2010-03 → 2010-06) |
| `docs/` | The wiki (GitHub Pages) and binary design documents (Word/PDF/Visio), meeting minutes, final presentation |

## Architecture

One executable, three modes (parsed in [`main.cpp`](HovercraftUniverse/HovercraftUniverse/main.cpp)):

```
HovercraftUniverse.exe                          # client (reads HovercraftUniverse.ini)
HovercraftUniverse.exe --server --console       # dedicated server (reads Server.ini)
HovercraftUniverse.exe --host=host:port         # client connecting to a server (default port 2375)
```

The solution builds 8 static libraries plus the game executable:

| Project | Role |
|---|---|
| **HovercraftUniverse** | Game logic: entities (Hovercraft, CheckPoint, Portal, SpeedBoost, PlanetGravity, Asteroid…), their Havok physics bindings ("Phantoms"/actions), their visual "Representations", game states (MainMenu → Lobby → InGame), race rules, loaders, client/server/dedicated-server classes |
| **CoreEngine** | Engine glue: entity/representation managers, game state manager, input (OIS), cameras (race cam, free cam, camera spring), views, config-driven key mapping |
| **Networking** | ZoidCom wrapper: network client/server, entity replication (incl. custom replicators for `Ogre::Vector3`/`Quaternion`/strings), chat client/server, event system |
| **GUI** | Hikari-based Flash overlays: menus, lobby, server browser, HUD widgets (speedometer, timer, position, direction, chat, countdown, results) |
| **Sound** | FMOD Ex / FMOD Designer wrapper (event-based audio, `HovSound.fev`), movable 3D emitters |
| **Scripting** | Lua 5.1 + LuaBind, with Ogre types bound for AI scripts |
| **OgreMax** | Vendored OgreMax scene loader (+ TinyXML) — levels are OgreMax `.scene` files with accompanying Havok `.hkx` physics |
| **Utils** | INI config reader, string utils, timing |
| **Exceptions** | Shared exception hierarchy |

Physics runs on a dedicated Havok thread at a fixed framerate (default 30 fps, `data/engine_settings.cfg`); the server is authoritative and replicates entity state to clients. The content pipeline is 3ds Max → OgreMax exporter (+ custom MaxScript tools in `art/exporter/`) → `.scene`/`.mesh`/`.hkx`.

## Dependencies (as of 2010)

The `HovercraftUniverse/dependencies/` folder is **not in the repository** — it was distributed as downloadable packages, which are still available on the [Google Code Archive downloads page](https://code.google.com/archive/p/uhasseltaacgua/downloads) (notably `Win-dependencies 2.0.rar`, 77 MB, "all dependencies needed for compilation" and `Win-runtime 2.0.rar`, "all the files needed to execute the project").

| Library | Version / era | License | Availability today |
|---|---|---|---|
| [Ogre3D](https://www.ogre3d.org/) | 1.7 era (custom SDK built by the team) | MIT | ✅ Open source, old versions buildable |
| OIS (input) | bundled with Ogre SDK | zlib | ✅ Open source |
| Havok Physics + Animation | 6.6.0 (PC XS, VS2008, Intel free-license program) | Proprietary, binary-only | ⚠️ Program discontinued; only via the archived dependency package |
| ZoidCom (networking) | ~2009 | Closed freeware, binary-only | ⚠️ Company/site gone; only via the archived dependency package |
| [FMOD Ex + FMOD Designer](https://www.fmod.com/) | 4.x (`fmodex_vc`, `fmod_event`) | Proprietary, free for non-commercial | ⚠️ Legacy API, old SDKs archived by FMOD |
| [Hikari](https://code.google.com/archive/p/hikari-library/) (Flash-in-Ogre GUI) | custom-patched (updates on downloads page) | MIT | ⚠️ Source available, but requires **Adobe Flash Player ActiveX** at runtime — discontinued 2021 |
| Lua + LuaBind | 5.1 / 0.9 era | MIT | ✅ Open source (archived team packages exist) |
| Boost | ~1.40 era | BSL | ✅ Open source |
| SkyX (sky rendering) | 0.1 | LGPL | ✅ Source archived on downloads page (`SkyX_0_1.rar`) |
| TinyXML | vendored in `OgreMax/tinyxml` | zlib | ✅ In repo |
| DirectX 9 runtime | June 2010 redist (`d3dx9_42.dll`) | — | ✅ Still installable on Windows 11 |

**Toolchain:** Visual Studio 2008 (VC9), 32-bit Windows build. The GUI sources additionally need Adobe Flash CS-era tooling to rebuild `.fla` → `.swf` (prebuilt `.swf` files are committed).

## Current state of this repository

- **Code: complete.** Full source of the v1.0 game as migrated from SVN (2 commits: initial + "migration from google code"). No known unfinished refactors; the wiki TODO list shows v1.0 shipped with only minor open issues.
- **Binary assets: currently unavailable via git, but fully recovered locally.** 557 files (all `.mesh` models, `.png`/`.jpg` textures, `.swf` GUI files, `.hkx` physics, `.max` sources, design docs — see `.gitattributes`) are stored in **Git LFS, and the LFS budget for this GitHub account is exhausted**. A fresh clone gets ~130-byte pointer stubs instead of real files (clone with `GIT_LFS_SKIP_SMUDGE=1` to avoid checkout errors). All 557 files have since been restored from the Google Code Archive's SVN source dump and verified byte-identical against their committed LFS SHA256 hashes; the corresponding objects now live in `.git/lfs/objects` and can be re-pushed with `git lfs push --all origin` once the LFS budget is restored. The archive packages themselves are mirrored in the untracked `archive-mirror/` folder (SHA1-verified against the archive metadata) pending upload as a GitHub Release.
- **Dependencies: not in repo** (see table above) — recoverable from the Google Code Archive.
- **Buildability on modern machines: not out of the box.** The solution is in VS2008 `.vcproj` format (modern Visual Studio cannot even open it without conversion), and the two most important closed-source libraries (Havok, ZoidCom) are C++ static/import libraries compiled with VC9, which a modern MSVC toolchain cannot link.
- **Runnability of the original binaries: good.** The v1.0 installer (`HovercraftUniverseSetup.exe`, on the archive downloads page) produces a 32-bit DX9 game, which Windows 11 still executes. The main runtime risks are the DirectX 9 helper DLLs (installable) and the Flash ActiveX control required by the Hikari GUI (discontinued, must be sourced from an archive).

## Revival plan

Goal: get the game running and buildable on modern hardware/software with the **least possible functionality change**. The strategy: first restore everything that still exists, then reproduce the original build exactly, and only then (optionally) swap out truly dead components.

> **Progress:** Phase 0 is **done** (all 557 LFS assets restored and hash-verified; all 34 archive packages + the SVN source dump mirrored locally in `archive-mirror/`, pending GitHub Release upload). Phase 1's dependency/runtime layout is **done and scripted** (`scripts/bootstrap-dependencies.ps1`) — the actual VS2008 compile is deliberately deferred (no legacy toolchain on the dev machine; use a VM/sandbox if a period build is ever wanted). Phase 2's CMake conversion is **done** (see `HovercraftUniverse/CMakeLists.txt`).

### Phase 0 — Rescue all artifacts (do this first; nothing else works without it)

1. **Fix the Git LFS situation** so the 557 binary assets are fetchable again: either increase the GitHub LFS budget, or (better long-term) re-host the large assets outside LFS (e.g. a GitHub Release archive, or plain git — the total size is likely manageable) so the repo is self-contained forever.
2. **Mirror the Google Code Archive downloads into this repo's Releases** before they ever disappear. Critical files (all at `https://storage.googleapis.com/google-code-archive-downloads/v2/code.google.com/uhasseltaacgua/<filename>`):
   - `Win-dependencies 2.0.rar` (77 MB) — all compile-time SDKs incl. Havok 6.6, ZoidCom, Hikari, FMOD, Ogre headers/libs
   - `Win-runtime 2.0.rar` (10 MB) — all DLLs/files needed to run
   - `HovercraftUniverseSetup.exe` (56 MB) — the finished v1.0 game incl. all media
   - `LuaBind-Fix.rar`, `luabind.release.lib`, `Win-Hikari-Update_3.rar`, `SkyX_0_1.rar`, `OgrePluginsDLLPackage.zip`, the `Win-Package - *` packages, `menu_art.zip`
3. **Baseline test:** install `HovercraftUniverseSetup.exe` on a modern Windows 11 machine (or a VM) and document exactly what works and what fails. This becomes the reference behavior for "least functionality change".

### Phase 1 — Reproduce the original build (period-correct, minimal risk)

Rebuild v1.0 exactly as documented in the wiki's [CompileAndRun](docs/wiki/CompileAndRun.md) page:

1. Set up a **VS2008 build environment**. VS2008 SP1 still installs and runs on Windows 11 (a VM or Windows Sandbox also works and keeps the host clean).
2. Extract `Win-dependencies 2.0.rar` into `HovercraftUniverse/` (creating `HovercraftUniverse/dependencies/`), apply `Win-Hikari-Update_3.rar` and the LuaBind fix, extract the runtime package into `bin/debug` / `bin/release`.
3. Open `HovercraftUniverse.sln`, build (F7), set the debug working directory to `$(SolutionDir)bin\debug` as per the wiki, run.
4. Runtime environment on modern Windows:
   - Install the **DirectX 9.0c End-User Runtime** (still available from Microsoft) for `d3dx9_42.dll` etc.
   - The Hikari GUI needs the **Flash Player ActiveX control** (`flash.ocx`, 32-bit). An archived final release works — Hikari loads it as a COM control, no browser involved. This is legally awkward but the only zero-code-change option; see Phase 3 for the clean alternative.
   - If the D3D9 renderer misbehaves on a modern GPU, fall back to Ogre's OpenGL render system (`plugins.cfg`), or wrap with [dgVoodoo2](http://dege.freeweb.hu/dgVoodoo2/).
5. Document the whole procedure in `BUILDING.md` and commit any small source fixes needed (expect a handful of header/strictness issues at most, since this is the exact original toolchain).

**Outcome:** a faithful, fully functional game on modern hardware with zero functionality changes — and the safety net for validating every later change.

### Phase 2 — Modernize the build system (still no functionality change)

1. Convert the nine `.vcproj` projects to **CMake** (or upgrade to modern `.vcxproj`), still targeting the **VC9 (v90) platform toolset, Win32**. This keeps ABI compatibility with the Havok/ZoidCom/LuaBind binaries while making the project openable in VS2022 and buildable from the command line.
2. Add a scripted dependency bootstrap (download + extract the Phase-0 mirrored packages into `dependencies/`).
3. Optional: a CI job that proves the build stays green (v90 toolset on a self-hosted or custom-image runner).

### Phase 3 — Modernize the toolchain (in progress; staged so each step leaves a running game)

Compiling with modern MSVC (v143) forces every C++ dependency to be rebuilt or replaced (the VC9 C++ ABI is incompatible). Dependency triage:

- **Survive as-is** (no C++ ABI exposure): `Flash.ocx` (COM/LoadLibrary — but x86-only, so the build stays Win32), FMOD Ex (C API), Lua 5.1, TinyXML.
- **Rebuild/port** (open source): Ogre → 1.12/1.14 (keep the D3D9 render system; materials rely on fixed-function + Cg/HLSL), OgreMax loader and SkyX ported along, OIS, Boost → current, LuaBind → maintained "deboostified" fork, Hikari from archived source.
- **Replace** (closed-source VC9 binaries): ZoidCom → ENet behind a ZoidCom-API-compatible shim; Havok 6.6 → Jolt, including a new collision pipeline (level collision lives in proprietary `.hkx` — regenerate from the OgreMax `.scene`/`.mesh` geometry).

Execution order:

1. **Reference build with VC9** (Windows Sandbox + VS2008 Express from Microsoft's still-live ISO link; automation in `toolchain/sandbox/`) — the baseline for detecting behavior drift.
2. **Modern-MSVC port of everything except physics/networking** (VS2022 Build Tools, Ogre 1.1x, rebuilt open-source deps; Havok/ZoidCom-dependent code temporarily stubbed) — proves the codebase compiles with current tools.
3. **ZoidCom → ENet shim** — restores multiplayer on the modern build.
4. **Havok → Jolt** *(on hold)* — the long pole: physics port + collision-from-mesh pipeline + feel tuning against the reference build.
5. Later options: x64 (requires replacing Flash.ocx with Ruffle first), FMOD Core, D3D11/GL3+ renderers.

**Deliberate remaster-side change:** the physics tick was raised from the
original 30 Hz to 60 Hz, with per-step tuning constants re-derived against a
documented 30 Hz reference so the feel doesn't drift, and rendering was
decoupled from the physics tick (dt-aware smoothing instead of snapping) so
motion stays smooth on high-refresh displays. This is explicitly *not*
faithful-port behavior — see `docs/porting/timing-and-smoothing.md` for the
full rationale, math and compile-only verification status.

### Phase 3 (original sketch) — replace dead components

Only two components are genuinely *dead* rather than merely old. If long-term sustainability matters more than binary fidelity, replace them one at a time, validating against the Phase-1 build:

1. **Flash GUI (highest value, lowest risk):** remove the Flash ActiveX dependency. Best fit: [Ruffle](https://ruffle.rs/) (open-source Flash emulator, embeddable) can play the existing committed `.swf` files — same assets, same look, no Adobe dependency. Alternative: reimplement the ~10 HUD widgets as Ogre overlays (the `.psd`/`.png` sources are in `GUI/`).
2. **Compiler unlock (big step, only if ever needed):** moving to a modern MSVC/x64 toolchain requires replacing the two VC9-ABI-locked closed libraries:
   - **Havok 6.6 → [Jolt](https://github.com/jrouwe/JoltPhysics) or Bullet.** The physics surface is well-contained (`AbstractHavokWorld`, `HavokEntity`, the phantom/action classes in the game project) but tuning-sensitive — expect gameplay-feel drift; keep the Phase-1 build as the reference.
   - **ZoidCom → [ENet](https://github.com/lsalzman/enet) + a small replication layer.** The `Networking` project already isolates replication (custom replicators, event parsers), so the port is mechanical but non-trivial.
   - With those replaced, Ogre can move to 1.12/1.14 (API-compatible line), Lua/Boost/LuaBind to maintained versions, and FMOD Ex to FMOD Core — all modest, mechanical changes.

### Suggested order of work

| Step | Effort | Risk | Unblocks |
|---|---|---|---|
| 0. Rescue assets & packages | hours | none | everything |
| 1. Period-correct VS2008 build | days | low | a running game |
| 2. CMake + v90 toolset | days | low | modern IDE/CI |
| 3a. Ruffle for GUI | days–weeks | medium | no Flash dependency |
| 3b. Physics/networking swap | weeks–months | high | fully modern toolchain |

Phases 0–2 deliver "runnable and buildable on modern hardware with the least functionality change" — Phase 3 is only for making it live forever.

## Historical documentation

- [Compile & Run](docs/wiki/CompileAndRun.md) — original build instructions
- [Libraries](docs/wiki/Libraries.md) — dependency choices and rationale
- [Planning](docs/wiki/Planning.md) — work packages and who built what
- [GUI](docs/wiki/GUI.md) — HUD architecture and Hikari/Flash setup
- [Scripting](docs/wiki/Scripting.md) — Lua/Python/Squirrel evaluation
- [Installer](docs/wiki/Installer.md) — Inno Setup packaging
- `docs/design/` — design documents, meeting minutes and the final presentation
