#Requires -RunAsAdministrator
<#
    sandbox-build.ps1
    Runs INSIDE a fresh Windows Sandbox instance (see build-vs2008.wsb).
    Installs Visual C++ 2008 Express (SP1) from the mounted all-in-one ISO,
    then builds HovercraftUniverse.sln (Release|Win32, then Debug|Win32) with
    vcbuild.exe.

    Everything is logged to C:\repo\toolchain\sandbox\build-<timestamp>.log,
    which lives on the mapped host share so it survives the sandbox closing.
#>

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# ---------------------------------------------------------------------------
# Paths / constants
# ---------------------------------------------------------------------------
$RepoRoot      = 'C:\repo'
$SandboxDir    = Join-Path $RepoRoot 'toolchain\sandbox'
$IsoPath       = Join-Path $RepoRoot 'toolchain\VS2008ExpressWithSP1ENUX1504728.iso'
$SlnPath       = Join-Path $RepoRoot 'HovercraftUniverse\HovercraftUniverse.sln'
$DepsPath      = Join-Path $RepoRoot 'HovercraftUniverse\dependencies'
$BootstrapHint = Join-Path $RepoRoot 'scripts\bootstrap-dependencies.ps1'
$ArtifactsDir  = Join-Path $SandboxDir 'artifacts'
$ExePath       = Join-Path $RepoRoot 'HovercraftUniverse\bin\release\HovercraftUniverse.exe'
$LibGlob       = Join-Path $RepoRoot 'HovercraftUniverse\lib\Release'
$ArtifactExe   = Join-Path $ArtifactsDir 'HovercraftUniverse-vc9-release.exe'

$VsInstallDir  = "${env:ProgramFiles(x86)}\Microsoft Visual Studio 9.0"
$VcInstallDir  = Join-Path $VsInstallDir 'VC'
$ClExe         = Join-Path $VcInstallDir 'bin\cl.exe'
$VcBuildExe    = Join-Path $VcInstallDir 'vcpackages\vcbuild.exe'
$VcVarsAllBat  = Join-Path $VcInstallDir 'vcvarsall.bat'

$InstallTimeoutMinutes = 30

# ---------------------------------------------------------------------------
# Logging
# ---------------------------------------------------------------------------
$timestamp = Get-Date -Format 'yyyyMMdd-HHmmss'
$LogPath   = Join-Path $SandboxDir "build-$timestamp.log"

if (-not (Test-Path $SandboxDir)) {
    New-Item -ItemType Directory -Path $SandboxDir -Force | Out-Null
}

Start-Transcript -Path $LogPath -Force | Out-Null

function Write-Phase {
    param([string]$Name)
    Write-Host ''
    Write-Host "=== PHASE: $Name ===" -ForegroundColor Cyan
}

function Write-FailedBanner {
    param([string]$Message)
    Write-Host ''
    Write-Host '=============================================' -ForegroundColor Red
    Write-Host '  BUILD FAILED' -ForegroundColor Red
    Write-Host "  $Message" -ForegroundColor Red
    Write-Host "  Log: $LogPath" -ForegroundColor Red
    Write-Host '=============================================' -ForegroundColor Red
}

