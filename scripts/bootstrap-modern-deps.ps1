#Requires -Version 5.1
<#
.SYNOPSIS
    Bootstraps the vcpkg-based dependency set for the modern (v143, x86-windows)
    CMake tree under modern\ -- Ogre 14.5.2, its D3D9 render system, Boost, ENet
    and OIS.

.DESCRIPTION
    This is a SEPARATE bootstrap from scripts\bootstrap-dependencies.ps1, which
    reconstructs the original 2010-era VC9 dependency tree for the faithful
    conversion under HovercraftUniverse\. This script instead prepares the
    modern build (modern\CMakeLists.txt) which sources its dependencies from
    vcpkg for the x86-windows triplet.

    Four phases:

      Phase 1 - Ensure vcpkg exists at -VcpkgRoot (clone + bootstrap if absent)
                and that 7-Zip is present (never installs it; aborts with a
                winget hint if missing).

      Phase 2 - Ensure the legacy DirectX SDK (June 2010) bits that Ogre's
                RenderSystem_Direct3D9 needs (d3dx9.lib / d3dx9.h -- NOT present
                in the modern Windows SDK). The SDK installer itself is never
                run; only Include\ and Lib\x86\ are extracted from it with
                7-Zip into toolchain\dxsdk\DXSDK\.

      Phase 3 - Run `vcpkg install` for the package list below, for triplet
                x86-windows, with DXSDK_DIR pointed at the extracted SDK so
                Ogre's FindDirectX9.cmake can locate d3dx9.

      Phase 4 - Verification pass: print a checklist of expected installed
                headers/libs/plugins and exit non-zero if anything required
                is missing.

    Package list installed (see docs/porting/modern-build-setup.md for why):
      - ogre[core,d3d9,overlay,zip] (NOT the default features -- the default
        'freeimage' feature pulls in openjph, which hits an MSVC C1001 internal
        compiler error in its AVX2 code on x86; dropping freeimage avoids it,
        Ogre falls back to stb-based image codecs)
      - boost-algorithm, boost-date-time, boost-thread, boost-bind,
        boost-function, boost-smart-ptr (Utils/Config.cpp uses
        boost::algorithm::trim, Utils/Timing.cpp uses boost::posix_time;
        LuaBind will need more of these later)
      - enet, ois

    NOT installed via vcpkg (vendored separately, out of scope for this
    script):
      - Lua: vcpkg's `lua` port is 5.5, but the game's scripts and LuaBind are
        Lua 5.1-era. Lua 5.1 will be vendored separately.
      - LuaBind: no vcpkg port exists; a maintained deboostified fork will be
        vendored separately.
      - Hikari: will be built from archived source separately.

.PARAMETER VcpkgRoot
    Path to (or where to create) the vcpkg tree. Defaults to C:\Users\dirk\vcpkg.

.PARAMETER SkipDxsdkDownload
    Do not download the DirectX SDK installer; only use a copy already present
    in toolchain\. Fails with a clear error if it is missing and the extracted
    d3dx9.lib is not already present.

.PARAMETER MaxConcurrency
    Value passed through as VCPKG_MAX_CONCURRENCY for the vcpkg install step.
    Defaults to 4.

.EXAMPLE
    .\scripts\bootstrap-modern-deps.ps1

.EXAMPLE
    .\scripts\bootstrap-modern-deps.ps1 -VcpkgRoot D:\vcpkg -MaxConcurrency 8
#>

