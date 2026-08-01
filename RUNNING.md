# Running Hovercraft Universe v1.0 on modern Windows

Verified working on Windows 11 (July 2026): the unmodified 2010 binaries run —
engine, sound, networking **and** the Flash-based GUI. This document records the
exact recipe.

## TL;DR

1. Get `HovercraftUniverseSetup.exe` (Google Code Archive; mirrored in
   `archive-mirror/` — see `scripts/bootstrap-dependencies.ps1` for the
   download base URL, or the project GitHub Release once published).
2. Don't install it — unpack it with [innoextract](https://constexpr.org/innoextract/):

   ```bash
   innoextract -s -d game HovercraftUniverseSetup.exe
   ```

   The playable game lands in `game/app/` (exe, DLLs, `Flash.ocx`, all data).
3. Supply the two DirectX 9 helper DLLs the 2010 build links against
   (modern Windows no longer ships them). Download the official
   **DirectX End-User Runtimes (June 2010)** redistributable from Microsoft
   (`directx_Jun2010_redist.exe`, Microsoft-signed), extract with 7-Zip:
   `Aug2009_d3dx9_42_x86.cab` → `d3dx9_42.dll` and
   `Aug2009_D3DCompiler_42_x86.cab` → `D3DCompiler_42.dll` (both x86), and copy
   both DLLs into `game/app/` **and** `game/app/data/` (the game changes its
   working directory to `data/`, and the D3D9 render plugin resolves its
   imports from there). No system-wide DirectX install needed.

4. (Optional, skips the startup config dialog) create `game/app/ogre.cfg` and
   `game/app/data/ogre.cfg`:

   ```
   Render System=Direct3D9 Rendering Subsystem
   [Direct3D9 Rendering Subsystem]
   Full Screen=No
   VSync=No
   Video Mode=1024 x 768 @ 32-bit colour
   ```

5. Run `HovercraftUniverse.exe` from `game/app/`. Modes:

   ```
   HovercraftUniverse.exe                      client (main menu → single/multiplayer)
   HovercraftUniverse.exe --server --console   dedicated server (Server.ini, port 2375)
   HovercraftUniverse.exe --host=host:port     client joining a server
   ```

## What was verified (2026-07-29, Windows 11 Pro)

- **Dedicated server**: full startup — Ogre 1.7.0RC1, all plugins, resource
  groups, ZoidCom chat/lobby entities, `[Server]: Ready for incoming
  connections`.
- **Client**: Flash main menu (Singleplayer / Multiplayer / Quit) renders and
  responds — Hikari loads `data/Flash.ocx` directly via `DllGetClassObject`,
  so no Flash installation or COM registration is needed. The discontinuation
  of Flash Player does not block this game.
- **In-game racing: works** — lobby → countdown → racing with bots, on the
  Direct3D9 renderer (tested on Intel Arc Pro graphics, Windows 11).

## Known environmental notes

- **OpenGL renderer**: reaches the main menu and lobby, but **races crash**
  with `OGRE EXCEPTION(2:InvalidParametersException): Named constants have not
  been initialised` — the SkyX 0.1 sky shaders are HLSL-only (`vs_1_1`/
  `ps_2_0`), which the GL render system cannot compile. Use Direct3D9 (the
  renderer the game shipped with); OpenGL is only a fallback for poking at the
  menus without the DX9 DLLs.
- **`d3dx9_42.dll` placement matters**: exe directory alone is NOT enough —
  the D3D9 render plugin is loaded while the process' working directory is
  `data/`, so the DLLs must (also) be there. Symptom otherwise:
  `OGRE EXCEPTION(7:InternalErrorException): Could not load dynamic library
  plugins_release\RenderSystem_Direct3D9`.
- **Direct3D10 renderer**: was already disabled by default in 2010; leave it off.
- Config locations: client `HovercraftUniverse.ini`, server `Server.ini`,
  embedded singleplayer server `data/SingleplayerServer.ini`, physics/gravity
  `data/engine_settings.cfg`. Logs are written into `data/`.
- The game is a 32-bit (Win32) executable; Windows 11 runs it via WOW64 without
  any compatibility settings.

## Building from source

See `README.md` (revival plan) — the CMake conversion lives in
`HovercraftUniverse/CMakeLists.txt` and dependencies are reconstructed by
`scripts/bootstrap-dependencies.ps1`. Linking requires the VC9 (v90) toolset;
the prebuilt exe above is the zero-toolchain way to run the game.