# ---------------------------------------------------------------------------
# Helper: import environment variables produced by a cmd.exe batch file
# (used to pick up vcvarsall.bat's INCLUDE/LIB/PATH without launching a
# separate shell for the actual build).
# ---------------------------------------------------------------------------
function Import-BatchEnvironment {
    param(
        [Parameter(Mandatory)][string]$BatchFile,
        [string]$Arguments = ''
    )

    if (-not (Test-Path $BatchFile)) {
        throw "Batch file not found: $BatchFile"
    }

    $cmd = "`"$BatchFile`" $Arguments && set"
    $output = & cmd.exe /c $cmd 2>&1

    if ($LASTEXITCODE -ne 0) {
        throw "Failed to run '$BatchFile $Arguments' (exit code $LASTEXITCODE): $output"
    }

    foreach ($line in $output) {
        if ($line -match '^([^=]+)=(.*)$') {
            $name  = $Matches[1]
            $value = $Matches[2]
            Set-Item -Path "Env:$name" -Value $value -ErrorAction SilentlyContinue
        }
    }
}

# ---------------------------------------------------------------------------
# Helper: wait until VS2008's cl.exe appears, or timeout
# ---------------------------------------------------------------------------
function Wait-ForInstallComplete {
    param(
        [Parameter(Mandatory)][string]$MarkerPath,
        [Parameter(Mandatory)][int]$TimeoutMinutes
    )

    $deadline = (Get-Date).AddMinutes($TimeoutMinutes)
    Write-Host "Waiting for install marker: $MarkerPath (timeout ${TimeoutMinutes}m)"

    while ((Get-Date) -lt $deadline) {
        if (Test-Path $MarkerPath) {
            Write-Host "Install marker found: $MarkerPath"
            return $true
        }

        # Also bail out early if setup.exe / vs_setup-ish processes have all exited
        # and enough time has passed for a quick sanity re-check.
        Start-Sleep -Seconds 15
    }

    return (Test-Path $MarkerPath)
}

# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------
try {
    Write-Phase 'Preflight checks'

    if (-not (Test-Path $IsoPath)) {
        throw "VS2008 Express ISO not found at '$IsoPath'. Place VS2008ExpressWithSP1ENUX1504728.iso in toolchain\ on the host before starting the sandbox."
    }
    Write-Host "Found ISO: $IsoPath"

    if (-not (Test-Path $SlnPath)) {
        throw "Solution file not found at '$SlnPath'."
    }
    Write-Host "Found solution: $SlnPath"

    if (-not (Test-Path $DepsPath)) {
        throw "Dependencies folder not found at '$DepsPath'. Run $BootstrapHint on the host first (it fetches/unpacks third-party dependencies the projects expect at HovercraftUniverse\dependencies), then restart the sandbox."
    }
    Write-Host "Found dependencies folder: $DepsPath"

    # -----------------------------------------------------------------------
    if (Test-Path $ClExe) {
        Write-Phase 'VC++ 2008 already installed, skipping install'
        Write-Host "Found existing cl.exe at $ClExe"
    }
    else {
        Write-Phase 'Enable .NET Framework 3.5 (NetFx3)'
        Write-Host 'VS2008 setup depends on .NET 3.5; enabling via DISM...'
        $dism = Start-Process -FilePath 'DISM.exe' `
            -ArgumentList '/Online', '/Enable-Feature', '/FeatureName:NetFx3', '/All', '/NoRestart' `
            -NoNewWindow -Wait -PassThru
        if ($dism.ExitCode -ne 0 -and $dism.ExitCode -ne 3010) {
            Write-Host "Warning: DISM exited with code $($dism.ExitCode); continuing anyway (VS setup may still succeed via WU)."
        }
        else {
            Write-Host "DISM completed with exit code $($dism.ExitCode)."
        }

        Write-Phase 'Mount VS2008 Express ISO'
        $mounted = Mount-DiskImage -ImagePath $IsoPath -PassThru
        $driveLetter = ($mounted | Get-Volume).DriveLetter
        if (-not $driveLetter) {
            throw "Failed to determine drive letter for mounted ISO '$IsoPath'."
        }
        $isoRoot = "${driveLetter}:\"
        Write-Host "ISO mounted at $isoRoot"

        try {
            $setupExe = Join-Path $isoRoot 'VCExpress\setup.exe'
            if (-not (Test-Path $setupExe)) {
                # Some ISO layouts put setup.exe at the root instead.
                $altSetup = Join-Path $isoRoot 'setup.exe'
                if (Test-Path $altSetup) {
                    $setupExe = $altSetup
                }
                else {
                    throw "Could not find setup.exe under '$isoRoot' (expected VCExpress\setup.exe)."
                }
            }
            Write-Host "Using installer: $setupExe"

            Write-Phase 'Install Visual C++ 2008 Express (silent)'

            $installed = $false
            $attempts = @('/q /norestart', '/qb /norestart')

            foreach ($argString in $attempts) {
                Write-Host "Launching: `"$setupExe`" $argString"
                try {
                    $proc = Start-Process -FilePath $setupExe -ArgumentList $argString -PassThru -Wait -ErrorAction Stop
                    Write-Host "setup.exe exited with code $($proc.ExitCode) (args: $argString)"
                }
                catch {
                    Write-Host "setup.exe invocation failed (args: $argString): $($_.Exception.Message)"
                }

                Write-Host 'Polling for install-complete marker (installer spawns child processes that may still be running)...'
                if (Wait-ForInstallComplete -MarkerPath $ClExe -TimeoutMinutes $InstallTimeoutMinutes) {
                    $installed = $true
                    break
                }

                Write-Host "Marker not found after attempt with args '$argString'; trying fallback invocation if available."
            }

            if (-not $installed) {
                throw "Visual C++ 2008 Express install did not complete within $InstallTimeoutMinutes minutes (marker '$ClExe' never appeared)."
            }

            Write-Host 'Visual C++ 2008 Express installation confirmed.'
        }
        finally {
            Write-Phase 'Dismount VS2008 Express ISO'
            try {
                Dismount-DiskImage -ImagePath $IsoPath -ErrorAction Stop | Out-Null
                Write-Host 'ISO dismounted.'
            }
            catch {
                Write-Host "Warning: failed to dismount ISO cleanly: $($_.Exception.Message)"
            }
        }
    }

    if (-not (Test-Path $ClExe)) {
        throw "cl.exe still not found at '$ClExe' after install phase."
    }
    if (-not (Test-Path $VcBuildExe)) {
        throw "vcbuild.exe not found at '$VcBuildExe'."
    }

    # -----------------------------------------------------------------------
    Write-Phase 'Set up VC9 build environment (vcvarsall.bat x86)'
    Import-BatchEnvironment -BatchFile $VcVarsAllBat -Arguments 'x86'
    Write-Host "VCINSTALLDIR=$env:VCINSTALLDIR"

    # -----------------------------------------------------------------------
    Write-Phase 'Build Release|Win32'
    & $VcBuildExe $SlnPath 'Release|Win32'
    $releaseExitCode = $LASTEXITCODE
    Write-Host "vcbuild.exe (Release|Win32) exit code: $releaseExitCode"

    if ($releaseExitCode -ne 0) {
        throw "vcbuild.exe failed for Release|Win32 (exit code $releaseExitCode). See log for full compiler/linker output."
    }

    Write-Phase 'Build Debug|Win32 (non-fatal on failure)'
    & $VcBuildExe $SlnPath 'Debug|Win32'
    $debugExitCode = $LASTEXITCODE
    Write-Host "vcbuild.exe (Debug|Win32) exit code: $debugExitCode"
    if ($debugExitCode -ne 0) {
        Write-Host "Debug|Win32 build failed (exit code $debugExitCode) - continuing, since Release is the reference target."
    }

    # -----------------------------------------------------------------------
    Write-Phase 'Verify Release artifacts'

    if (-not (Test-Path $ExePath)) {
        throw "Expected Release executable not found at '$ExePath' even though vcbuild.exe reported success."
    }

    $exeItem = Get-Item $ExePath
    Write-Host ("{0}  {1:N0} bytes" -f $exeItem.FullName, $exeItem.Length)

    if (Test-Path $LibGlob) {
        Get-ChildItem -Path $LibGlob -Filter '*.lib' -ErrorAction SilentlyContinue | ForEach-Object {
            Write-Host ("{0}  {1:N0} bytes" -f $_.FullName, $_.Length)
        }
    }
    else {
        Write-Host "Note: lib output folder '$LibGlob' not found (no .lib files to list)."
    }

    Write-Phase 'Copy artifact to toolchain\sandbox\artifacts'
    if (-not (Test-Path $ArtifactsDir)) {
        New-Item -ItemType Directory -Path $ArtifactsDir -Force | Out-Null
    }
    Copy-Item -Path $ExePath -Destination $ArtifactExe -Force
    Write-Host "Copied to: $ArtifactExe"

    Write-Phase 'Build succeeded'
    Write-Host ''
    Write-Host '=============================================' -ForegroundColor Green
    Write-Host '  BUILD SUCCEEDED' -ForegroundColor Green
    Write-Host "  Executable: $ExePath" -ForegroundColor Green
    Write-Host "  Artifact copy: $ArtifactExe" -ForegroundColor Green
    Write-Host "  Log: $LogPath" -ForegroundColor Green
    Write-Host '=============================================' -ForegroundColor Green
}
catch {
    Write-FailedBanner -Message $_.Exception.Message
    Write-Host ''
    Write-Host 'Full error record:'
    Write-Host ($_ | Out-String)
}
finally {
    try { Stop-Transcript | Out-Null } catch { }
}
