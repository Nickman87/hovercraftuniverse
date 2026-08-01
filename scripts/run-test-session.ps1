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
    multiplayer: dedicated server + -Clients N clients on THIS machine, each
                 with its own generated config so they get separate Ogre logs
                 and distinct player names. The cheap pre-flight for real
                 multiplayer: it exercises two genuine ZCom_Control client
                 instances in separate processes without needing a second
                 machine. It is NOT a substitute for the real thing -- it
                 shares a host, a clock and a loopback path.
    join       : -Clients N clients on THIS machine pointed at a remote
                 server via -Host. No local server is started. This is the
                 second-machine half of a real multiplayer test.

.PARAMETER Clients
    How many client processes to launch (multiplayer/join modes). Each gets a
    generated Client<N>.ini in the run dir with its own [Ogre] LogFile and
    [Player] PlayerName, so the lobby can tell them apart and their logs do
    not overwrite each other.

.PARAMETER PlayerName
    Base name for the generated client configs. Defaults to $env:COMPUTERNAME,
    so two machines never produce the same lobby name -- with -Clients > 1 a
    -<n> suffix is appended per client. Override only if you want something
    shorter or more readable in the lobby.

.PARAMETER NoStart
    multiplayer mode: connect the clients but do NOT start the race, so the
    session sits in the lobby. Use this to watch lobby-phase behaviour (player
    names, hovercraft selection, chat) which otherwise gets about three
    seconds before --autostart tears the lobby down.

.PARAMETER HostAddress
    Server address for join mode (IP or hostname, optionally host:port).
    Named HostAddress, not Host, because $Host is a reserved PowerShell
    automatic variable. The game's UDP ports are 2375 (game) and 2377 (chat)
    -- both must be reachable, they are separate ZCom_Control instances.

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
.EXAMPLE
    # Local two-client pre-flight, one machine, no human input needed.
    .\scripts\run-test-session.ps1 -Mode multiplayer -Clients 2 -Seconds 60
.EXAMPLE
    # Real two-machine test.
    # On the hosting machine:
    .\scripts\run-test-session.ps1 -Mode server -Seconds 0
    # On the other machine (get the host's LAN IP with ipconfig):
    .\scripts\run-test-session.ps1 -Mode join -HostAddress 192.168.1.42 -Clients 1 -Seconds 0
    # NOTE: the client ignores any :port in -HostAddress and always uses 2375,
    # exactly as the GUI "Join game" box does (MainMenuState::onConnect has
    # carried a "TODO: Parse IP and Port?" since 2010).