[CmdletBinding()]
param(
    [string]$VcpkgRoot = 'C:\Users\dirk\vcpkg',
    [switch]$SkipDxsdkDownload,
    [int]$MaxConcurrency = 4
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# ---------------------------------------------------------------------------
# Paths / constants
# ---------------------------------------------------------------------------

$RepoRoot     = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$ToolchainDir = Join-Path $RepoRoot 'toolchain'
$DxsdkRoot    = Join-Path $ToolchainDir 'dxsdk'
$DxsdkDir     = Join-Path $DxsdkRoot 'DXSDK'          # extracted Include\, Lib\x86\
$DxsdkExe     = Join-Path $ToolchainDir 'DXSDK_Jun10.exe'

$DxsdkDownloadUrl  = 'https://download.microsoft.com/download/A/E/7/AE743F1F-632B-4809-87A9-AA1BB3458E31/DXSDK_Jun10.exe'
$DxsdkExpectedSize = 599455936

$Triplet = 'x86-windows'

# Packages installed as a single `vcpkg install` invocation.
$VcpkgPackages = @(
    'ogre[core,d3d9,overlay,zip]'
    'boost-algorithm'
    'boost-date-time'
    'boost-thread'
    'boost-bind'
    'boost-function'
    'boost-smart-ptr'
    'enet'
    'ois'
)

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

function Write-Phase {
    param([string]$Message)
    Write-Host ''
    Write-Host "=== $Message ===" -ForegroundColor Cyan
}

function Write-Step {
    param([string]$Message)
    Write-Host "--> $Message" -ForegroundColor Cyan
}

function Write-Info {
    param([string]$Message)
    Write-Host "    $Message"
}

function Resolve-SevenZip {
    $candidatePaths = @(
        'C:\Program Files\7-Zip\7z.exe'
        'C:\Program Files (x86)\7-Zip\7z.exe'
    )
    foreach ($candidate in $candidatePaths) {
        if (Test-Path -LiteralPath $candidate -PathType Leaf) {
            return $candidate
        }
    }

    $onPath = Get-Command '7z' -ErrorAction SilentlyContinue
    if ($onPath) {
        return $onPath.Source
    }

    Write-Host ''
    Write-Host '7-Zip (7z.exe) was not found on this machine.' -ForegroundColor Red
    Write-Host 'The DirectX SDK installer is extracted (not installed) with 7-Zip.' -ForegroundColor Red
    Write-Host ''
    Write-Host 'Install 7-Zip and re-run this script:' -ForegroundColor Yellow
    Write-Host '    winget install 7zip.7zip' -ForegroundColor Yellow
    Write-Host ''
    throw '7-Zip is required but was not found (checked Program Files and PATH).'
}

function Test-VcpkgPresent {
    param([Parameter(Mandatory)][string]$Root)
    $exe = Join-Path $Root 'vcpkg.exe'
    return (Test-Path -LiteralPath $exe -PathType Leaf)
}

function Initialize-Vcpkg {
    param([Parameter(Mandatory)][string]$Root)

    if (Test-VcpkgPresent -Root $Root) {
        Write-Info "vcpkg already present at $Root"
        return
    }

    $parent = Split-Path -Parent $Root
    if ($parent -and -not (Test-Path -LiteralPath $parent -PathType Container)) {
        New-Item -ItemType Directory -Path $parent -Force | Out-Null
    }

    if (Test-Path -LiteralPath $Root -PathType Container) {
        # Directory exists but vcpkg.exe doesn't -- assume an un-bootstrapped clone.
        Write-Info "Found existing directory at $Root without vcpkg.exe; attempting bootstrap."
    }
    else {
        Write-Step "Cloning vcpkg into $Root"
        $git = Get-Command 'git' -ErrorAction SilentlyContinue
        if (-not $git) {
            throw 'git is required to clone vcpkg but was not found on PATH.'
        }
        & git clone https://github.com/microsoft/vcpkg.git $Root
        if ($LASTEXITCODE -ne 0) {
            throw "git clone of vcpkg failed with exit code $LASTEXITCODE."
        }
    }

    Write-Step "Bootstrapping vcpkg at $Root"
    $bootstrap = Join-Path $Root 'bootstrap-vcpkg.bat'
    if (-not (Test-Path -LiteralPath $bootstrap -PathType Leaf)) {
        throw "Expected bootstrap script not found: $bootstrap"
    }
    & $bootstrap -disableMetrics
    if ($LASTEXITCODE -ne 0) {
        throw "bootstrap-vcpkg.bat failed with exit code $LASTEXITCODE."
    }

    if (-not (Test-VcpkgPresent -Root $Root)) {
        throw "vcpkg.exe still not found at $Root after bootstrap."
    }
}

function Test-AuthenticodeValid {
    param([Parameter(Mandatory)][string]$Path)
    $sig = Get-AuthenticodeSignature -LiteralPath $Path
    return ($sig.Status -eq [System.Management.Automation.SignatureStatus]::Valid)
}

function Expand-DxsdkBits {
    param(
        [Parameter(Mandatory)][string]$SevenZipExe,
        [Parameter(Mandatory)][string]$ExePath,
        [Parameter(Mandatory)][string]$DestinationDir
    )

    if (-not (Test-Path -LiteralPath $DestinationDir -PathType Container)) {
        New-Item -ItemType Directory -Path $DestinationDir -Force | Out-Null
    }

    # Only pull the two subtrees the D3D9 render system build needs.
    $arguments = @(
        'x', '-y', "-o$DestinationDir", $ExePath,
        'DXSDK\Include\*', 'DXSDK\Lib\x86\*'
    )
    $output = & $SevenZipExe @arguments 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Host ($output | Out-String)
        throw "7z.exe exited with code $LASTEXITCODE while extracting the DirectX SDK."
    }
}

