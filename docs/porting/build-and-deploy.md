# Build and deploy: the canonical procedure

**Use `scripts\build-and-deploy.ps1`. Do not hand-roll cmake invocations and
do not hand-copy binaries.** This page exists so nobody -- human or agent --
has to reverse-engineer the procedure from `CMakeLists.txt` again.

```powershell
.\scripts\build-and-deploy.ps1
```

That is the whole thing for the common case. It configures if needed, builds,
verifies the exe is actually fresh, and copies exe + pdb + linker map into the
runtime directory.

## The three directories

| directory | what it is | in git? |
|---|---|---|
| `<repo>` | source | yes |
| `C:\hu-modern-build` | CMake binary dir, disposable | no |
| `C:\hu-modern-run` | runtime dir the game is actually launched from | no |

`C:\hu-modern-build` is disposable but **expensive** -- a from-scratch
configure + full build is long. Do not delete it casually. If it goes missing,
the script reconfigures automatically; you just pay for a full rebuild.

`C:\hu-modern-run` is **not** disposable. It holds hand-fixed runtime config
(`data\plugins_release.cfg`, `data\resources.cfg`, `data\engine_settings.cfg`),
the Ogre 14 DLLs, `OgreCoreMedia`, and `D3DCompiler_43.dll`. See
`first-run.md` sections 4 and 9. Never recreate it by re-copying
`local-game/` -- that reintroduces the config contamination documented in
section 9.8.

## Options

| invocation | when |
|---|---|
| `.\scripts\build-and-deploy.ps1` | default; build everything, deploy |
| `... -Target HovercraftUniverse` | iterating; builds one target, much faster |
| `... -NoDeploy` | just check it compiles |
| `... -Clean` | wipe the build dir and configure from scratch. Slow. Last resort. |

## What the script guarantees

1. **It refuses to run while the game is running.** A running exe cannot be
   overwritten; without this check you get a half-deployed runtime dir and a
   very confusing test session.
2. **It configures only when needed** -- presence of
   `C:\hu-modern-build\CMakeCache.txt` is the test.
3. **It warns if the "successful" build produced a stale exe.** A zero exit
   code with an untouched binary is a real failure mode and has burned time
   before.
4. **It deploys the pdb and the linker map alongside the exe.** The map is
   required to symbolize crash offsets from the Application event log (see
   `first-run.md`); an exe deployed without its map is a debugging dead end.
5. **It never scans the build tree recursively without a depth bound.** These
   volumes are slow. Unbounded scans have wedged in uninterruptible I/O here.

## Underlying commands, for reference only

Presets live in `modern/CMakePresets.json`, so cmake must be invoked with the
working directory set to `modern/`:

```powershell
cmake --preset x86          # configures into C:/hu-modern-build
cmake --build --preset x86  # Debug|Win32
```

Preset `x86` = Visual Studio 17 2022 generator, Win32 architecture, vcpkg
toolchain at `C:/Users/dirk/vcpkg/scripts/buildsystems/vcpkg.cmake`, triplet
`x86-windows`, binary dir `C:/hu-modern-build`, configuration `Debug`.

The Visual Studio generator places top-level targets in
`C:\hu-modern-build\Debug\`.

First-time setup of the dependencies (vcpkg ports, the extracted legacy
DirectX SDK) is a separate, one-off concern -- see `modern-build-setup.md`.

## Launching

Launch from `C:\hu-modern-run`; the exe resolves `data\` relative to its own
directory via `Application::parseIni()`. Test protocol (who launches, who
clicks) is in `CLAUDE.md`.
