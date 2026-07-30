# Phase B — ZoidCom Replication and Node Linking

**Workstream C of [phase-b-plan.md](phase-b-plan.md).** The critical path to a playable game.
Goal: single-player reaches `RaceState::RACING` with a driveable, owned hovercraft.

---

## 1. Why this is the critical path, not the multiplayer part

`MainMenu::onSingleplayer` ([MainMenu.cpp:84-125](../../HovercraftUniverse/HovercraftUniverse/MainMenu.cpp))
starts a full `HUDedicatedServer` on a boost thread in the same process, sleeps a second, then
calls `onConnect("localhost", this)` — the identical code path as a multiplayer "join by IP".
`HUClient("localhost")` sets `mRemote = true` and dials real UDP through ENet.

There is no non-networked path. A loopback mode (`eZCom_AddressLocal`) exists in
`NetworkClient` but only the demo chat system uses it.

**So every mechanism below is required to race at all.** Nothing here can be deferred as
"multiplayer polish".

## 2. Current state

Phase A built real infrastructure and stubbed the semantics.

**Real** (ENet-backed, verified working): socket setup, `ZCom_Connect`/`Disconnect`,
`ZCom_processInput`/`processOutput` genuinely pumping `enet_host_service` and dispatching the
`ZCom_cb*` callbacks, `ZCom_registerClass` with a real name→ID table, raw `ZCom_sendData`,
connection stats from real ENet peers. Plus `ZCom_BitStream` — a complete from-scratch
bit-level serializer, which matters because every payload below rides on it.

**Stubbed** — and the stubs are total. From a live server log
(`C:\hu-modern-run\data\DedicatedServer.log:79-84`), five `TODO(phaseB)` lines fire once each
and then the server sits happily doing nothing:

```
registerNodeDynamic / addReplicationInt / addReplicationBool / registerNodeUnique
[Server]: Ready for incoming connections
ZCom_Control::ZCom_processReplicators
```

`ZCom_Node` today is identity bookkeeping only. Per the shim's own comment
([Node.cpp:7-22](../../compat/zoidcom/src/Node.cpp)): *"every `ZCom_Node` is islanded on its
own `ZCom_Control`."*

## 3. The four mechanisms that matter

Of ~85 stubbed symbols, racing needs four. The rest (Zoidlevels, file transfer, LAN discovery,
lag simulation, interpolation) are not on the path.

### 3.1 Cross-network node linking — the one blocker

This is what makes everything else possible. Today the server creates `Lobby`,
`PlayerSettings`, `RaceState`, `RacePlayer` and 11 world-entity classes as authority nodes, and
the client never hears about any of them. `HUClient::onNodeDynamic`
([HUClient.cpp:137-271](../../HovercraftUniverse/HovercraftUniverse/HUClient.cpp)) — 130 lines
whose entire job is to construct the client-side counterpart of every game object — **is never
invoked at all.**

Consequence: the client connects successfully and then renders an empty lobby forever. No
hovercraft, no player list, no race state.

Two flavours to support:

- **Dynamic** (`registerNodeDynamic`) — per-instance objects. Server assigns a network ID,
  tells the client "build class C as network ID N, here is the announce data". Client's
  `ZCom_cbNodeRequest_Dynamic` fires, game code constructs the object, which registers its own
  node; shim binds that node to N.
- **Unique** (`registerNodeUnique`) — singletons, currently `Lobby`, `PlayerSettings`,
  `RaceState`. Both sides construct the object independently and register with the same class
  ID; linking is by class ID rather than by spawn message. `HUClient::initialize`
  ([HUClient.cpp:45-59](../../HovercraftUniverse/HovercraftUniverse/HUClient.cpp)) already
  creates and registers the client's `Lobby` this way.

Class IDs must agree across peers. They already do: `EntityRegister::registerAll`
([EntityRegister.cpp:32-52](../../HovercraftUniverse/HovercraftUniverse/EntityRegister.cpp)) is
called by both server and client and `ZCom_registerClass` maintains a real name→ID table, so
IDs match by construction as long as registration order matches — which it does, same function.

### 3.2 Node-to-node events

Every discrete game message. Today `sendEvent`/`sendEventDirect` delete the stream and return
`false` ([Node.cpp:278-300](../../compat/zoidcom/src/Node.cpp)), and `checkEventWaiting()`
always returns `false`.

Four independent event namespaces, each with a 5-bit class tag (see
`Networking/NetworkEvent.h`):

