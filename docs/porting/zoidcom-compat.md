# ZoidCom compatibility shim (Phase A)

## Why this exists

The game (`HovercraftUniverse/**`) is built against **ZoidCom**, a
closed-source networking library ("Zoidcom Automated Networking System",
Copyright 2002-2007 by Joerg Rueppel). The vendor is gone; only prebuilt
VC9 (`_MSC_VER` 1500-era) binaries survive under
`HovercraftUniverse/dependencies/zoidcom/`, and those cannot link against
the MSVC v143 (Win32/x86) toolchain this porting effort targets. There is
no source to port, no vendor to ask, and no newer binary to fetch. ZoidCom
is one of the two hard blockers for the modern build (the other being
Havok).

The fix is a **drop-in, API-compatible replacement written from scratch**:
`compat/zoidcom/**`, a new directory owned by this porting effort (not a
port of anything under `HovercraftUniverse/`). It matches the real
ZoidCom headers (`HovercraftUniverse/dependencies/zoidcom/include/zoidcom/
*.h`, used here strictly as an API reference) closely enough that game
code doing `#include <zoidcom/zoidcom.h>`, `#include <zoidcom/
zoidcom_node.h>` or `#include <zoidcom/zoidcom_control.h>` compiles
unchanged, with the same class names, method signatures, enum values and
macros.

## Phase A scope (this document describes what actually landed)

Phase A's job was: get every ZoidCom-dependent file to **compile and
link**, wire real behavior underneath wherever that was cheap and safe,
and stub the rest honestly (a `TODO(phaseB):` comment plus a runtime log
line, not silent no-ops). It was explicitly **not** Phase A's job to
reimplement ZoidCom's replication/synchronization engine -- that's Phase
B, scoped at the bottom of this document.

## Design: ENet underneath

`compat/zoidcom/src/Control.cpp` wraps a real
[ENet](http://enet.bespin.org/) host (`vcpkg`'s `enet` port, exposed as
the `unofficial-enet` CMake config package -- the port ships no plain
`enet` config package, only `unofficial-enet`, confirmed against
`C:\Users\dirk\vcpkg\installed\x86-windows\share\{enet,unofficial-enet}`).
Every wire-level ZCom_Control operation -- socket setup, connect,
disconnect, the input/output pump, and raw `ZCom_sendData()`/
`ZCom_cbDataReceived()` -- is backed by real ENet calls, not stubs.

