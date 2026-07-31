<#
.SYNOPSIS
    Runs a test session against C:\hu-modern-run and captures every log into
    one per-run folder.

.DESCRIPTION
    THE canonical way to run the game for testing. Use this instead of
    launching the exe by hand, so that:

      * Two sessions can never run at once. Concurrent instances overwrite
        each other's Ogre logs and fight over UDP 2375, which has already
        produced misleading results more than once. This script refuses to
        start if anything is still running.
      * Every artefact of a run lands in its own timestamped folder, so a run
        can never be contaminated by the previous one's leftovers.
      * stdout/stderr are captured per process, to separate files.
      * The Ogre logs (which the game writes into data\ because
        Application::parseIni chdir's there) are snapshotted into the run
        folder at the end, before the next run can overwrite them.

    Run folders: C:\hu-modern-run\testruns\<timestamp>-<mode>\

.PARAMETER Mode
    twoprocess : dedicated server + --autostart client. Physics runs in the
                 server, so this exercises the networked path and keeps
                 physics out of the rendering process.
    server     : dedicated server only.
    client     : plain client, no arguments -- the human path (menu ->
                 Single Player). NOTE: in single-player the game hosts its
                 server INSIDE the client process, so collision
                 reconstruction, physics and rendering all share one process
                 and one Ogre log. Bugs that only appear here are usually
                 about that sharing.

.PARAMETER Seconds
    How long to let the session run before stopping it. Use 0 to leave it
    running (for interactive human testing) -- the script then skips log
    collection and tells you to re-run with -Collect afterwards.

.PARAMETER Collect
    Don't launch anything; just snapshot the current logs into a new run
    folder. Use after an interactive session started with -Seconds 0.

.PARAMETER Debugger
    Launch under cdb.exe (x86, from the WinDbg package). The game runs
    normally; first-chance exceptions are passed through to the game's own
    handlers exactly as usual, so behaviour is unchanged. On an UNHANDLED
    (second-chance) access violation, cdb writes stacks for every thread plus
    a full dump into the run folder and exits. Use this to get a real
    symbolized stack instead of guessing from Windows event-log offsets.

.EXAMPLE
    .\scripts\run-test-session.ps1 -Mode twoprocess -Seconds 45
.EXAMPLE
    .\scripts\run-test-session.ps1 -Mode client -Seconds 0
    # ... play ...
    .\scripts\run-test-session.ps1 -Collect
#>
[CmdletBinding()]
param(
    [ValidateSet('twoprocess', 'server', 'client')]
    [string] $Mode = 'twoprocess',
    [int]    $Seconds = 45,
    [switch] $Collect,
    [switch] $Debugger
)

$ErrorActionPreference = 'Stop'

$RunDir  = 'C:\hu-modern-run'
$DataDir = Join-Path $RunDir 'data'
$Exe     = Join-Path $RunDir 'HovercraftUniverse.exe'
$Runs    = Join-Path $RunDir 'testruns'

function Step($m) { Write-Host "==> $m" -ForegroundColor Cyan }
function Fail($m) { Write-Host "!!! $m" -ForegroundColor Red; exit 1 }

function New-RunFolder([string] $tag) {
    if (-not (Test-Path $Runs)) { New-Item -ItemType Directory $Runs | Out-Null }
    $stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
    $dir = Join-Path $Runs "$stamp-$tag"
    New-Item -ItemType Directory $dir | Out-Null
    return $dir
}

# Snapshot every log the game may have written. The Ogre logs live in data\
# because Application::parseIni() chdir's into DataPath before Root is
# created; physics-diag*.log (when a diagnostic build is deployed) lands in
# the run root.
function Save-Logs([string] $dest) {
    $n = 0
    foreach ($src in @(
        (Join-Path $DataDir 'HovercraftUniverse.log'),
        (Join-Path $DataDir 'DedicatedServer.log'),
        (Join-Path $DataDir 'SinglePlayerServer.log'))) {
        if (Test-Path $src) { Copy-Item $src $dest -Force; $n++ }
    }
    foreach ($src in (Get-ChildItem $RunDir -Filter 'physics-diag*.log' -ErrorAction SilentlyContinue)) {
        Copy-Item $src.FullName $dest -Force; $n++
    }
    return $n
}

if ($Collect) {
    $dir = New-RunFolder 'collect'
    $n = Save-Logs $dir
    Step "Collected $n log file(s) into $dir"
    Get-ChildItem $dir | Select-Object Name, LastWriteTime, @{n='KB'; e={[math]::Round($_.Length/1KB, 1)}} | Format-Table -AutoSize
    exit 0
}

# --- Refuse to run concurrently ---------------------------------------------
$running = @(Get-Process -Name 'HovercraftUniverse' -ErrorAction SilentlyContinue)
if ($running.Count -gt 0) {
    Fail "$($running.Count) HovercraftUniverse process(es) already running (PIDs: $($running.Id -join ', ')). Two sessions share the same Ogre log files and UDP port -- results would be unreliable. Stop them first."
}
if (-not (Test-Path $Exe)) { Fail "$Exe not found -- build and deploy first (scripts\build-and-deploy.ps1)." }

$runFolder = New-RunFolder $Mode
Step "Run folder: $runFolder"
Step ("Deployed exe: " + (Get-Item $Exe).LastWriteTime)

# Delete the previous run's Ogre logs so this run's are unambiguous -- they
# have already been snapshotted into their own folder by whichever run wrote
# them, and the game appends nothing across runs anyway.
foreach ($stale in @('HovercraftUniverse.log', 'DedicatedServer.log', 'SinglePlayerServer.log')) {
    Remove-Item (Join-Path $DataDir $stale) -Force -ErrorAction SilentlyContinue
}
Remove-Item (Join-Path $RunDir 'physics-diag*.log') -Force -ErrorAction SilentlyContinue

# cdb.exe ships inside the WinDbg MSIX package. Resolve it dynamically -- the
# version is part of the path and changes when WinDbg updates. x86, to match
# the game's Win32 build.
function Get-Cdb {
    $pkg = Get-AppxPackage -Name Microsoft.WinDbg -ErrorAction SilentlyContinue
    if (-not $pkg) { return $null }
    $candidate = Join-Path $pkg.InstallLocation 'x86\cdb.exe'
    if (Test-Path $candidate) { return $candidate }
    return $null
}

$procs = @()
function Launch([string] $name, [string[]] $arguments) {
    $out = Join-Path $runFolder "$name.stdout.log"
    $err = Join-Path $runFolder "$name.stderr.log"

    if ($Debugger) {
        $cdb = Get-Cdb
        if (-not $cdb) { Fail 'WinDbg (cdb.exe x86) not found. Install with: winget install --id Microsoft.WinDbg' }

        $dbgLog = Join-Path $runFolder "$name.cdb.log"
        $dump   = Join-Path $runFolder "$name.crash.dmp"

        # sxd -c2: pass first-chance exceptions through to the game untouched
        # (its own SEH handlers must behave exactly as they do without a
        # debugger), but on an UNHANDLED access violation run the second-chance
        # command: stacks for all threads, a full dump, then quit-and-detach.
        $onCrash = "~*kb 40; .dump /ma `"$dump`"; qd"
        $script  = ".symfix+; .sympath+ `"$RunDir`"; .reload; sxd -c2 `"$onCrash`" av; g"

        $cdbArgs = @('-g', '-G', '-logo', $dbgLog, '-c', $script, $Exe) + $arguments
        $p = Start-Process -FilePath $cdb -ArgumentList $cdbArgs -WorkingDirectory $RunDir -PassThru `
                           -RedirectStandardOutput $out -RedirectStandardError $err
        Step "launched $name under cdb (PID $($p.Id)) -- debugger log: $dbgLog"
        return $p
    }

    $splat = @{
        FilePath               = $Exe
        WorkingDirectory       = $RunDir
        PassThru               = $true
        RedirectStandardOutput = $out
        RedirectStandardError  = $err
    }
    if ($arguments.Count -gt 0) { $splat['ArgumentList'] = $arguments }
    $p = Start-Process @splat
    Step "launched $name (PID $($p.Id))"
    return $p
}

switch ($Mode) {
    'server'     { $procs += Launch 'server' @('--server') }
    'client'     { $procs += Launch 'client' @() }
    'twoprocess' {
        $procs += Launch 'server' @('--server')
        Start-Sleep -Seconds 8
        $procs += Launch 'client' @('--autostart')
    }
}

if ($Seconds -le 0) {
    Step "Left running for interactive testing. When finished, stop the game and run:"
    Write-Host "    .\scripts\run-test-session.ps1 -Collect" -ForegroundColor Yellow
    exit 0
}

Step "Running for $Seconds seconds..."
Start-Sleep -Seconds $Seconds

# Report what survived and whether anything is showing an error dialog before
# tearing down -- a crashed process usually leaves a modal window behind.
Step 'State at end of run:'
Get-Process -Name 'HovercraftUniverse' -ErrorAction SilentlyContinue |
    Select-Object Id, MainWindowTitle, Responding | Format-Table -AutoSize

foreach ($p in $procs) {
    if (-not $p.HasExited) { Stop-Process -Id $p.Id -Force -ErrorAction SilentlyContinue }
}
Start-Sleep -Seconds 2

$n = Save-Logs $runFolder
Step "Saved $n log file(s):"
Get-ChildItem $runFolder | Select-Object Name, @{n='KB'; e={[math]::Round($_.Length/1KB, 1)}} | Format-Table -AutoSize
Write-Host "Session complete: $runFolder" -ForegroundColor Green
