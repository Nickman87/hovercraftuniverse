# Modern build setup (v143 / x86-windows)

This describes the **separate, modern** CMake tree under `modern/` --
independent of the faithful VC9 conversion under `HovercraftUniverse/`, which
must not be touched. The modern tree targets MSVC v143 (VS2022), Win32/x86,
with dependencies from [vcpkg](https://vcpkg.io).

## Prerequisites

- **Visual Studio 2022 Build Tools**, with the "Desktop development with
  C++" workload (v143 toolset, x86 target support).
- **CMake** >= 3.20 (bundled with recent VS2022 installs, or standalone).
- **7-Zip** (`7z.exe`) -- used to extract the legacy DirectX SDK installer.
  Install with `winget install 7zip.7zip` if not already present. The
  bootstrap script never installs 7-Zip itself; it aborts with this hint if
  missing.
- **git** -- used to clone vcpkg if it isn't already present.
- Network access to `github.com` (vcpkg + ports), the vcpkg-configured
  download mirrors for each port's upstream source, and
  `download.microsoft.com` (DirectX SDK installer).

vcpkg itself is expected at `C:\Users\dirk\vcpkg` by default (overridable);
it does not need to be installed manually -- the bootstrap script clones and
bootstraps it if absent.

## Running the bootstrap script

```powershell
.\scripts\bootstrap-modern-deps.ps1
```

This is idempotent and safe to re-run. It performs four phases:

1. **vcpkg + 7-Zip** -- ensures vcpkg exists (cloning + bootstrapping if
   needed) and that 7-Zip is on the machine.
2. **DirectX SDK bits** -- ensures `toolchain\dxsdk\DXSDK\Lib\x86\d3dx9.lib`
   and the matching header exist (see below for why).
3. **vcpkg install** -- installs Ogre and the other required ports for the
   `x86-windows` triplet.
4. **Verification** -- prints a checklist of expected installed headers/
   libs/plugins and exits non-zero if anything required is missing.

Useful parameters:

- `-VcpkgRoot <path>` -- use a vcpkg tree somewhere other than
  `C:\Users\dirk\vcpkg`.
- `-SkipDxsdkDownload` -- don't fetch the DirectX SDK installer; fail if it
  (or the already-extracted `d3dx9.lib`) isn't already present locally.
- `-MaxConcurrency <n>` -- passed through as `VCPKG_MAX_CONCURRENCY`
  (default 4).

## Why FreeImage is excluded from the Ogre build

Ogre's vcpkg port defaults to a `freeimage` feature for image loading. On
this x86 toolchain, FreeImage's dependency chain pulls in `openjph`, whose
AVX2 code hits an **MSVC internal compiler error (C1001)** during
compilation. The fix is not to fix openjph, but to avoid needing it: this
tree installs Ogre as

```
ogre[core,d3d9,overlay,zip]
```

explicitly, i.e. *without* the default feature set. Ogre falls back to its
built-in stb-based image codecs, which is sufficient for this game's assets.
`d3d9` is required (the render system that keeps the fixed-function pipeline
used by 13 of the game's 16 materials), `overlay` is required (HUD/GUI),
`zip` is required (resource archives, `.zip`-packed media).

## Why the (legacy) DirectX SDK is needed

Building Ogre's `RenderSystem_Direct3D9` requires `d3dx9.lib` plus the D3DX9
headers. These were removed from the Windows SDK years ago and only ship in
the legacy **DirectX SDK (June 2010)** -- `d3d9.lib` alone (still present in
modern Windows SDKs) is not enough.

The bootstrap script downloads the official Microsoft installer
(`DXSDK_Jun10.exe`, 599,455,936 bytes, Microsoft-signed) from
`https://download.microsoft.com/download/A/E/7/AE743F1F-632B-4809-87A9-AA1BB3458E31/DXSDK_Jun10.exe`,
verifies its Authenticode signature is `Valid`, and then **extracts** (never
runs/installs) just the two subtrees needed:

```
7z x -y -o<repo>\toolchain\dxsdk <exe> "DXSDK\Include\*" "DXSDK\Lib\x86\*"
```

giving `<repo>\toolchain\dxsdk\DXSDK\{Include,Lib\x86}`. The script points
the `DXSDK_DIR` environment variable (with a trailing backslash) at that
folder before invoking `vcpkg install`, because Ogre's `FindDirectX9.cmake`
honours that variable. `modern/CMakeLists.txt` exposes the same location as
the cached `HU_DXSDK_DIR` variable (defaulting to
`modern/../toolchain/dxsdk/DXSDK`) for when D3D9-linked targets are wired up.

The full DirectX SDK is intentionally **not installed** on the machine --
only these two subtrees are extracted into the repo's `toolchain/` folder,
which is git-ignored.

## Vendoring TODOs (not yet handled by this bootstrap)

These dependencies are not available (in the right version, or at all) from
vcpkg, and are not yet wired into the modern tree:

- **Lua 5.1** -- vcpkg's `lua` port is 5.5, which is API/ABI-incompatible
  with the game's Lua 5.1-era scripts and LuaBind bindings. Lua 5.1 will be
  vendored separately (built from source or a vendored prebuilt).
- **LuaBind** -- no vcpkg port exists. A maintained "deboostified" fork
  (LuaBind without a hard Boost dependency) will be vendored separately.
- **Hikari** (the in-game GUI library used for menus) -- will be built from
  its archived source separately; see `scripts/bootstrap-dependencies.ps1`
  for how it was originally distributed for the VC9 tree.
- **Havok** and **ZoidCom** -- out of scope for the modern tree for now
  (physics and networking middleware respectively; the VC9 tree's originals
  are proprietary/end-of-life and need a separate replacement strategy, not
  just a recompile).

## Configuring and building the modern tree

Once `scripts/bootstrap-modern-deps.ps1` has completed successfully, use the
`x86` CMake preset defined in `modern/CMakePresets.json` (VS2022 v143,
Win32, wired to the vcpkg toolchain file and `x86-windows` triplet):

```powershell
cmake --preset x86
cmake --build --preset x86
```

This configures into `C:/hu-modern-build` (out-of-tree) and builds the
Debug configuration. `modern/CMakeLists.txt` currently builds only the
`hu_exceptions` and `hu_utils` static libraries (proving those two folders
compile clean under v143); it does not yet define Ogre-dependent targets --
it only probes `find_package(OGRE CONFIG QUIET)` and reports via
`message(STATUS ...)` whether vcpkg's Ogre is visible, as a checkpoint for
the next step once Ogre finishes building.
