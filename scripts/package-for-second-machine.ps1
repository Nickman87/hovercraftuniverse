<#
.SYNOPSIS
    Packages the runtime into the (Synology-synced) project folder so a second
    machine can run it for a real multiplayer test.

.DESCRIPTION
    C:\hu-modern-run is outside the synced project folder, and it is not
    reproducible from local-game/ -- it holds hand-fixed runtime config and
    DLLs (see CLAUDE.md). This copies it into dist\second-machine\runtime\
    inside the project folder, which syncs to the other machines, and writes an
    install.ps1 + README next to it.

    Two things the copy adds that C:\hu-modern-run does not have:

      * The x86 DEBUG CRT (msvcp140d.dll, vcruntime140d.dll, ucrtbased.dll).
        This is a Debug|Win32 build, so the exe imports the debug runtime,
        which ships only with Visual Studio and is NOT part of the
        redistributable. Without these the game dies at launch on any machine
        without VS -- which is exactly what a second test machine is.

      * install.ps1, which copies runtime\ to C:\hu-modern-run on the target.
        Do not run the game straight from the synced folder: these disks are
        slow and network-backed (CLAUDE.md), and scripts\run-test-session.ps1
        expects C:\hu-modern-run anyway.

    dist\ is gitignored: it is ~225 MB of build output that belongs in the
    sync, not in the repo.

.PARAMETER Force
    Overwrite an existing dist\second-machine without asking.

.EXAMPLE
    .\scripts\package-for-second-machine.ps1
#>
[CmdletBinding()]
param(
    [switch] $Force
)

$ErrorActionPreference = 'Stop'

$RunDir  = 'C:\hu-modern-run'
$Root    = Split-Path -Parent $PSScriptRoot
$Dist    = Join-Path $Root 'dist\second-machine'
$Runtime = Join-Path $Dist 'runtime'

function Step($m) { Write-Host "==> $m" -ForegroundColor Cyan }
function Fail($m) { Write-Host "!!! $m" -ForegroundColor Red; exit 1 }

if (-not (Test-Path $RunDir)) { Fail "$RunDir not found -- build and deploy first (scripts\build-and-deploy.ps1)." }
if (Get-Process -Name 'HovercraftUniverse' -ErrorAction SilentlyContinue) {
    Fail 'The game is running. Stop it first -- copying a runtime mid-run gives you a torn snapshot.'
}

if (Test-Path $Dist) {
    if (-not $Force) {
        Write-Host "$Dist already exists. Re-run with -Force to replace it." -ForegroundColor Yellow
        exit 1
    }
    Step "Replacing existing $Dist"
    Remove-Item $Dist -Recurse -Force
}
New-Item -ItemType Directory $Runtime -Force | Out-Null

# --- 1. The runtime itself -------------------------------------------------
# Excluded: testruns\ (per-run log snapshots, can be gigabytes and mean nothing
# on another machine), stale stdout captures, screenshots, and the generated
# Client<N>.ini files (run-test-session.ps1 regenerates those from
# HovercraftUniverse.ini on whichever machine it runs).
Step "Copying runtime from $RunDir"
$excludeDirs  = @('testruns')
$excludeFiles = @('*.log', '*.txt', 'screenshot.png', 'Client*.ini')

$robocopyArgs = @(
    $RunDir, $Runtime, '/E', '/NFL', '/NDL', '/NJH', '/NJS', '/NP',
    '/XD'
) + $excludeDirs + @('/XF') + $excludeFiles

$null = & robocopy @robocopyArgs
# robocopy uses exit codes 0-7 for success (8+ is a real failure).
if ($LASTEXITCODE -ge 8) { Fail "robocopy failed with exit code $LASTEXITCODE" }

# --- 2. The debug CRT ------------------------------------------------------
Step 'Adding the x86 debug CRT (not redistributable -- see README)'

