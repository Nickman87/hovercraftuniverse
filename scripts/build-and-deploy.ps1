<#
.SYNOPSIS
    Builds the modern tree and deploys the result to the runtime directory.

.DESCRIPTION
    This is THE canonical way to build and deploy. Do not hand-roll cmake
    invocations or hand-copy binaries -- use this script so every build is
    identical and every deploy is complete.

    Steps:
      1. Refuse to run if the game is currently running (a running exe cannot
         be overwritten, and a half-deployed runtime dir is worse than none).
      2. Configure C:\hu-modern-build from the `x86` preset if it is missing.
      3. Build the `x86` build preset (Debug|Win32).
      4. Locate the freshly built HovercraftUniverse.exe.
      5. Verify it is actually newer than when the build started -- a "success"
         exit code with a stale exe means the build silently did nothing.
      6. Copy exe + pdb + linker map into C:\hu-modern-run.
      7. Print the deployed timestamps.

.PARAMETER Target
    Build only this CMake target instead of everything. Much faster when
    iterating on one library.

.PARAMETER Clean
    Delete the build directory first and configure from scratch.

.PARAMETER NoDeploy
    Build only; skip the copy into the runtime directory.

.EXAMPLE
    .\scripts\build-and-deploy.ps1
.EXAMPLE
    .\scripts\build-and-deploy.ps1 -Target HovercraftUniverse
#>
[CmdletBinding()]
param(
    [string] $Target,
    [switch] $Clean,
    [switch] $NoDeploy
)

$ErrorActionPreference = 'Stop'

$RepoRoot  = Split-Path -Parent $PSScriptRoot
$ModernDir = Join-Path $RepoRoot 'modern'
$BuildDir  = 'C:\hu-modern-build'
$RunDir    = 'C:\hu-modern-run'
$ExeName   = 'HovercraftUniverse.exe'

function Step($msg) { Write-Host "==> $msg" -ForegroundColor Cyan }
function Fail($msg) { Write-Host "!!! $msg" -ForegroundColor Red; exit 1 }

# --- 1. Nothing may be holding the exe open ---------------------------------
$running = @(Get-Process -Name 'HovercraftUniverse' -ErrorAction SilentlyContinue)
if ($running.Count -gt 0) {
    Fail "$($running.Count) HovercraftUniverse process(es) still running (PIDs: $($running.Id -join ', ')). Stop them first -- the deploy would fail or, worse, half-succeed."
}

if ($Clean -and (Test-Path $BuildDir)) {
    Step "Removing $BuildDir (-Clean)"
    Remove-Item $BuildDir -Recurse -Force
}

# --- 2. Configure if needed -------------------------------------------------
if (-not (Test-Path (Join-Path $BuildDir 'CMakeCache.txt'))) {
    Step "Configuring (preset x86) -- build dir absent or not configured"
    Push-Location $ModernDir
    try {
        cmake --preset x86
        if ($LASTEXITCODE -ne 0) { Fail "cmake --preset x86 failed (exit $LASTEXITCODE)" }
    } finally { Pop-Location }
} else {
    Step "Build dir already configured: $BuildDir"
}

# --- 3. Build ---------------------------------------------------------------
$buildStart = Get-Date
Step ("Building" + $(if ($Target) { " target '$Target'" } else { " everything" }) + " (Debug|Win32)")
Push-Location $ModernDir
try {
    if ($Target) {
        cmake --build --preset x86 --target $Target
    } else {
        cmake --build --preset x86
    }
    if ($LASTEXITCODE -ne 0) { Fail "build failed (exit $LASTEXITCODE)" }
} finally { Pop-Location }

if ($NoDeploy) { Step 'Build succeeded; -NoDeploy given, stopping here.'; exit 0 }

# --- 4. Locate the built exe ------------------------------------------------
# Visual Studio generator puts top-level targets in <buildDir>\<Config>\.
# Depth 2 keeps this bounded -- never scan the whole build tree, it is huge
# and lives on a slow volume.
$exe = Get-ChildItem $BuildDir -Filter $ExeName -Recurse -Depth 2 -ErrorAction SilentlyContinue |
       Sort-Object LastWriteTime -Descending | Select-Object -First 1
if (-not $exe) { Fail "build reported success but $ExeName was not found under $BuildDir" }

# --- 5. Guard against a stale "successful" build ----------------------------
if ($exe.LastWriteTime -lt $buildStart.AddSeconds(-5)) {
    Write-Host "!!! WARNING: $($exe.FullName) is dated $($exe.LastWriteTime), before this build started ($buildStart)." -ForegroundColor Yellow
    Write-Host "!!! The build may not have recompiled anything. Deploying it anyway -- verify this is what you expect." -ForegroundColor Yellow
}

# --- 6. Deploy --------------------------------------------------------------
Step "Deploying to $RunDir"
$copied = @()
foreach ($ext in @('.exe', '.pdb', '.map')) {
    $src = [IO.Path]::ChangeExtension($exe.FullName, $ext)
    if (Test-Path $src) {
        Copy-Item $src $RunDir -Force
        $copied += Split-Path -Leaf $src
    }
}
if ($copied -notcontains $ExeName) { Fail "failed to copy $ExeName into $RunDir" }

# --- 7. Report --------------------------------------------------------------
Step 'Deployed:'
Get-ChildItem $RunDir | Where-Object { $copied -contains $_.Name } |
    Select-Object Name, LastWriteTime, @{n='MB'; e={[math]::Round($_.Length/1MB, 1)}} |
    Format-Table -AutoSize

Write-Host "Build + deploy OK." -ForegroundColor Green