| Class | Events | Payload |
|-------|--------|---------|
| 1 `ChatEventType` | `textLine`, `notifyLine` | user + line strings |
| 2 `ControllerEventType` | `BasicEntity` | **5 bools: forward / backward / left / right / reset** — the player input packet, owner→authority every controller poll ([Entity.cpp:245-261](../../HovercraftUniverse/CoreEngine/Entity.cpp)) |
| 3 `GameEventType` | `init`, `stateChange`, `startTrack`, `checkPoint` | `InitEvent` wraps a nested `ZCom_BitStream` state snapshot; `StateEvent` a race-state enum; `StartTrackEvent` empty; `CheckpointEvent` user + checkpoint + timestamp |
| 4 `VisualEventType` | `onCollision` | position + normal, cosmetic, server→client |

Without events: **player input never reaches the server**, the countdown and race-state
transitions never reach the client, and the `RaceState` state machine deadlocks — its
`INITIALIZING → LOADING → INTRO → COUNTDOWN → RACING` progression is driven by client acks
sent as events (`RaceState::SystemState::sendEvent`,
[RaceState.cpp:565-571](../../HovercraftUniverse/HovercraftUniverse/RaceState.cpp)).

So events are second only to linking, and ahead of replication — they unlock the state machine,
whereas replication only makes things *smooth*.

**`eZCom_EventInit` — verified. Our reading was right.** See
[zoidcom-original-semantics.md](zoidcom-original-semantics.md) for the full extracted
reference. It fires **on the authority** when a new proxy/owner links up, confirmed two ways:
by `setEventNotification`'s own header comment ("if a new proxy node connected"), and
decisively by the game's original pre-shim production code
([RaceState.cpp:411](../../HovercraftUniverse/HovercraftUniverse/RaceState.cpp)):

```cpp
if (type == eZCom_EventInit && mNode->getRole() == eZCom_RoleAuthority)
```

…followed by building a snapshot and `sendEventDirect()` to `conn_id`. The event's own
bitstream carries no payload. One nuance: in the Zoidlevel-transition case the docs say it is
received "on both nodes" — irrelevant here since we are not implementing Zoidlevels.

Other findings from that extraction worth having in hand before writing code:

- **`ZCom_cbNodeRequest_Unique` does not exist.** Only `_Dynamic` and `_Tag`, both returning
  `void` with **no decline path** — failing to comply gets the connection disconnected by the
  library. So unique-node linking is purely local class-ID matching, as assumed in §3.1.
- **`ZCOM_REPRULE_PROXY_2_AUTH` does not exist** — only `OWNER_2_AUTH`. Anything expecting a
  generic proxy→authority path is mistaken.
- `mindelay`/`maxdelay` are **milliseconds, best-effort**, and explicitly *not enforced* for
  `ZCom_ReplicatorAdvanced`.
- `setAnnounceData` is set once but **re-evaluated per client** via `outPreReplicateNode()`
  before each announcement; the receiver reads it inside `ZCom_cbNodeRequest_Dynamic/_Tag`.
- `setOwner()` "won't do anything special on its own" — it is a pure permission gate.
- `checkState()` is called once per `ZCom_processOutput()`, **not** once per connection.

**And one real bug it found in our shim:** `registerNodeDynamic`
([Node.cpp:112-116](../../compat/zoidcom/src/Node.cpp)) unconditionally assigns
`eZCom_RoleAuthority`, but the real semantics require `eZCom_RoleProxy` when it is called
from inside `ZCom_cbNodeRequest_Dynamic()`. Harmless today because no dynamic-spawn delivery
exists — and a guaranteed source of confusion the moment step 1 below starts working, since
every client-side entity would claim to be authoritative. **Fix it as part of step 1.**

Original context, retained: the game relies on `eZCom_EventInit` to deliver
a one-shot full-state snapshot to a newly-linked node — `Lobby`, `PlayerSettings` and
`RaceState` each build their own snapshot inside an `InitEvent`
([Lobby.cpp:309-315](../../HovercraftUniverse/HovercraftUniverse/Lobby.cpp),
[PlayerSettings.cpp:108-114](../../HovercraftUniverse/HovercraftUniverse/PlayerSettings.cpp),
[RaceState.cpp:411-415](../../HovercraftUniverse/HovercraftUniverse/RaceState.cpp)).
Our reading is that vendor ZoidCom raised it on the **authority** node when a new proxy
appeared, so the authority could respond with a direct snapshot — but that is inference from
call sites, and getting the direction wrong deadlocks the race state machine.

