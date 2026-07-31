# Multiplayer testing

How to actually test multiplayer, and what has been verified so far.

## Why single-player and twoprocess are not multiplayer tests

- **Single player** hosts the server *inside* the client process
  (`MainMenu::onSingleplayer` constructs `HUDedicatedServer`). One process,
  one Ogre log, no wire between client and server for most of the state.
- **twoprocess** is a real dedicated server plus one client, but the second
  racer is a **bot**, and a bot is not a second client. A bot's
  `PlayerSettings` is constructed on the server with its name already set
  (`PlayerSettings(Lobby*, const Ogre::String& name)`), so its name reaches
  clients in the `eZCom_EventInit` snapshot that fires when a connection
  links to the node.

A human's name does not travel that way. A joining client's `PlayerSettings`
is constructed on the server **empty** (`PlayerSettings(Lobby*, unsigned int
connID)`); the owning client fills it in afterwards, and it then has to travel
owner -> authority -> other proxies by *replication*. Nothing in the
one-client-plus-bot setup ever exercises that path, in either direction. Same
for the connection-accept ordering: every dynamic node in a bot race is
registered *after* the single client connects, so "a node that already existed
when you connected" is never tested either (see phase-b-replication.md §11).

Anything that only breaks with two real clients was therefore invisible until
`multiplayer` mode existed.

## Local pre-flight: two clients on one machine

```powershell
.\scripts\run-test-session.ps1 -Mode multiplayer -Clients 2 -Seconds 75
```

Dedicated server + N client processes, each launched with `--config=Client<N>.ini`.
The script generates those configs from `HovercraftUniverse.ini`, overriding
only `[Ogre] LogFile` and `[Player] PlayerName`. Without that the clients
share a log (each overwriting the other, so the run is unreadable) and share a
player name (so the lobby cannot tell them apart).

Add `-NoStart` to stay in the lobby instead of racing -- the lobby otherwise
gets about three seconds before `--autostart` tears it down, which is not long
enough to watch names, hovercraft selection or chat.

This is genuinely two `ZCom_Control` client instances in two processes over a
real ENet socket, and it caught real bugs. It is still **not** a substitute for
two machines: same host, same clock, loopback only, no MTU or packet loss, no
firewall.

### What to check in the logs

`LobbyState` logs every player entry it pushes to the Flash GUI, which is
otherwise unobservable (the Hikari call is write-only -- it returns
`<undefined/>` and the SWF cannot be queried). Three lines, covering the three
ways a player can reach the lobby:

```
[LobbyState]: existing user 20 'Player1'                        <- present before this state activated
[LobbyState]: join user 21 '' (name not yet replicated, deferred)  <- joined while we were watching
[LobbyState]: update user 21 'Player2' Cloudera/USAHovercraft   <- a replicated change arrived
```

A correct two-client run shows **both** players named on **both** clients. The
empty-name `join`/`update` lines are expected and benign: the node exists
before its owner has pushed a name, `onJoin` defers it into `mDelayedUsers`,
and the later non-empty update promotes it via `addUser`.

Beware: `LobbyState::onPlayerUpdate` returns early in that delayed-user branch.
A log line placed after the early return misses exactly the case you care about
-- a late-arriving replicated name -- and makes working replication look
broken. That cost an investigation; the line now sits at the top of the
function.

## The real thing: two machines

Verified so far on one machine only. The two-machine run is still to do.

### 1. Get the runtime onto the second machine

On the build machine, after `build-and-deploy.ps1`:

```powershell
.\scripts\package-for-second-machine.ps1
```

