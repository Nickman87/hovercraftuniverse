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
3. Switch the renderer to OpenGL (modern machines lack the DX9 helper DLLs):
   in `game/app/data/plugins_release.cfg`, comment out the Direct3D lines and
   keep OpenGL:

   ```
   # Plugin=RenderSystem_Direct3D9
    Plugin=RenderSystem_GL
   ```

4. (Optional, skips the startup config dialog) create `game/app/ogre.cfg`:

   ```
   Render System=OpenGL Rendering Subsystem
   [OpenGL Rendering Subsystem]
   Colour Depth=32
   Display Frequency=N/A
   FSAA=0
   Full Screen=No
   RTT Preferred Mode=FBO
   VSync=No
   Video Mode=1024 x 768
   ```

5. Run `HovercraftUniverse.exe` from `game/app/`. Modes:

   ```
   HovercraftUniverse.exe                      client (main menu → single/multiplayer)
   HovercraftUniverse.exe --server --console   dedicated server (Server.ini, port 2375)
   HovercraftUniverse.exe --host=host:port     client joining a server
   ```

## What was verified (2026-07-29, Windows 11 Pro)

- **Dedicated server**: full startup — Ogre 1.7.0RC1 on OpenGL, all plugins,
  resource groups, ZoidCom chat/lobby entities, `[Server]: Ready for incoming
  connections`.
- **Client**: stable for minutes, window created, all materials/particles/fonts
  parsed, FMOD event bank (`HovSound.fev`) loaded, and the Flash main menu
  (Singleplayer / Multiplayer / Quit) **renders and responds** — Hikari loads
  `data/Flash.ocx` directly via `DllGetClassObject`, so no Flash installation
  or COM registration is needed. The discontinuation of Flash Player does not
  block this game.
- In-game racing: not yet tested (needs a human at the keyboard).

## Known environmental notes

- **Direct3D9 renderer**: fails to load because `d3dx9_42.dll` isn't present on
  modern systems. Either use OpenGL (verified) or place 32-bit `d3dx9_42.dll`
  (DirectX 9.0c End-User Runtime, June 2010) next to the exe.
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
