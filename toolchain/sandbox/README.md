# Windows Sandbox: reference VS2008 (VC9) build

Produces a reference **32-bit Release** build of HovercraftUniverse using the
real Visual C++ 2008 Express (SP1) toolchain, inside a disposable Windows
Sandbox instance. Nothing is installed on the host.

## One-time setup

1. Enable Windows Sandbox: Settings -> Apps -> Optional Features -> More
   Windows Features -> check "Windows Sandbox" -> reboot when prompted.
   (Requires Windows 11 Pro/Enterprise/Education with virtualization enabled
   in the BIOS.)
2. Make sure the installer ISO is present at:
   `toolchain\VS2008ExpressWithSP1ENUX1504728.iso`
   (relative to the repo root — it's already there if you're reading this
   from a checkout that includes it).

## Running the build

1. Double-click `toolchain\sandbox\build-vs2008.wsb`.
2. Windows Sandbox launches, maps this repo read-write at `C:\repo`, and
   automatically runs `sandbox-build.ps1` in a PowerShell window that stays
   open (`-NoExit`) so you can watch progress and see any errors.
3. The script will, in order: enable .NET Framework 3.5, mount the ISO,
   silently install VC++ 2008 Express, wait for the install to finish, then
   build `HovercraftUniverse.sln` for `Release|Win32` (and `Debug|Win32` as a
   best-effort extra).
4. First run takes a while — the VS2008 Express install alone can take
   10-20 minutes inside the sandbox.

## Where things land

Everything the script writes goes through the `C:\repo` mapping, so it
appears on the host under `toolchain\sandbox\`:

- `build-<timestamp>.log` — full transcript of the run (every phase, all
  installer/compiler output).
- `artifacts\HovercraftUniverse-vc9-release.exe` — copy of the built Release
  executable, for convenience.

The actual build output also stays in the normal project locations (visible
on the host since the whole repo is mapped in):

- `HovercraftUniverse\bin\release\HovercraftUniverse.exe` (the 32-bit Release
  binary — this is the reference build artifact)
- `HovercraftUniverse\lib\Release\*.lib`

## When you're done

Just close the sandbox window (or the PowerShell window and then the
sandbox). Everything *inside* the sandbox — the installed VS2008 Express,
temp files, registry changes — evaporates. Only what was written through the
`C:\repo` mapping (the log and the artifacts folder, plus the normal
bin/lib build output) survives on the host.

## If it fails

- "ISO not found" — put `VS2008ExpressWithSP1ENUX1504728.iso` at
  `toolchain\VS2008ExpressWithSP1ENUX1504728.iso` and try again.
- "Dependencies folder not found" — run `scripts\bootstrap-dependencies.ps1`
  on the host first, then relaunch the sandbox.
- Anything else — check the `build-<timestamp>.log` file named in the
  FAILED banner; it has the full transcript including installer and
  compiler/linker output.