# CAREFUL: match '*\x86\*', never '*x86*'. Both of these live under
# "C:\Program Files (x86)\...", so a bare '*x86*' matches the x64 copy just as
# happily as the x86 one -- and x64 sorts first, so -First 1 silently picks the
# wrong architecture. That produced a package whose exe died on launch with
# 0xC000007B (STATUS_INVALID_IMAGE_FORMAT) and nothing in any log. The
# architecture assertion below exists so it can never happen quietly again.
$crt = @{}
$vcRedist = Get-ChildItem 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Redist\MSVC' -Directory -ErrorAction SilentlyContinue |
            Where-Object { $_.Name -match '^\d+\.' } | Sort-Object Name -Descending | Select-Object -First 1
if ($vcRedist) {
    foreach ($dll in @('msvcp140d.dll', 'vcruntime140d.dll')) {
        $hit = Get-ChildItem (Join-Path $vcRedist.FullName 'debug_nonredist') -Recurse -Filter $dll -ErrorAction SilentlyContinue |
               Where-Object { $_.FullName -like '*\x86\*' } | Select-Object -First 1
        if ($hit) { $crt[$dll] = $hit.FullName }
    }
}
$ucrt = Get-ChildItem 'C:\Program Files (x86)\Windows Kits\10\bin' -Recurse -Filter 'ucrtbased.dll' -ErrorAction SilentlyContinue |
        Where-Object { $_.FullName -like '*\x86\*' } | Sort-Object FullName -Descending | Select-Object -First 1
if ($ucrt) { $crt['ucrtbased.dll'] = $ucrt.FullName }

foreach ($name in @('msvcp140d.dll', 'vcruntime140d.dll', 'ucrtbased.dll')) {
    if ($crt.ContainsKey($name)) {
        Copy-Item $crt[$name] (Join-Path $Runtime $name) -Force
        Write-Host "    + $name"
    } else {
        Write-Host "    ! $name NOT FOUND -- the game will not start on a machine without Visual Studio" -ForegroundColor Yellow
    }
}

# --- 2b. Assert every binary is x86 ---------------------------------------
# This is a Win32 build. A single x64 binary in here makes the exe fail at
# load with 0xC000007B (STATUS_INVALID_IMAGE_FORMAT): a Windows error dialog,
# no log output anywhere, and nothing for run-test-session.ps1 to collect --
# so it is worth several seconds to catch it on the machine that can fix it.
Step 'Verifying every packaged binary is x86'

function Get-PeMachine([string] $path) {
    try {
        $fs = [IO.File]::OpenRead($path)
        $br = New-Object IO.BinaryReader($fs)
        $fs.Position = 0x3C
        $fs.Position = $br.ReadInt32()
        if ($br.ReadUInt32() -ne 0x00004550) { $br.Close(); return 'notPE' }  # 'PE\0\0'
        $machine = $br.ReadUInt16()
        $br.Close()
        switch ($machine) {
            0x014C  { 'x86' }
            0x8664  { 'x64' }
            0xAA64  { 'arm64' }
            default { '0x{0:X}' -f $machine }
        }
    } catch { 'unreadable' }
}

$wrong = @()
foreach ($bin in (Get-ChildItem $Runtime -Recurse -Include '*.dll', '*.exe' -File)) {
    $arch = Get-PeMachine $bin.FullName
    if ($arch -ne 'x86') { $wrong += [pscustomobject]@{ Name = $bin.Name; Arch = $arch } }
}
if ($wrong.Count -gt 0) {
    $wrong | Format-Table -AutoSize | Out-String | Write-Host
    Fail "$($wrong.Count) packaged binary/binaries are not x86 (see above). This build is Win32; the game would die at launch with 0xC000007B and no log output."
}
Write-Host '    all x86'

# --- 3. install.ps1 on the target -----------------------------------------
$installer = @'
<#
.SYNOPSIS
    Installs this packaged runtime to C:\hu-modern-run on THIS machine.
