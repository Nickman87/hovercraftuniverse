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

Copy the whole `C:\hu-modern-run` folder. It is **not** reproducible from
`local-game/` -- it holds hand-fixed runtime config and DLLs (see CLAUDE.md).
Copy it wholesale rather than rebuilding it, then deploy new binaries onto it
with the normal script.

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

Hosting machine:

```powershell
.\scripts\run-test-session.ps1 -Mode server -Seconds 0
```

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

Still unverified: anything that needs two machines -- real latency, MTU, packet
loss, firewall, and the InitEvent/interceptor ordering fixes under a handshake
that does *not* complete synchronously (see phase-b-replication.md §11, and the
lobby fixes in commit b3ec550, which were all found against an in-process
transport that collapses several round trips into one call).