A `ZCom_BitStream` is genuinely implemented as a self-contained bit-level
serialization buffer (see "BitStream fidelity" below); it is the substrate
everything else (raw sends, and Phase B's future replication protocol)
serializes into.

What is **not** real yet is the ZoidCom-specific layer built on top of
raw connections: node registration/linking, data replication, event
routing between linked nodes, interpolation, Zoidlevels, and authority
migration. Those are the subject of Phase B (see below). Every stub logs
`TODO(phaseB): <what> -- <why>` exactly once per distinct call site via
`zshim::todoPhaseBOnce()`, so a real playtest session will show, once each,
everything that still needs Phase B work, without spamming the log.

## API surface actually used by the game (enumeration)

Enumerated by grepping `ZCom_`/`zoidcom`/`eZCom_` across
`HovercraftUniverse/Networking/**`, the ZoidCom-touching parts of
`HovercraftUniverse/CoreEngine/**`, and `HovercraftUniverse/
HovercraftUniverse/**`.

| Symbol / area | Where used | Status |
|---|---|---|
| `ZoidCom` (ctor variants, `Init`/`Clear`) | `main.cpp` | **Real** -- owns process-wide `enet_initialize`/`enet_deinitialize`, refcounted |
| `ZoidCom::Sleep`, `ZoidCom::getTime` | `NetworkClient`/`NetworkServer::process()` | **Real** |
| `ZoidCom::setLogLevel/setConnectionTimeout/setResendTimeout` | rarely used | Accepted; timeout knobs are `TODO(phaseB)` (not plumbed to ENet peer timeouts) |
| `ZoidCom::overrideMemoryHandlers` | not used by game, implemented anyway | **Real** -- honored by all `operator new`/`delete` in the shim |
| `ZCom_Control` (base class), `ZCom_initSockets` | `NetworkClient`, `NetworkServer`, `ChatClient`, `ChatServer` | **Real** -- creates an `ENetHost` |
| `ZCom_setDebugName`, `ZCom_setUpstreamLimit` | ctors | **Real** (upstream limit maps to `enet_host_bandwidth_limit`) |
| `ZCom_requestDownstreamLimit`, `ZCom_requestZoidMode` | `HUClient`/`HUServerCore`/`ChatClient` | `TODO(phaseB)` stub (logged, returns/no-ops) |
| `ZCom_registerClass`/`ZCom_getClassID` | `NetworkIDManager` | **Real** -- name-to-id table per `ZCom_Control` |
| `ZCom_Connect`/`ZCom_Disconnect`/`ZCom_disconnectAll` | `NetworkClient` | **Real** over ENet -- see "known semantic gap" below |
| `ZCom_processInput`/`ZCom_processOutput` | per-tick pump | **Real** -- drives `enet_host_service`/`enet_host_flush` |
| `ZCom_processReplicators` | per-tick pump | `TODO(phaseB)` stub -- no replicator is ever polled |
| `ZCom_sendData`/`ZCom_cbDataReceived` | direct data channel | **Real** over ENet |
| `ZCom_sendDataToGroup` (`ZCOM_CONNGROUP_ALL` only) | not used by current game code | **Real** for the "all" pseudo-group; named groups are `TODO(phaseB)` |
| `ZCom_sendDataRaw` | not used | **Real**, via `enet_socket_send` |
| `ZCom_getPeer`/`ZCom_getConnectionStats`/`ZCom_get/setUserData` | not used | **Real** bookkeeping; stats pull `roundTripTime`/`packetsLost` etc. from the ENet peer |
| `ZCom_simulateLag`/`ZCom_simulateLoss` | debug aid, not used | `TODO(phaseB)` stub |
| `ZCom_Discover`/`ZCom_setDiscoverListener`/`ZCom_cbDiscover*` | overridden (return `false`) everywhere | `TODO(phaseB)` stub (LAN discovery) |
| All `ZCom_cb*` connection callbacks | `NetworkClient`/`NetworkServer`/`ChatClient`/`ChatServer` | **Real** dispatch, with one important caveat -- see below |
| `ZCom_Address` (`setAddress`, `getAddressIP`, `resolveHostname`, ...) | `NetworkClient::connect` | **Real**, backed by ENet's own resolver; async resolution is `TODO(phaseB)` (always resolves synchronously) |
| `ZCom_Node` (ctor/dtor, `registerNode{Unique,ByTag,Dynamic}`) | `NetworkEntity` | **Real bookkeeping** (class id, role, control pointer, authority network-id assignment); **no cross-network linking** -- `TODO(phaseB)` |
| `ZCom_Node::getRole/getControl/getClassID/getNetworkID/isPrivate` | pervasive (`Entity.cpp`, `ChatEntity.cpp`, `Lobby.cpp`, `RaceState.cpp`, ...) | **Real** accessors over the bookkeeping above |
| `ZCom_Node::checkEventWaiting/getNextEvent` | `NetworkEntity::processEvents` | `TODO(phaseB)` stub -- always empty (no event ever arrives) |
| `ZCom_Node::sendEvent/sendEventDirect/sendEventToGroup` | `NetworkEntity::sendEvent`/`sendEventDirect` templates | `TODO(phaseB)` stub -- logs once, deletes the stream, returns `false` |
| `ZCom_Node::addReplicationInt/Float/Bool/String/StringW` | `NetworkEntity::replicate*`, `Lobby.cpp`, `RaceState.cpp`, `PlayerSettings.cpp`, `Hovercraft.cpp` | `TODO(phaseB)` stub -- recorded, never synced |
| `ZCom_Node::addInterpolationInt/Float` | commented out in `Entity.cpp` (dead code path) | `TODO(phaseB)` stub |
| `ZCom_Node::addReplicator`/`setReplicationInterceptor`/`setEventInterceptor` | `NetworkEntity::setReplicationInterceptor`, custom replicators | **Real bookkeeping** (ownership/autodelete); driving them is `TODO(phaseB)` |
| `ZCom_Node::setOwner`/`setMustSync`/`setSyncResult*`/Zoidlevel methods | not used by current game code | `TODO(phaseB)` stub |
| `ZCom_Node::sendFile`/`acceptFile`/`getFileInfo` | not used | `TODO(phaseB)` stub |
| `ZCom_BitStream` (all `add*`/`get*`/`skip*`, `Serialize`/`Deserialize`, `saveReadState`/`restoreReadState`, `isEqual`, `Duplicate`) | universally, especially `EntityPropertySystem.cpp`'s hand-rolled ADD/UPDATE/REMOVE/RESET protocol | **Real, from-scratch implementation** -- see fidelity notes below |
| `ZCom_ReplicatorSetup`, `ZCom_Replicator` base (flags/setup/peek plumbing) | `OgreVector3_Replicator`, `OgreQuaternion_Replicator`, `String_Replicator`, `EntityPropertyMapReplicator` | **Real** bookkeeping |
| `ZCom_ReplicatorBasic` (`checkState`/`packData`/`unpackData`) | the three `*_Replicator` classes above | Base plumbing **real**; nothing calls these virtuals yet (`TODO(phaseB)`, see `ZCom_processReplicators`) |
| `ZCom_ReplicatorAdvanced` (`sendData`/`sendDataDirect`/`onDataReceived`/etc.) | `EntityPropertyMapReplicator` | Base plumbing **real**; `sendData`/`sendDataDirect` are `TODO(phaseB)` stubs (log + drop) |
| `ZCom_NodeReplicationInterceptor`/`ZCom_NodeEventInterceptor` | `PlayerSettingsInterceptor`, `RacePlayer`, `Lobby` | Interfaces compile and can be registered; never invoked (`TODO(phaseB)`) |
| `eZCom_Event`, `eZCom_NodeRole`, `eZCom_ConnectResult`, `eZCom_CloseReason`, `eZCom_ZoidResult`, `eZCom_SendMode`, `eZCom_AddressType`, ... | pervasive | **Real** -- identical enumerators/values to the real header |
| `ZCOM_REPFLAG_*`, `ZCOM_REPRULE_*`, `ZCOM_REPLICATOR_*`, `ZCOM_CLASSFLAG_ANNOUNCEDATA`, `ZCOM_FTRANS_*_BITS` | pervasive | **Real** -- identical macro values (see "local macro shadow" note below) |
| `zU8`/`zU16`/`zU32`/`zU64`/`zS8`/.../`zFloat`/`zDouble` typedefs | pervasive | **Real** -- identical typedefs |

**Rough count**: of roughly 140 distinct symbols/methods enumerated, ~55
are backed by real, working behavior (BitStream in full; Control's
connection lifecycle, class registration, and raw data channel; Node's
identity/role/user-data bookkeeping; Address; the enum/macro/typedef
surface), and ~85 are honest `TODO(phaseB)` stubs (everything that requires
a live cross-network node link: replication, node-to-node events,
interpolation, Zoidlevels/authority migration, file transfer, LAN
discovery, lag/loss simulation).

## `ZCom_BitStream` fidelity notes

This is the one piece Phase A was asked to implement for real, and it is:
a self-contained, growable, bit-addressable buffer with matching
`addX()`/`getX()`/`skipX()` triplets for ints, signed ints, bools, floats,
strings (narrow and wide), raw buffers, and nested bitstreams, plus
`Serialize`/`Deserialize` for turning a stream into a flat byte array (used
directly as the ENet packet payload in `Control.cpp`).

Two deliberate departures from the original vendor's encoding, both
harmless in context:

1. **Bit order and layout are shim-internal, not wire-compatible with the
   real ZoidCom.** The real library's exact bit layout was never fully
   documented in the headers (only behavior, not encoding, is specified),
   and there is no working ZoidCom binary left anywhere to interoperate
   with -- every peer in this codebase now runs this same shim. So the
   implementation only needs to be **self-consistent** (whatever `addX()`
   writes, the matching `getX()` reads back correctly), which is a much
   weaker and easier requirement than bit-for-bit vendor compatibility,
   and was chosen deliberately to keep the implementation simple and
   obviously correct.
2. **Float encoding** truncates the IEEE-754 mantissa to the requested
   `_mant_bits` (plus a full sign bit and 8 exponent bits), which
   reproduces the same *kind* of precision loss the original docs describe
   ("10.723(10) => 10.719", etc.) without claiming to reproduce it
   bit-for-bit.

Also implemented: `ZoidCom::overrideMemoryHandlers()` is honored by every
`operator new`/`delete` in the shim (`ZCom_BitStream`, `ZCom_Replicator`,
`ZCom_ReplicatorSetup`), even though nothing in the current game code calls
it -- it was cheap to do properly.

## Known semantic gap: the connection handshake

Real ZoidCom's connection handshake is **application-negotiated**: the
server's `ZCom_cbConnectionRequest()` runs and can reject a connection
*before* the client is ever told it succeeded, and the client's connection
request can carry an arbitrary `ZCom_BitStream` of application data.

ENet's handshake is **transport-level** and completes (three-way, at the
protocol layer) before any of our application code runs a single line. This
shim cannot fully bridge that gap without adding its own app-level
handshake round-trip on top of ENet's (a Phase B item), so today:

- The connection-request bitstream passed to `ZCom_Connect()` is **not
  transmitted at all** -- ENet's `enet_host_connect()` only carries a
  single 32-bit integer, not an arbitrary payload. The remote side's
  `ZCom_cbConnectionRequest()` always sees an **empty** request stream.
- The client's `ZCom_cbConnectResult()` fires as soon as the ENet
  transport handshake completes, **always** with `eZCom_ConnAccepted`
  (there is no way yet to carry an app-level "denied" decision back
  through ENet's connect handshake).
- The server's `ZCom_cbConnectionRequest()` is called immediately after
  the incoming ENet peer connects. If it returns `false`, the shim
  disconnects the peer immediately (`enet_peer_disconnect_now`) -- but the
  client has *already* seen `ZCom_cbConnectResult(eZCom_ConnAccepted, ...)`
  by that point, and will now unexpectedly see
  `ZCom_cbConnectionClosed()` right after. Real ZoidCom would never have
  told the client it was accepted in this situation.

None of the game's current `ZCom_cbConnectionRequest()` overrides actually
reject a connection (`NetworkServer`/`ChatServer` always return `true`, and
`HUServerCore` also always accepts and just replies with the assigned
connection id) -- so this gap has zero observed effect on current game
behavior. It's flagged here because it's the kind of thing that will bite
if/when connection-request validation or a real handshake payload is
added.

## Re-enabled CoreEngine files (Phase A CMake changes)

`modern/CMakeLists.txt` gained:

- `add_subdirectory(../compat/zoidcom ...)` building `hu_zoidcom_compat`
  (links `unofficial::enet::enet`, plus `ws2_32`/`winmm` on Windows).
- A new `hu_networking` target, mirroring
  `HovercraftUniverse/Networking/CMakeLists.txt`'s source list, linked
  against `hu_zoidcom_compat` + `hu_utils` + `hu_exceptions` + `OgreMain`.
- `hu_coreengine` gained `find_package(Boost REQUIRED COMPONENTS thread)` +
  `Boost::thread` (needed once `EntityPropertySystem.h`'s
  `<boost/thread/mutex.hpp>` came into play) and links `hu_networking` +
  `hu_zoidcom_compat`.

Of the 13 CoreEngine `.cpp` files previously excluded purely by
Networking/ZoidCom `#include` taint, **6 are re-enabled** (verified by an
actual clean build against MSVC v143 -- zero errors, see the build log
summary in the report):

- `FreeroamCameraController.cpp`
- `BasicEntityEvent.cpp`
- `ControllerEvent.cpp`
- `ControllerEventParser.cpp`
- `Entity.cpp`
- `EntityManager.cpp`

The other 7 remain excluded, but **not for ZoidCom/Networking reasons
anymore** -- see `modern/CMakeLists.txt` for the exact per-file notes:

- **`RaceCamera.cpp`/`.h`** -- newly-discovered, pre-existing Ogre API-gap
  breakage (same class of issue as `docs/porting/ogre-api-gap.md`, just
  not caught by that document's Ogre-only pass because the file was
  Networking-blocked at the time): calls `Ogre::Camera::setPosition`/
  `setDirection`/`lookAt`/`setFixedYawAxis`/`pitch`/`yaw`/`roll`/
  `getOrientation`/`getPosition`, none of which exist on `Ogre::Camera` in
  Ogre 14.5.2 anymore.
- **`GameView.cpp`/`.h`** -- transitively includes `RaceCamera.h` (same
  breakage) and additionally calls the removed `Ogre::Light::setPosition()`.
- **`EntityPropertySystem.cpp`/`.h`** -- `<zoidcom/zoidcom.h>` now resolves
  fine, but the `.cpp` uses `std::cout` (around lines 239/252/262) without
  `#include <iostream>` -- a latent, pre-existing bug unrelated to ZoidCom.
- **`EntityRepresentation.cpp`/`.h`** -- still genuinely GUI-tainted: the
  `.cpp` directly `#include`s `"Application.h"`, which pulls in
  `<GUIManager.h>`/`<SoundManager.h>` (GUI/Sound projects, not yet ported).
- **`BasicGameState.cpp`/`.h`**, **`GameStateManager.cpp`**,
  **`RepresentationManager.cpp`/`.h`**, **`Application.cpp`/`.h`** --
  unchanged from before this task, still directly or transitively blocked
  by `<GUIManager.h>`/`<SoundManager.h>`.

Per the task's file-ownership rule, none of the above were "fixed" by
editing `HovercraftUniverse/**` -- they're documented here and left
excluded, exactly as instructed ("if you find a game usage that is
fundamentally incompatible... document it rather than hacking the game").
The Ogre-API-gap items (`RaceCamera`, `GameView`, the missing
`<iostream>`) are better fixed alongside the rest of
`docs/porting/ogre-api-gap.md`'s work, not as part of this ZoidCom task.

`hu_networking` (new target, all 14 `Networking/*.cpp` files, mirroring
`HovercraftUniverse/Networking/CMakeLists.txt`) builds clean against the
shim with zero errors (needed one extra include dir, `HU_ROOT/Utils`, for
`Listenable.h`/`PlayerMap.h`, which `ChatEntity.h` needs).

## What Phase B needs to build

In priority order (each depends on the previous):

1. **Cross-network node linking.** The core missing piece: when node A
   (authority, `ZCom_Control` X) registers, and node B (proxy, connected
   `ZCom_Control` Y) registers the same class, ZoidCom is supposed to link
   them by class id + registration order (unique nodes), tag (tag nodes),
   or an explicit spawn request (dynamic nodes: `ZCom_cbNodeRequest_Dynamic`
   / `registerNodeDynamic`). None of that exists yet -- every `ZCom_Node` is
   an island on its own `ZCom_Control`. This is the prerequisite for
   basically everything else on this list.
2. **A real connection handshake** riding on top of ENet's transport
   handshake, to close the semantic gap described above (send the request
   bitstream as the first reliable packet after `ENET_EVENT_TYPE_CONNECT`;
   gate the client's `ZCom_cbConnectResult()` on the server's reply instead
   of firing it immediately).
3. **Data replication tick.** Once nodes are linked, `ZCom_processReplicators()`
   needs to actually walk each node's registered replication items (the
   `addReplicationInt/Float/Bool/String` list plus any `ZCom_Replicator`
   instances) and, for changed ("dirty") values, pack them into an update
   packet, send it down the (now real) node link, and unpack on the other
   end -- calling `checkState()`/`packData()`/`unpackData()` on
   `ZCom_ReplicatorBasic` subclasses, and `Process()`/`onPreSendData()`/
   `onDataReceived()` on `ZCom_ReplicatorAdvanced` subclasses
   (`EntityPropertyMapReplicator` needs this specifically).
4. **Node-to-node event delivery** (`sendEvent`/`sendEventDirect`/
   `sendEventToGroup` + `checkEventWaiting`/`getNextEvent`), which is what
   `NetworkEntity::sendEvent`/`processEvents` and essentially every game
   event type (`ChatEvent`, `NotifyEvent`, `TextEvent`, and the
   `NetworkEvent<...>`-templated ones) ultimately ride on.
5. **Interpolation** (`addInterpolationInt/Float`) -- lower priority since
   the game only has a commented-out call site today (`Entity.cpp`).
6. **Zoidlevels, `setOwner`/authority migration, `setMustSync`** -- not
   exercised by any current game code path (grep found zero call sites
   outside declarations), so lowest priority; implement on demand.
7. **LAN discovery, file transfer, lag/loss simulation** -- likewise
   unused by current game code; nice-to-haves, not blockers.

## Ambiguities where real ZoidCom's documented semantics were guessed

- **Exact bit layout of `ZCom_BitStream`.** The real header documents
  *behavior* (what each `addX`/`getX` pair does, how many extra bits a
  signed int or float costs) but never nails down the physical bit order.
  Since no real ZoidCom binary survives to test against, Phase A chose the
  simplest self-consistent layout (LSB-first, sign+exponent+truncated-
  mantissa floats) rather than guessing at vendor bit order. See "BitStream
  fidelity notes" above.
- **`ZCom_registerClass()` on a duplicate name.** The header states names
  "must be unique" but doesn't say what happens if you register the same
  name twice. This shim returns `ZCom_Invalid_ID` (0) and logs; real
  ZoidCom's actual behavior here is unknown and unverifiable now.
- **`ZCom_Node::addReplicator()` and `ZCOM_REPLICATOR_INITIALIZED`.** The
  real header implies `ZCom_Node` sets this flag when a replicator is
  registered (`"if replicator is registered to a node and this flag is not
  set, an error occurs"`), which would require `ZCom_Node` to poke a
  protected field on `ZCom_Replicator`. This shim does not grant that
  friendship; instead it relies on the observed fact that every concrete
  replicator subclass in this codebase (`OgreVector3_Replicator`,
  `OgreQuaternion_Replicator`, `String_Replicator`,
  `EntityPropertyMapReplicator`) already sets the flag itself in its own
  constructor -- confirmed by reading all four `.cpp` files.
- **`ZCom_Address::resolveHostname(async=true, ...)`.** Always resolves
  synchronously regardless of the `_async` flag (logged once). No game
  code currently passes `_async=true`.