That copies `C:\hu-modern-run` (~220 MB) into `dist\second-machine\runtime\`
**inside the project folder**, which is Synology-synced, so it reaches the
other machines by itself. `dist/` is gitignored -- it is build output, and it
belongs in the sync rather than the repo.

`C:\hu-modern-run` is **not** reproducible from `local-game/` -- it holds
hand-fixed runtime config and DLLs (see CLAUDE.md) -- so it is copied
wholesale rather than rebuilt.

Then on the second machine, from the synced project folder:

```powershell
.\dist\second-machine\install.ps1
```

which installs to that machine's own `C:\hu-modern-run`. Do **not** run the
game straight out of the synced folder: these disks are network-backed and
slow, and `run-test-session.ps1` expects `C:\hu-modern-run` anyway.

**The debug CRT.** This is a `Debug|Win32` build, so the exe imports
`msvcp140d.dll` / `vcruntime140d.dll` / `ucrtbased.dll`. Microsoft ships the
debug CRT only with Visual Studio and excludes it from the redistributable, so
a machine without VS -- exactly what a second test machine is -- cannot start
the game. The packaging script bundles those three DLLs for that reason. Fine
between your own machines; not something to hand to anyone else. The real fix,
when the port gets there, is a Release build, which needs only the ordinary
VC++ redistributable.

### 2. Open the ports on the hosting machine

The game uses **two** UDP ports, because chat is a completely separate
`ZCom_Control` pair from the game:

| port | what |
|---|---|
| 2375 | game (`HUServerCore`) |
| 2377 | chat (`ChatServer`) |

Both must be reachable or you get a lobby that works with no chat. Run this
**yourself**, elevated, on the hosting machine -- it changes firewall
configuration, so it is deliberately not scripted:

```powershell
New-NetFirewallRule -DisplayName "Hovercraft Universe (UDP 2375,2377)" -Direction Inbound -Protocol UDP -LocalPort 2375,2377 -Action Allow
```

Remove it again with
`Remove-NetFirewallRule -DisplayName "Hovercraft Universe (UDP 2375,2377)"`.

### 3. Run it

Hosting machine -- server **and** a local client, so the host is a player too:

```powershell
.\scripts\run-test-session.ps1 -Mode multiplayer -Clients 1 -NoStart -Seconds 0
```

`-NoStart` matters here: without it the host's client autostarts the race
before the other machine has finished joining. Start it from the lobby GUI
once both players are in.

Use plain `-Mode server` only if the host is not playing. That gives one human
plus a **bot**, which does not test client-to-client replication -- see the
top of this document for why a bot is not a second client.

Joining machine (get the host's LAN IP with `ipconfig`):

```powershell
.\scripts\run-test-session.ps1 -Mode join -HostAddress 192.168.1.42 -Clients 1 -Seconds 0
```

Or join from the GUI: **Multiplayer -> Join game -> type the IP -> OK**. That
is the same `MainMenuState::onConnect()` path `--autoconnect` drives, so both
routes exercise identical code.

Collect logs on **both** machines afterwards:

```powershell
.\scripts\run-test-session.ps1 -Collect
```

### Known limitation, not a port bug

The client ignores any `:port` you give it and always connects to 2375.
`MainMenuState::onConnect()` has carried a `TODO: Parse IP and Port?` since
2010 and the GUI "Join game" box has the same limitation, so this is original
behaviour, not a regression. `--host=ip:port` parses the port in `main.cpp` but
`onConnect()` drops it.

## Race progress is on stdout, not in the Ogre log

`RaceState::onCheckPoint()`, `onStart()` and `onFinish()` report with
`std::cout`, not `Ogre::LogManager`. So the only record of who passed which
checkpoint is **`server.stdout.log`, in the run folder of the session that
launched the server** -- not `DedicatedServer.log`, and not anything
`-Collect` produces (that snapshots the Ogre logs only).

This is the one place to look when a race does not end:

```
21 reaches correct checkpoint 0
20 reaches correct checkpoint 0
20 reaches correct checkpoint 1
21 reaches correct checkpoint 1
21 reaches correct checkpoint 2
finish ID is 3
20 reaches incorrect finish, skipped checkpoint 2
```

Checkpoints are **strictly sequential**. `RacePlayer::addCheckpoint()` only
accepts `checkpoint == mLastCheckpoint + 1` and silently ignores anything else,
and `RaceState::onFinish()` refuses the finish unless the player's next
expected checkpoint *is* the finish. Miss one and the rest of the lap cannot be
completed, with no in-game feedback whatsoever -- you simply drive through the
finish line and nothing happens. That is original 2010 behaviour, not a port
regression, and it is an easy thing to mistake for a broken race.

## Verified so far

Two clients + dedicated server, one machine (31/07/2026):

- Both clients connect, get distinct connection IDs, and see **two**
  `PlayerSettings` and two `RacePlayer` nodes -- no bot is added, because
  `RaceState` only fills empty slots (`bots = maxPlayers - humans`), so
  `FillWithBots=1` is harmless once the humans are in. `MaximumPlayers` in
  `Server.ini` is the setting that actually gates them, and the script refuses
  to launch more clients than it allows.
- Each client receives exactly one "own hovercraft".
- **Both clients display both player names**, i.e. owner -> authority ->
  proxy replication of a human's name works in both directions.
- Both reach `RACING` in the same second. Zero errors in any log.

Two machines over a real LAN, host running server-only (31/07/2026):

- Client on the second machine connected to `192.168.0.182:2375`, reached the
  lobby, saw the host-side player named correctly, **chat worked**, and the
  race ran to `RACING` with zero errors in any log. So the lobby fixes in
  b3ec550 and the chat fix hold up over a real network, not just over the
  in-process transport they were diagnosed against.
- The human did not finish, because they missed checkpoint 2 -- correct game
  behaviour, confirmed in `server.stdout.log` (see above).

Still unverified: **two humans on two machines**. The run above had the host on
`-Mode server` with no local client, so the second racer was a bot, and
client-to-client replication over a real network is still untested. Use the
`-Mode multiplayer -Clients 1 -NoStart` recipe above on the host for that.