# ---------------------------------------------------------------------------
# Phase 1 -- vcpkg + 7-Zip present
# ---------------------------------------------------------------------------

Write-Phase 'Phase 1: vcpkg + 7-Zip'

Write-Step "Repo root:   $RepoRoot"
Write-Step "Vcpkg root:  $VcpkgRoot"

Initialize-Vcpkg -Root $VcpkgRoot
$vcpkgExe = Join-Path $VcpkgRoot 'vcpkg.exe'
Write-Info "vcpkg.exe:   $vcpkgExe"

$sevenZip = Resolve-SevenZip
Write-Info "7-Zip:       $sevenZip"

# ---------------------------------------------------------------------------
# Phase 2 -- DirectX SDK (June 2010) Include + Lib\x86, extracted not installed
# ---------------------------------------------------------------------------

Write-Phase 'Phase 2: DirectX SDK (d3dx9) bits'

$d3dx9Lib = Join-Path $DxsdkDir 'Lib\x86\d3dx9.lib'
$d3dx9Header = Join-Path $DxsdkDir 'Include\d3dx9.h'

if ((Test-Path -LiteralPath $d3dx9Lib -PathType Leaf) -and (Test-Path -LiteralPath $d3dx9Header -PathType Leaf)) {
    Write-Info "Already extracted: $d3dx9Lib"
    Write-Info "Already extracted: $d3dx9Header"
}
else {
    if (-not (Test-Path -LiteralPath $ToolchainDir -PathType Container)) {
        New-Item -ItemType Directory -Path $ToolchainDir -Force | Out-Null
    }

    $haveExe = $false
    if (Test-Path -LiteralPath $DxsdkExe -PathType Leaf) {
        $existingSize = (Get-Item -LiteralPath $DxsdkExe).Length
        if ($existingSize -eq $DxsdkExpectedSize) {
            Write-Info "Found existing installer with expected size: $DxsdkExe"
            $haveExe = $true
        }
        else {
            Write-Info "Existing installer has wrong size ($existingSize bytes, expected $DxsdkExpectedSize) -- will re-download."
            Remove-Item -LiteralPath $DxsdkExe -Force
        }
    }

    if (-not $haveExe) {
        if ($SkipDxsdkDownload) {
            throw "DirectX SDK installer not found at $DxsdkExe and -SkipDxsdkDownload was specified."
        }

        Write-Step "Downloading DirectX SDK (June 2010) installer"
        Write-Info "  from $DxsdkDownloadUrl"
        Write-Info "  (this is downloaded and extracted from -- never run/installed)"

        $tempPath = "$DxsdkExe.download"
        try {
            Invoke-WebRequest -Uri $DxsdkDownloadUrl -OutFile $tempPath -UseBasicParsing
        }
        catch {
            if (Test-Path -LiteralPath $tempPath) {
                Remove-Item -LiteralPath $tempPath -Force -ErrorAction SilentlyContinue
            }
            throw "Failed to download DirectX SDK installer: $($_.Exception.Message)"
        }
        Move-Item -LiteralPath $tempPath -Destination $DxsdkExe -Force

        $downloadedSize = (Get-Item -LiteralPath $DxsdkExe).Length
        if ($downloadedSize -ne $DxsdkExpectedSize) {
            Remove-Item -LiteralPath $DxsdkExe -Force -ErrorAction SilentlyContinue
            throw "Downloaded DirectX SDK installer has unexpected size $downloadedSize bytes (expected $DxsdkExpectedSize). Deleted; re-run the script."
        }
    }

    Write-Step "Verifying Authenticode signature on the DirectX SDK installer"
    if (-not (Test-AuthenticodeValid -Path $DxsdkExe)) {
        throw "Authenticode signature on '$DxsdkExe' is not Valid. Refusing to extract an unverified installer. Delete it and re-run."
    }
    Write-Info "Signature: Valid"

    Write-Step "Extracting Include\ and Lib\x86\ from the DirectX SDK installer"
    Expand-DxsdkBits -SevenZipExe $sevenZip -ExePath $DxsdkExe -DestinationDir $DxsdkRoot

    if (-not (Test-Path -LiteralPath $d3dx9Lib -PathType Leaf)) {
        throw "Extraction completed but d3dx9.lib not found at expected path: $d3dx9Lib"
    }
    if (-not (Test-Path -LiteralPath $d3dx9Header -PathType Leaf)) {
        throw "Extraction completed but d3dx9.h not found at expected path: $d3dx9Header"
    }
    Write-Info "Extracted OK: $d3dx9Lib"
    Write-Info "Extracted OK: $d3dx9Header"
}

# ---------------------------------------------------------------------------
# Phase 3 -- vcpkg install
# ---------------------------------------------------------------------------