The original headers survive in `archive-mirror/Win-Package - ZoidCom.zip`. **Extract them and
confirm the documented semantics before writing this.** Same for the precise meaning of
`ZCOM_REPFLAG_MOSTRECENT` and the `maxdelay` argument.

### 3.3 The replication tick

`ZCom_processReplicators` is a log-once stub, so all registered fields are recorded into a
bookkeeping vector ([Node.cpp:205-270](../../compat/zoidcom/src/Node.cpp)) and never synced.

The full spec, extracted from every `setupReplication()` in the game:

| Class | Fields | Rule |
|-------|--------|------|
| `Entity` (base — all game objects) | `mPosition`, `mOrientation`, `mVelocity` (custom replicators), `EntityPropertyMap` | `AUTH_2_ALL`, `MOSTRECENT`, maxdelay 1000 |
| `Hovercraft` | `mDisplayName`, `mDescription`, `mAcceleration`, `mMass`, `mMaximumSpeed`, `mSpeed`, `mSteering` (float, 4 mantissa bits), `mCollisionState` (bool), `mPlayerId` (int32) | `AUTH_2_ALL`, mostly `MOSTRECENT` |
| `Asteroid` | `mDisplayName`, `mGravity`, `mAsteroidType` | `AUTH_2_ALL` |
| `CheckPoint` | `mDisplayName`, `mNumber` | `AUTH_2_ALL` |
| `SpeedBoost` | `mBoost` | `AUTH_2_ALL` |
| `StartPosition` | `mPlayerNumber` (8 bits) | `AUTH_2_ALL` |
| `Lobby` | `mTrack` (8b), `mAdmin` (32b), `mCurrentPlayers` (8b), `mMaximumPlayers` (8b), `mBots` (bool) | mix of `OWNER_2_AUTH \| AUTH_2_ALL` and `AUTH_2_ALL` |
| `PlayerSettings` | `mPlayerName`, `mHovercraft` (4b), `mCharacter` (4b) | `OWNER_2_AUTH \| AUTH_2_PROXY` |
| `RaceState` | `mNumberPlayers` (8b), `mCountdown` (custom width, delay 250–1000 ms) | `AUTH_2_ALL` |
| `RacePlayer` | `mPlayerPosition` (4b), `mFinished` (bool) | `AUTH_2_ALL` |

Note the game reaches ZoidCom through its own wrappers in
`NetworkEntity` (`replicateUnsignedInt` / `replicateFloat` / `replicateOgreVector3` /
`replicateOgreQuaternion` / `replicateString`), so the surface to implement is narrower than
the raw ZoidCom API.

Also note **announce data is separate from replication** and delivered once at spawn:
`Entity`'s name / ogreEntity / processInterval / initial transform, `Hovercraft`'s display
name + player ID, `PlayerSettings`' connection + user IDs, `RaceState`'s track filename.
`ZCom_Node::setAnnounceData` currently accepts the stream and immediately deletes it
([Node.cpp:192-196](../../compat/zoidcom/src/Node.cpp)).

### 3.4 Custom replicator dispatch — larger than it first appears

The game reaches ZoidCom through its own wrappers in `NetworkEntity`, and **four of those
wrappers do not use ZoidCom's primitive replication at all** — they construct game-authored
`ZCom_Replicator` subclasses and hand them to `addReplicator`
([NetworkEntity.cpp:110-126](../../HovercraftUniverse/Networking/NetworkEntity.cpp)):

| Wrapper | Class | Base | Used for |
|---------|-------|------|----------|
| `replicateOgreVector3` | `OgreVector3_Replicator` | `ZCom_ReplicatorBasic` | **`mPosition`, `mVelocity`** on every entity |
| `replicateOgreQuaternion` | `OgreQuaternion_Replicator` | `ZCom_ReplicatorBasic` | **`mOrientation`** on every entity |
| `replicateString` | `String_Replicator` | `ZCom_ReplicatorBasic` | every `mDisplayName` / `mDescription` / `mPlayerName` |
| — | `EntityPropertyMapReplicator` | `ZCom_ReplicatorAdvanced` | the property map on every entity |

Only `replicateUnsignedInt` / `replicateFloat` / `addReplicationBool` use ZoidCom's built-in
primitives.

**This corrects the ordering assumption made earlier in planning.** Custom replicator dispatch
is not a late refinement for the property map — it is *the* mechanism by which a hovercraft's
position and orientation reach the client. Implementing the primitive replication tick alone
would sync the lobby and the HUD numbers but leave every craft frozen at its spawn point.