#>
[CmdletBinding()]
param(
    [ValidateSet('twoprocess', 'server', 'client', 'multiplayer', 'join')]
    [string] $Mode = 'twoprocess',
    [int]    $Seconds = 45,
    [int]    $Clients = 2,
    [string] $HostAddress = '',
    [string] $PlayerName = '',
    [switch] $NoStart,
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
    # Per-client logs from multiplayer/join modes (see New-ClientConfig).
    foreach ($src in (Get-ChildItem $DataDir -Filter 'Client*.log' -ErrorAction SilentlyContinue)) {
        Copy-Item $src.FullName $dest -Force; $n++
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
Remove-Item (Join-Path $DataDir 'Client*.log') -Force -ErrorAction SilentlyContinue
Remove-Item (Join-Path $RunDir 'physics-diag*.log') -Force -ErrorAction SilentlyContinue

# Two clients on one machine would otherwise share HovercraftUniverse.ini, and
# so share [Ogre] LogFile (their Ogre logs overwrite each other, making the run
# unreadable) and [Player] PlayerName (both appear in the lobby as the same
# person). Generate a per-client config instead, derived from the real one so
# the hand-fixed settings in C:\hu-modern-run are inherited rather than
# reinvented. The game reads this via --config= (see main.cpp).
function New-ClientConfig([int] $n) {
    $master = Join-Path $RunDir 'HovercraftUniverse.ini'
    if (-not (Test-Path $master)) { Fail "$master not found -- cannot derive a client config." }

    # The player name must be unique across MACHINES, not just across the
    # clients on one machine. Indexing by $n alone gave every machine's first
    # client the name "Player1", so a two-machine race had both humans called
    # Player1 in both lobbies -- which looks exactly like broken name
    # replication and is not. Prefix with the computer name unless the caller
    # overrides it.
    $who = if ($PlayerName) { $PlayerName } else { $env:COMPUTERNAME }
    $label = if ($Clients -gt 1) { "$who-$n" } else { $who }

    $name = "Client$n.ini"
    $dest = Join-Path $RunDir $name
    $out  = foreach ($line in (Get-Content $master)) {
        if     ($line -match '^\s*LogFile\s*=')    { "LogFile=Client$n.log" }
        elseif ($line -match '^\s*PlayerName\s*=') { "PlayerName=$label" }
        else                                       { $line }
    }
    Set-Content -Path $dest -Value $out -Encoding utf8
    return $name
}

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
    'multiplayer' {
        if ($Clients -lt 1) { Fail '-Clients must be at least 1.' }
        # RaceState fills empty slots only: bots = maxPlayers - humans (see
        # RaceState.cpp). So FillWithBots=1 is harmless once the humans have
        # joined; MaximumPlayers is the setting that actually gates them.
        $maxp = (Select-String -Path (Join-Path $RunDir 'Server.ini') -Pattern '^\s*MaximumPlayers\s*=\s*(\d+)').Matches.Groups[1].Value
        if ([int] $maxp -lt $Clients) {
            Fail "Server.ini MaximumPlayers=$maxp but -Clients $Clients requested; the extra client(s) would be refused (Lobby::canJoin)."
        }
        Step "Server.ini MaximumPlayers=$maxp, launching $Clients client(s)."
        $procs += Launch 'server' @('--server')
        Start-Sleep -Seconds 8
        for ($i = 1; $i -le $Clients; $i++) {
            $cfg = New-ClientConfig $i
            # Only the first client autoconnects-and-starts; it is the admin.
            # The rest just autoconnect, so the race does not begin before
            # they have actually joined the lobby.
            $flags = if ($i -eq 1 -and -not $NoStart) { '--autostart' } else { '--autoconnect' }
            $procs += Launch "client$i" @("--config=$cfg", $flags)
            Start-Sleep -Seconds 3
        }
    }
    'join' {
        if (-not $HostAddress) { Fail 'join mode needs -HostAddress <ip> (the machine running --server).' }
        if ($Clients -lt 1) { Fail '-Clients must be at least 1.' }
        for ($i = 1; $i -le $Clients; $i++) {
            $cfg = New-ClientConfig $i
            # No --autostart here: on the joining machine you are not the
            # admin, and the race is started from the hosting side.
            $procs += Launch "client$i" @("--config=$cfg", "--host=$HostAddress", '--autoconnect')
            Start-Sleep -Seconds 3
        }
    }
}

# A process that dies during image load -- a missing or wrong-architecture DLL
# is the classic cause -- never gets far enough to open an Ogre log, so the run
# folder fills with empty files and the script used to report nothing at all.
# Windows shows a modal error box on the machine itself, which is no help when
# the interesting machine is the other one. Catch it here instead.
Start-Sleep -Seconds 3
$dead = @($procs | Where-Object { $_.HasExited })
if ($dead.Count -gt 0) {
    foreach ($p in $dead) {
        $code = $p.ExitCode
        $hex  = '0x{0:X8}' -f ([uint32] ($code -band 0xFFFFFFFF))
        Write-Host "!!! PID $($p.Id) exited immediately, code $hex ($code)" -ForegroundColor Red
        switch ($hex) {
            '0xC000007B' { Write-Host '    STATUS_INVALID_IMAGE_FORMAT -- a DLL next to the exe is the wrong architecture. This build is Win32, so every DLL must be x86. If this machine was set up from dist\second-machine, re-run package-for-second-machine.ps1 on the build machine (it now asserts this) and re-install.' -ForegroundColor Yellow }
            '0xC0000135' { Write-Host '    STATUS_DLL_NOT_FOUND -- a required DLL is missing next to the exe. On a machine without Visual Studio this is usually the debug CRT (msvcp140d/vcruntime140d/ucrtbased).' -ForegroundColor Yellow }
            '0xC0000142' { Write-Host '    STATUS_DLL_INIT_FAILED -- a DLL loaded but failed to initialise.' -ForegroundColor Yellow }
        }
    }
    Fail 'Nothing to test -- the game did not start. Logs in the run folder will be empty.'
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