Write-Phase 'Phase 3: vcpkg install'

$dxsdkDirWithSlash = $DxsdkDir.TrimEnd('\') + '\'
$env:DXSDK_DIR = $dxsdkDirWithSlash
$env:VCPKG_MAX_CONCURRENCY = "$MaxConcurrency"

Write-Info "DXSDK_DIR              = $env:DXSDK_DIR"
Write-Info "VCPKG_MAX_CONCURRENCY   = $env:VCPKG_MAX_CONCURRENCY"
Write-Info "Triplet                 = $Triplet"

$installArgs = @('install') + $VcpkgPackages + @("--triplet=$Triplet")

Write-Step "Running vcpkg install"
Write-Info ("Command: `"$vcpkgExe`" " + ($installArgs -join ' '))

& $vcpkgExe @installArgs
$vcpkgExitCode = $LASTEXITCODE

if ($vcpkgExitCode -ne 0) {
    Write-Host ''
    Write-Host "vcpkg install failed with exit code $vcpkgExitCode." -ForegroundColor Red
    $buildtreesLogs = Join-Path $VcpkgRoot 'buildtrees'
    Write-Host "Check per-port failure logs under: $buildtreesLogs\<port>\*-out.log / *-err.log" -ForegroundColor Yellow
    $installedLog = Join-Path $VcpkgRoot 'installed\vcpkg\issue_body.md'
    if (Test-Path -LiteralPath $installedLog -PathType Leaf) {
        Write-Host "vcpkg also wrote a diagnostic report to: $installedLog" -ForegroundColor Yellow
    }
    throw "vcpkg install failed (exit code $vcpkgExitCode). See logs referenced above."
}

Write-Info "vcpkg install completed."

# ---------------------------------------------------------------------------
# Phase 4 -- verification
# ---------------------------------------------------------------------------

Write-Phase 'Phase 4: Verification'

$installedRoot = Join-Path $VcpkgRoot "installed\$Triplet"

$pluginCandidates = @(
    (Join-Path $installedRoot 'plugins\ogre\RenderSystem_Direct3D9.dll')
    (Join-Path $installedRoot 'bin\RenderSystem_Direct3D9.dll')
    (Join-Path $installedRoot 'bin\Plugin_Direct3D9.dll')
)
$pluginFound = $pluginCandidates | Where-Object { Test-Path -LiteralPath $_ -PathType Leaf } | Select-Object -First 1

$checks = New-Object System.Collections.Generic.List[object]
$checks.Add([pscustomobject]@{ Name = 'OGRE\Ogre.h';                    Path = (Join-Path $installedRoot 'include\OGRE\Ogre.h') })
$checks.Add([pscustomobject]@{ Name = 'OGRE\Overlay\OgreOverlaySystem.h'; Path = (Join-Path $installedRoot 'include\OGRE\Overlay\OgreOverlaySystem.h') })
$checks.Add([pscustomobject]@{ Name = 'RenderSystem_Direct3D9 plugin';  Path = $pluginFound })
$checks.Add([pscustomobject]@{ Name = 'enet\enet.h';                    Path = (Join-Path $installedRoot 'include\enet\enet.h') })
$checks.Add([pscustomobject]@{ Name = 'OIS\OIS.h';                      Path = (Join-Path $installedRoot 'include\OIS\OIS.h') })
$checks.Add([pscustomobject]@{ Name = 'boost\algorithm\string\trim.hpp'; Path = (Join-Path $installedRoot 'include\boost\algorithm\string\trim.hpp') })
$checks.Add([pscustomobject]@{ Name = 'DXSDK d3dx9.lib';                Path = $d3dx9Lib })

$allOk = $true
foreach ($check in $checks) {
    $ok = -not [string]::IsNullOrEmpty($check.Path) -and (Test-Path -LiteralPath $check.Path -PathType Leaf)
    if (-not $ok) { $allOk = $false }
    $status = if ($ok) { [char]0x2713 } else { [char]0x2717 }
    $color = if ($ok) { 'Green' } else { 'Red' }
    $shownPath = if ([string]::IsNullOrEmpty($check.Path)) { '(not found -- checked plugins\ogre, bin)' } else { $check.Path }
    Write-Host ("  [{0}] {1,-40} {2}" -f $status, $check.Name, $shownPath) -ForegroundColor $color
}

Write-Host ''
if ($allOk) {
    Write-Host 'All modern dependency checks passed.' -ForegroundColor Green
    exit 0
}
else {
    Write-Host 'One or more modern dependency checks FAILED. See the table above.' -ForegroundColor Red
    exit 1
}