So step 3 of the implementation order must drive `ZCom_ReplicatorBasic`'s
`checkState()` / `packData()` / `unpackData(stream, store, estimated_time_sent)` for
`addReplicator`-registered instances, alongside the primitive paths. Verified against
`OgreVector3_Replicator.h:67,76` — the subclasses implement exactly those virtuals.

### 3.5 `ZCom_ReplicatorAdvanced` dispatch

The game ships a hand-rolled `ZCom_ReplicatorAdvanced` subclass —
`EntityPropertyMapReplicator` ([EntityPropertySystem.h:280-377](../../HovercraftUniverse/CoreEngine/EntityPropertySystem.h),
impl `EntityPropertySystem.cpp:191-522`) — implementing its own ADD / UPDATE / REMOVE / RESET
wire protocol for a dynamic property map attached to every `Entity`. Every game object has one.

It needs its virtuals actually driven: `sendData` / `sendDataDirect` / `Process` /
`onDataReceived`. Currently they drop silently
([Replicator.cpp:111-123](../../compat/zoidcom/src/Replicator.cpp)).

This is the fiddliest item because we're driving someone else's protocol through our shim
rather than implementing a known one, and it is why it's ordered last of the four.

## 4. Wire protocol

Our shim is self-consistent but **not wire-compatible with vendor ZoidCom**. That's acceptable
— every peer runs this same shim — but it does mean a modern client can never talk to a 2010
server. Noted as a deliberate divergence in the plan's §5.

Message types, layered on ENet channels (reliable vs unreliable chosen per type):

| Message | Direction | Channel | Payload |
|---------|-----------|---------|---------|
| `NODE_CREATE` | auth → proxy | reliable ordered | class ID, network ID, role for receiver, announce-data blob |
| `NODE_LINK_UNIQUE` | auth → proxy | reliable ordered | class ID, network ID |
| `NODE_REMOVE` | auth → proxy | reliable ordered | network ID |
| `NODE_EVENT` | both | reliable ordered | network ID, event type, `ZCom_BitStream` payload |
| `NODE_REPL` | both | unreliable for `MOSTRECENT`, else reliable | network ID, changed-replicator bitmap, packed values |
| `NODE_OWNER` | auth → proxy | reliable ordered | network ID, owner connection ID |

`ZCom_BitStream` already handles the packing, including nested streams (which `InitEvent`
needs).

Design notes:

- **Unreliable for `MOSTRECENT`.** That flag means "only the latest value matters" — exactly
  UDP-unreliable semantics. Sending position updates reliably would head-of-line block.
- **Dirty tracking.** Keep a shadow copy per replicator, send on change. For `MOSTRECENT`
  fields honour `maxdelay` as a resend interval so a dropped final update doesn't leave a
  stale value forever.
- **Batch per node per tick** rather than one datagram per field. `Entity` alone has 4
  replicators and there will be dozens of entities.

## 5. Implementation order

Strictly sequential — each step is only testable once the previous works.

1. **Node registry + linking.** Network-ID map on `ZCom_Control`; `NODE_CREATE` /
   `NODE_LINK_UNIQUE`; make `ZCom_cbNodeRequest_Dynamic` fire. Wire `setAnnounceData` to
   actually carry its payload.
   *Test:* client log shows `HUClient::onNodeDynamic` invoked and a `PlayerSettings` constructed
   client-side. This alone proves the whole architecture.
2. **Events.** `sendEvent` / `sendEventDirect` / `checkEventWaiting` / `getNextEvent`, and
   `eZCom_EventInit` — after confirming its semantics against the original headers.
   *Test:* lobby chat round-trips; clicking Start produces a countdown on the client; the
   `RaceState` machine advances past `INITIALIZING`.
3. **Replication tick.** `ZCom_processReplicators` walking each node's registered fields, both
   directions per rule — covering **both** the primitive paths (`addReplicationInt` / `Float` /
   `Bool`) **and** `ZCom_ReplicatorBasic` subclasses registered via `addReplicator`
   (`checkState` / `packData` / `unpackData`). Per §3.4 the latter is what actually moves
   position and orientation, so it is not separable from this step.
   *Test:* lobby player count and track selection sync (primitive path); a hovercraft's
   position changes on the client (custom path).
4. **`ZCom_ReplicatorAdvanced`.** Drive the property-map virtuals.
   *Test:* `SpeedBoost::onLeave`'s property removal ([SpeedBoost.cpp:64](../../HovercraftUniverse/HovercraftUniverse/SpeedBoost.cpp)) propagates.