.DESCRIPTION
    Run once on the second machine, from the synced project folder. Afterwards
    use scripts\run-test-session.ps1 exactly as on the build machine.
#>
[CmdletBinding()]
param([string] $Destination = 'C:\hu-modern-run')

$ErrorActionPreference = 'Stop'
$src = Join-Path $PSScriptRoot 'runtime'

if (-not (Test-Path $src)) { Write-Host "!!! runtime\ not found next to this script." -ForegroundColor Red; exit 1 }
if (Get-Process -Name 'HovercraftUniverse' -ErrorAction SilentlyContinue) {
    Write-Host '!!! The game is running here. Stop it first.' -ForegroundColor Red; exit 1
}

Write-Host "==> Installing to $Destination" -ForegroundColor Cyan
$null = & robocopy $src $Destination /E /NFL /NDL /NJH /NJS /NP
if ($LASTEXITCODE -ge 8) { Write-Host "!!! robocopy failed ($LASTEXITCODE)" -ForegroundColor Red; exit 1 }

Write-Host "==> Done. Next:" -ForegroundColor Green
Write-Host '    .\scripts\run-test-session.ps1 -Mode join -HostAddress <host-ip> -Clients 1 -Seconds 0'

# robocopy returns 1-7 on success; do not let that leak out as failure.
exit 0
'@
Set-Content -Path (Join-Path $Dist 'install.ps1') -Value $installer -Encoding utf8

# --- 4. README -------------------------------------------------------------
$readme = @'
# Second-machine test package

Generated by `scripts\package-for-second-machine.ps1`. Everything here is
build output -- do not edit it, regenerate it.

## On the second machine

From this synced project folder:

```powershell
.\dist\second-machine\install.ps1
```

That copies `runtime\` to `C:\hu-modern-run` locally. Run it from a local disk,
not from the synced folder: these drives are network-backed and slow, and
`scripts\run-test-session.ps1` expects `C:\hu-modern-run`.

Then join the host (see `docs\porting\multiplayer-testing.md` for the full
procedure, including the firewall rule the HOST needs for UDP 2375 *and* 2377):

```powershell
.\scripts\run-test-session.ps1 -Mode join -HostAddress <host-ip> -Clients 1 -Seconds 0
```

Or use the GUI: Multiplayer -> Join game -> type the host IP -> OK. Same code
path.

Collect logs on both machines when you are done:

```powershell
.\scripts\run-test-session.ps1 -Collect
```

## About the debug CRT

`msvcp140d.dll`, `vcruntime140d.dll` and `ucrtbased.dll` are bundled here.

This is a `Debug|Win32` build, so the exe imports the *debug* CRT. Microsoft
ships that only with Visual Studio and explicitly excludes it from the
redistributable, so a machine without VS cannot start the game without these.
Copying them between your own machines for testing is fine; shipping this
package to anyone else is not. The real fix, when the port gets there, is a
Release build -- which then needs only the ordinary VC++ redistributable.

## What was left out

`testruns\` (per-run log snapshots -- large and meaningless on another
machine), stale stdout captures, screenshots, and the generated `Client<N>.ini`
files, which `run-test-session.ps1` recreates from `HovercraftUniverse.ini`
wherever it runs.
'@
Set-Content -Path (Join-Path $Dist 'README.md') -Value $readme -Encoding utf8

# --- Report ----------------------------------------------------------------
$s = Get-ChildItem $Dist -Recurse -File | Measure-Object -Property Length -Sum
Step ("Packaged {0:N0} files, {1:N1} MB -> {2}" -f $s.Count, ($s.Sum / 1MB), $Dist)
Write-Host ''
Write-Host 'Wait for Synology Drive to finish syncing, then on the second machine run:' -ForegroundColor Green
Write-Host '    .\dist\second-machine\install.ps1'

# robocopy sets $LASTEXITCODE to 1-7 on SUCCESS (1 = files copied), which would
# otherwise leak out as this script's exit code and make a good run look failed.
exit 0