5. **`setOwner`.** Currently a no-op ([Node.cpp:184-188](../../compat/zoidcom/src/Node.cpp)),
   which silently breaks `Lobby`'s admin assignment ([Lobby.cpp:181](../../HovercraftUniverse/HovercraftUniverse/Lobby.cpp))
   and `RaceState`'s per-connection ownership ([RaceState.cpp:77](../../HovercraftUniverse/HovercraftUniverse/RaceState.cpp)).
   *Test:* the local player gets a `HovercraftPlayerController` — i.e. input is accepted —
   because `onNodeDynamic` only attaches one when `role == eZCom_RoleOwner`.

Steps 1–2 get to a race *starting*. Steps 3–5 make it work.

## 6. Implementation hazards

Found while reading the shim and the game's networking layer. Both are cheap to get right up
front and unpleasant to debug later.

### 6.1 Server and client share one process in single-player

Because `onSingleplayer` runs the server on a thread in the same process, **two
`ZCom_Control` instances live in one address space** — and any shim state that is `static` or
file-global is silently shared between them.

`Node.cpp` already has one such global:

```cpp
// compat/zoidcom/src/Node.cpp:29
std::atomic<ZCom_NodeID> gNextNetworkId(1);
```

Today that is harmless — only authority nodes allocate IDs, and only the server creates them.
It stops being harmless the moment Phase B adds a node registry: **the network-ID space,
the registry, and any pending-link bookkeeping must live in `ZCom_Control_Private`, not at
file scope.** A shared registry would let the client resolve a network ID to the *server's*
node object and replicate into it directly — which would appear to work in single-player and
fail utterly over a real network. That is the worst possible failure mode: the bug hides
exactly where testing is easiest.

Corollary for testing: **a genuine two-process test is mandatory**, not optional polish. Any
single-player-only validation of this workstream is untrustworthy by construction.

`zshim::todoPhaseBOnce`'s log-once state is fine to keep global — one message per process is
the desired behaviour.

### 6.2 A latent bug in `networkRegisterUnique`

[NetworkEntity.cpp:55-58](../../HovercraftUniverse/Networking/NetworkEntity.cpp):

```cpp
void NetworkEntity::networkRegisterUnique(NetworkIDManager* idmanager, std::string name,
        bool authority) {
    networkRegisterUnique(idmanager->getID(name), idmanager->getControl());
    //                                                    ^ `authority` is dropped
}
```

The `authority` argument is silently discarded and the two-argument call falls back to the
default. This is currently **harmless by luck**: the only caller of this overload is
`HUClient.cpp:58`, which wants proxy behaviour and passes nothing, while the server registers
its `Lobby` through the other overload with an explicit `true`
([HUServerCore.cpp:31](../../HovercraftUniverse/HovercraftUniverse/HUServerCore.cpp)).

Leave it alone for now — it is pre-existing 2010 behaviour and fixing it changes nothing
observable. But note it here, because once node linking works, a future caller passing
`true` would get a proxy and the resulting "unique node never becomes authority" symptom
would look exactly like a linking bug in our new code.

## 7. Acceptance criteria

- Single-player: menu → lobby with the local player listed → Start → countdown → `RACING`,
  with a hovercraft spawned, owned, and responding to input.
- Two clients on one machine (or a LAN): both appear in each other's lobby, both see the other
  craft move.
- No `TODO(phaseB)` lines in a clean race log for any of the four mechanisms above.
- Chat works in lobby and in race — cheap, and it exercises events end to end independently of
  gameplay.

## 8. Deliberately out of scope

Not needed for racing, left stubbed and logged:

- **Zoidlevels** (`ZCom_requestZoidMode`) — only the demo chat system gates on them.
- **Interpolation** (`addInterpolationFloat`) — the only call site is commented out
  ([Entity.cpp:286-291](../../HovercraftUniverse/CoreEngine/Entity.cpp)). We do our own
  smoothing now anyway, from the 60 Hz work in `89335b4`.
- **LAN discovery, file transfer, lag/loss simulation.**
- **App-level handshake payload.** The connect-request bitstream isn't transmitted
  ([Control.cpp:9-28](../../compat/zoidcom/src/Control.cpp)), but no game code sends one —
  `ClientConnectThread` passes `0`. There is no auth, no password and no protocol-version
  check in this game. Worth noting as a security property of the original design rather than
  something Phase B introduces: **any client that can reach the port is accepted** if the lobby
  has room and no race is active.
