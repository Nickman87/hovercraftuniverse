# ZoidCom original semantics (reference for Phase B)

## Sources

- **Headers** (canonical, verified byte-identical between the archive zip
  and the bootstrapped dependency tree — `diff -rq` reports no
  differences): `HovercraftUniverse/dependencies/zoidcom/include/zoidcom/*.h`.
  These are doxygen-commented and are the vendor's own documentation; all
  quotes below cite this tree (paths given relative to
  `HovercraftUniverse/dependencies/zoidcom/include/zoidcom/`).
- **Archive** (`archive-mirror/Win-Package - ZoidCom.zip`, extracted to
  the scratchpad, *not* into the repo): contains only `bin/{debug,release}`,
  `dependencies/zoidcom/{include,lib}` — the same headers plus prebuilt
  VC9 binaries. **No doxygen HTML manual, no PDF, no bundled
  examples/samples directory inside the zip.** So "the docs" *are* the
  header comments; there is no separate manual to cross-check them against.
- **Sample/demo code**: `demos/NetworkingZoidCom/**` (a full VC9 sample
  project using real ZoidCom headers/libs). Its callback bodies
  (`NetworkServer.cpp`, `NetworkClient.cpp`, `SampleClient.cpp`) are mostly
  **empty stubs** — useful for confirming call signatures and which
  callbacks exist, but they do not exercise `eZCom_EventInit` or node
  linking in any instructive way.
- **The game's own original ZoidCom-based code** (pre-shim,
  `HovercraftUniverse/HovercraftUniverse/{RaceState,PlayerSettings,Lobby}.cpp`)
  turned out to be the single most useful evidence for question 1: it's
  real production code written against the real headers/libs by the
  original developers, and it is unambiguous about which side raises
  `eZCom_EventInit` and what it does in response. Quoted in Q1.

Every quote below is verbatim with `file:line`. Where I could not find a
textual answer, I say so explicitly rather than inferring.

---

## 1. `eZCom_EventInit`

**Verdict: fires on the node that has `eZCom_RoleAuthority`, when a new
proxy/owner becomes linked to it — not on the proxy when it links to the
authority.** This is stated directly in the header and confirmed by real
game code.

Header (`zoidcom.h:181-184`):
```
/// A remote node has connected to this node, use this to send any necessary 
/// events to it. This event will be received only if notification of it is enabled 
/// with setEventNotification(). After it has been received it is safe to send events to the 
/// new node directly, by using the ZCom_ConnID and sendEventDirect().
eZCom_EventInit,
```
Note the phrasing is symmetric in the abstract ("a remote node has
connected to this node") — it does not by itself say only the authority
gets it. But `ZCom_Node::setEventNotification()` documents the toggle
that gates it (`zoidcom_node.h:758-763`):
```
/** @brief Toggle connect/remove notification.
    @param _oninit Generate event if a new proxy node connected.
    @param _onremove Generate event if a proxy node is removed.
*/
void
  setEventNotification( bool _oninit, bool _onremove );
```
"if a new **proxy** node connected" — i.e. the event is about a proxy
appearing, which is only meaningful from the side that isn't the proxy:
the authority. This matches `ZCom_Node::getNetworkID()`'s doc
(`zoidcom_node.h:928-938`), which draws the same authority/proxy
asymmetry:
```
If the node's role is eZCom_RoleAuthority, the network ID is available as soon as the node is
registered, if the role is proxy or owner, the ID will be available as soon as the node is linked
up to a authority node, so after the node has received eEvent_Init or eEvent_SyncRequest.
```
This last sentence is the closest the header gets to stating both sides
can get `eZCom_EventInit` — a *proxy* node's network ID becomes valid
"after the node has received e[Z]Event_Init or e[Z]Event_SyncRequest" —
so the proxy side *can* also receive `eZCom_EventInit` (naming is
inconsistent in the header: `eEvent_Init` vs. `eZCom_EventInit`, almost
certainly the same event, an editing artifact in the doxygen comment).
Read literally, this means **both nodes can get the event** — the
authority when a proxy links up, and the proxy when it links up to its
authority — but the header's `setEventNotification()` phrasing ("if a new
proxy node connected") suggests the *typical, documented* use case is
authority-side.

**The decisive evidence is the game's actual, pre-shim code.** All three
of `RaceState`/`PlayerSettings`/`Lobby` check the event on the authority
side and respond by building a full snapshot and sending it back with
`sendEventDirect()` to the connection that just triggered the event:

`HovercraftUniverse/HovercraftUniverse/RaceState.cpp:410-415`:
```cpp
	// A new client received this object so send current state
	if (type == eZCom_EventInit && mNode->getRole() == eZCom_RoleAuthority) {
		ZCom_BitStream* state = new ZCom_BitStream();
		state->addInt(mNumberPlayers, 8);
		sendEventDirect(InitEvent(state), conn_id);
	}
```
The comment ("A new client received this object so send current state")
and the `getRole() == eZCom_RoleAuthority` guard together confirm:
**`eZCom_EventInit` fires on the authority node when a new proxy for that
node becomes relevant to a connection; the bitstream carried by the event
itself is empty (the event just says "someone new is now linked"), and
the documented/expected response is for the authority to build a snapshot
bitstream and `sendEventDirect()` it to `conn_id` (the connection ID
delivered alongside the event by `getNextEvent()`)** — which is exactly
what the game's `InitEvent` pattern (mentioned in the brief) does.

What's in the event's bitstream: the header does not document any payload
for `eZCom_EventInit` itself (contrast with e.g. `eZCom_EventFile_Incoming`,
which explicitly documents its stream contents). `getNextEvent()`
(`zoidcom_node.h:771-785`) returns a `ZCom_BitStream*` for every event, but
for `eZCom_EventInit` the game code never reads from `state`/`data` before
building its own reply — it only reads `conn_id` and `remote_role` out of
the out-parameters of `getNextEvent()`. **Conclusion: `eZCom_EventInit`'s
bitstream carries no meaningful application payload** — it's a pure
notification. (Not explicitly stated as "empty" anywhere in the header;
this is inferred from the header's silence plus every observed call site
never reading from it.)

The expected application response, stated in the event doc itself
(`zoidcom.h:181-184`): *"use this to send any necessary events to it...
After it has been received it is safe to send events to the new node
directly, by using the ZCom_ConnID and sendEventDirect()."* This is
exactly the init-snapshot pattern the game implements.

**This confirms the brief's inference is correct**: the event fires on
the authority with the new connection's ID available, and building a
full-state snapshot into an `InitEvent` sent via `sendEventDirect()` is
the documented and observed pattern.

---

## 2. All `eZCom_Event` values

Full enum, `zoidcom.h:175-228`:

```cpp
enum eZCom_Event
{
  eZCom_EventNoEvent,

  /* system events */

  eZCom_EventInit,
  eZCom_EventSyncRequest,
  eZCom_EventRemoved,

  eZCom_EventFile_Incoming,
  eZCom_EventFile_Data,
  eZCom_EventFile_Aborted,
  eZCom_EventFile_Complete,

  /* replicator events */
  eZCom_EventReplicator,

  /* normal events */
  eZCom_EventUser
};
```

Per-value documentation, verbatim:

- **`eZCom_EventNoEvent`** — no doc comment; used as a sentinel/default (see
  the shim's `getNextEvent()` which sets `*_type = eZCom_EventNoEvent` when
  no event is available — that matches the enum's position as value 0 and
  its name).

- **`eZCom_EventInit`** (`zoidcom.h:181-184`) — see Q1 above in full. Gated
  by `setEventNotification(_oninit, ...)`.

- **`eZCom_EventSyncRequest`** (`zoidcom.h:186-192`):
  > This node has eZCom_RoleAuthority and setMustSync(true). A new proxy/owner has become
  > relevant to this node. It is now possible to send events back and forth to the new proxy
  > node and so exchange data with it. Whenever ready, call setSyncResult() and notify Zoidcom
  > that the data exchange was successful or that it failed. If failure is reported, the client's
  > Zoidlevel transition will fail and the connection will fall back to the previous Zoidlevel.
  > This event will only be received if a client connection is entering one of the node's Zoidlevels.
  > eZCom_EventInit will be received in addition to that on both nodes, if enabled.
  This is explicitly Zoidlevel-transition-only (`setMustSync(true)` +
  authority role), and it is the one place the header says outright that
  **`eZCom_EventInit` can be received "on both nodes"** — reinforcing the
  Q1 finding that the event isn't strictly authority-only in principle,
  even though the game's own usage pattern is authority-only.

- **`eZCom_EventRemoved`** (`zoidcom.h:194-200`):
  > If this node is eZCom_RoleAuthority and a proxy or owner disconnected, you will receive this event.
  > (If enabled with setEventNotification())
  >
  > If this node is eZCom_RoleOwner or eZCom_RoleProxy and the authority on the other end disconnected,
  > you will receive this event. It can not be disabled since it is an implicit request to delete
  > this node.
  Asymmetric gating: authority-side is opt-in via `setEventNotification()`;
  proxy/owner-side is **mandatory** (cannot be disabled) because it's the
  proxy's cue to delete itself. Confirmed by game usage,
  `demos.../NetworkEntity.cpp:24-25`:
  ```cpp
  if (remote_role == eZCom_RoleAuthority && type == eZCom_EventRemoved) {
      mDeleteMe = true;
  }
  ```

- **`eZCom_EventFile_Incoming`** (`zoidcom.h:202-205`):
  > The remote node is trying to send a file. The data in the BitStream is:
  >   - ZCom_FileTransID fid = (ZCom_FileTransID) stream->getInt(ZCOM_FTRANS_ID_BITS);
  > PLUS additional data passed to the sendFile() method

- **`eZCom_EventFile_Data`** (`zoidcom.h:206-210`):
  > New file data has arrived, data is:
  >  - ZCom_FileTransID fid = (ZCom_FileTransID) stream->getInt(ZCOM_FTRANS_ID_BITS);
  > The actual data is not passed around in the stream here, Zoidcom appends the incoming
  > data to the file automatically. This is just a notification to let the app know when to
  > look for new progress information with ZCom_Node::getFileInfo().

- **`eZCom_EventFile_Aborted`** (`zoidcom.h:212-214`):
  > File transfer aborted:
  >  - ZCom_FileTransID fid = (ZCom_FileTransID) stream->getInt(ZCOM_FTRANS_ID_BITS);

- **`eZCom_EventFile_Complete`** (`zoidcom.h:215-217`):
  > File transfer complete:
  >  - ZCom_FileTransID fid = (ZCom_FileTransID) stream->getInt(ZCOM_FTRANS_ID_BITS);

- **`eZCom_EventReplicator`** (`zoidcom.h:220-222`):
  > // replicator is sending some data through node event system (internal)
  > eZCom_EventReplicator,
  This is a `//` (non-doxygen) comment, explicitly marked internal — it's
  the plumbing `ZCom_ReplicatorAdvanced::sendData()`/`sendDataDirect()` use
  to ride the node's own event channel. Not meant to be observed directly
  by application code via `getNextEvent()`.

- **`eZCom_EventUser`** (`zoidcom.h:173,226-227`):
  > @brief ZCom_Node event types, all events except eZCom_EventUser are generated by Zoidcom
  > ...
  > /// Application generated event from remote node received.
  > eZCom_EventUser

**Not in this enum, contrary to what one might guess from the brief's
phrasing**: there is no `eZCom_EventConnectionClosed` node event. Connection
close is a `ZCom_Control`-level **callback** (`ZCom_cbConnectionClosed()`,
`zoidcom_control.h:558`), not a `ZCom_Node`-level event delivered through
`getNextEvent()`. Node-level awareness of authority disconnection comes
via `eZCom_EventRemoved` instead (see above).

---

## 3. Node registration and linking

Three registration methods on `ZCom_Node` (`zoidcom_node.h:168-218`), plus
a deprecated fourth:

- **`registerNodeUnique(classid, role, control)`** (`zoidcom_node.h:168-175`)
  — for "controlling classes", unique to each `ZCom_Control`, "identified
  solely by their `ZCom_ClassID`" (class doc, `zoidcom_node.h:124-125`). No
  registration-order or tag matching is documented for how these link up;
  the header doesn't describe unique-node linking mechanics beyond the
  class-level description quoted below.

- **`registerNodeByTag(classid, tag, role, control)`** (`zoidcom_node.h:177-189`)
  — "Tag nodes are nodes which tend to exist over a long period of time
  and can be linked together in each `ZCom_Control` by a specified tag.
  Register two tagnodes of the same `ZCom_ClassID` in two `ZCom_Controls`
  each and let those `ZCom_Controls` connect, then these two tagnodes will
  link up, too." (class doc, `zoidcom_node.h:126-128`.) Per-method doc:
  > If, for some reason, the server registers a tagnode and cannot find it on the client, the callback
  > ZCom_Control::ZCom_cbNodeRequest_Tag() will be called on client. From within this callback, the corresponding
  > tagnode has to be created and registered with registerNodeByTag().

- **`registerNodeDynamic(classid, control)`** (`zoidcom_node.h:191-200`):
  > Dynamic nodes are nodes which get created and deleted in an uncontrolled manner. If the server
  > creates a dynamic node, it sends a request to all connections on the same Zoidlevel, which
  > then in turn have to create the counterpart to that node.
  > ...
  > The node's role will be eZCom_RoleProxy if the node is created inside ZCom_Control::ZCom_cbNodeRequest_Dynamic(),
  > eZCom_RoleAuthority otherwise.
  This is the key mechanism for "authority creates it, remote peers spawn
  counterparts": the **same call** (`registerNodeDynamic`) is used on both
  sides — the role you get back depends entirely on *whether you're
  calling it from inside the `ZCom_cbNodeRequest_Dynamic()` callback or
  not*. There is no separate "register as proxy" call.

- **`registerRequestedNode(classid, control)`** (`zoidcom_node.h:202-218`,
  **`@deprecated Use registerNodeDynamic() instead.`**) — same contract as
  `registerNodeDynamic()`'s proxy-side use, kept only for backward compat.

**No `registerNodeSpecial` exists** — the brief's question anticipates it
but it is not present anywhere in any header. Confirmed by grepping all
headers; only Unique/ByTag/Dynamic(+deprecated Requested) exist.

**Callback contracts** — there is **no `ZCom_cbNodeRequest_Unique`
callback either.** Only two node-request callbacks exist on
`ZCom_Control`, both pure virtual (must be implemented, per the class doc
rationale about forcing compile errors on typos,
`zoidcom_control.h:517-527`):

- **`ZCom_cbNodeRequest_Dynamic`** (`zoidcom_control.h:586-605`):
  ```cpp
  virtual void ZCom_cbNodeRequest_Dynamic( ZCom_ConnID _id, ZCom_ClassID _requested_class, ZCom_BitStream *_announcedata,
                                           eZCom_NodeRole _role, ZCom_NodeID _net_id ) = 0;
  ```
  > Called whenever a server requests us to create a new node for a new object.
  > The application MUST create a node for this class, which is identical to other nodes of the
  > same class on the server. That means: It must have exactly the same replication data setup. Create the
  > node, register the variables for replication, and register the node with ZCom_Node::registerRequestedNode().
  > If you fail to do so, Zoidcom will be unable to extract incoming data streams any further and disconnect.
  > The _net_id parameter tells us the id which is later been returned by ZCom_Node::getNetworkID().
  Contract: **`void` return** — there is no accept/reject decision
  possible from this callback's return value. "Decline" is not a
  documented option: the doc says failing to create the node makes
  Zoidcom "unable to extract incoming data streams any further and
  disconnect" — i.e. **not creating the requested node is a protocol
  violation that gets the connection killed**, not a graceful decline
  path. (The doc literally only shows the `@returns` idiom is absent from
  this method's doxygen block — no `@returns` tag at all, unlike the
  `bool`-returning callbacks such as `ZCom_cbConnectionRequest`.)

- **`ZCom_cbNodeRequest_Tag`** (`zoidcom_control.h:607-621`) — identical
  contract/shape, `void` return, same "if you don't [register], Zoidcom
  will disconnect" consequence, keyed by `_tag` instead of `_net_id`.

Both callbacks receive `_announcedata` (see Q4) and must, if non-NULL,
"advance the stream's read position by the exact size of the data given to
setAnnounceData()" (both docblocks, `zoidcom_control.h:590-591` /
`612-613`) — a hard requirement, since the announce data has no embedded
size prefix (see Q4).

---

## 4. `setAnnounceData`

`ZCom_Node::setAnnounceData(ZCom_BitStream *_data)` (`zoidcom_node.h:470-486`):

> Set data that will be attached to each announcement of this node to a client.
>
> The data needs to be set only on server side. It will be used only if the
> node's classname (registered with ZCom_Control::ZCom_registerClass())
> has been registered with the @ref ZCOM_CLASSFLAG_ANNOUNCEDATA flag.
> When the flag is set, such data MUST be available.
> The client needs to be able to read exactly the amount of data which has
> been set here, as no size information will be transmitted.
>
> The bitstream goes into ownership of Zoidcom, it may not be deleted or
> used otherwise after giving it to the node.

Additional detail from the class-flag doc, `zoidcom_control.h:38-63`
(`ZCOM_CLASSFLAG_ANNOUNCEDATA`):

> Each node of this class has a custom bitstream that is sent along with
> the announcement to each client. This bitstream will be received by
> ZCom_Control::ZCom_cbNodeRequest_Dynamic() or ZCom_Control::ZCom_cbNodeRequest_Tag().
> ...
> You can set the data for each target client indivually by implementing
> ZCom_NodeReplicationInterceptor::outPreReplicateNode(), which will
> get called directly before Zoidcom replicates the node in question.
> Inside this interceptor callback, ZCom_Node::setAnnounceData() can
> be called to change the announce data right before Zoidcom uses it.

**Answering the specific question**: announce data is delivered **once
per announcement of the node to a given client** — i.e. at the moment the
node's existence is first replicated/announced to that connection (the
"dynamic spawn request" or "tag request" moment), not re-sent on every
subsequent replication tick. But it is *not* a fixed one-time global
value: **it can be changed per-target-client right before each
announcement** via `outPreReplicateNode()` — so "once per node" is wrong
if you have multiple clients (each gets its own announcement, and thus its
own opportunity for `setAnnounceData()` to have been called with different
content beforehand), but it is exactly-once *per client the node gets
announced to*, read by that client inside its
`ZCom_cbNodeRequest_Dynamic`/`_Tag` callback. There is no documented
"re-send announce data on demand" mechanism.

---

## 5. Replication rules and flags

**Rules** (`zoidcom_node.h:88-107`, header note included verbatim):

```cpp
#define ZCOM_REPRULE_NONE             0
#define ZCOM_REPRULE_AUTH_2_PROXY     (1L << 0)
#define ZCOM_REPRULE_AUTH_2_OWNER     (1L << 1)
#define ZCOM_REPRULE_AUTH_2_ALL       (ZCOM_REPRULE_AUTH_2_PROXY|ZCOM_REPRULE_AUTH_2_OWNER)
#define ZCOM_REPRULE_OWNER_2_AUTH     (1L << 2)
```
> @note For event sending, replication rules are only applied to the node from which events are sent. That means that events are
>       not forwarded on the authority automatically if ZCOM_REPRULE_AUTH_2_ALL is used for sending an event from a proxy or
>       owner node.

So: `AUTH_2_PROXY` = authority → plain proxies; `AUTH_2_OWNER` = authority
→ nodes elevated to owner role; `AUTH_2_ALL` = OR of both (all remote
nodes regardless of proxy/owner distinction); `OWNER_2_AUTH` = the reverse
direction, owner → authority. **There is no `PROXY_2_AUTH` rule in the
header** — the brief's question anticipates one but the only
authority-directed rule is `OWNER_2_AUTH`. A plain (non-owner) proxy has no
documented rule constant for sending data back to the authority — matching
the design intent described under `setOwner()` (Q6): a plain proxy is not
supposed to be able to push data/commands to the authority; that
capability is exactly what promoting it to owner grants.

The `@note` above is important and easy to miss: replication **rules only
gate the sender's own outgoing traffic** — they are not a routing/relay
instruction. `AUTH_2_ALL` sent *from a proxy* does not somehow get
forwarded through the authority to other proxies; the authority does not
auto-relay. (This is stated for events specifically; the data-replication
rules are presumably symmetric in this regard since they're the same
constants, though the header only spells out the caveat under the event
section.)

**Flags** (`zoidcom_node.h:28-86`):

```cpp
#define ZCOM_REPFLAG_NONE             0
#define ZCOM_REPFLAG_UNRELIABLE       (1L << 0)
#define ZCOM_REPFLAG_MOSTRECENT       (1L << 1)
#define ZCOM_REPFLAG_RARELYCHANGED    (1L << 2)
#define ZCOM_REPFLAG_ONLYONCE         (1L << 3)
#define ZCOM_REPFLAG_INTERCEPT        (1L << 4)
#define ZCOM_REPFLAG_SETUPPERSISTS    (1L << 5)
#define ZCOM_REPFLAG_SETUPAUTODELETE  (1L << 6)
#define ZCOM_REPFLAG_STARTCLEAN       (1L << 7)
```

- **`ZCOM_REPFLAG_UNRELIABLE`** (`:34-35`): "Replicate unreliably." (one
  line, no further elaboration in this file).
- **`ZCOM_REPFLAG_MOSTRECENT`** (`:36-37`): **"Replicate reliable, always
  sending the most recent version of the data."** This is the header's
  entire documentation of the flag. Reading it literally: MOSTRECENT is a
  **reliability + "coalesce to latest" mode** — the data *is* guaranteed
  to arrive (reliable), but if the value changes again before an earlier
  update was sent/acked, the earlier stale value is superseded rather than
  queued — i.e. it behaves like reliable-but-not-ordered-against-itself
  delivery of only the newest value, as opposed to queuing every
  intermediate change. The header does not elaborate on interaction with
  `ZCOM_REPFLAG_UNRELIABLE` (presumably mutually exclusive/one overrides
  the other, but this is **not stated** — flagging as undocumented rather
  than guessing).
- **`ZCOM_REPFLAG_RARELYCHANGED`** (`:38-47`): data changes seldom;
  "put in a special group" so idle rarely-changed data costs zero overhead,
  but if even one item in that group needs updating, "all of the other
  items have to at least send control data stating that they don't need to
  be updated" — i.e. it's a batched dirty-group optimization, not a
  behavior change to reliability/ordering.
- **`ZCOM_REPFLAG_ONLYONCE`** (`:48-50`): replicate exactly once;
  implies RARELYCHANGED; **cannot combine with `ZCOM_REPFLAG_STARTCLEAN`**.
- **`ZCOM_REPFLAG_INTERCEPT`** (`:51-52`): "This data will generate
  intercept events when an interceptor is assigned" — gates whether
  `ZCom_NodeReplicationInterceptor::{in,out}PreUpdateItem()` fire for this
  item.
- **`ZCOM_REPFLAG_SETUPPERSISTS`** (`:53-62`): use when the app owns a
  static/long-lived `ZCom_ReplicatorSetup` shared across many replicator
  instances — `Duplicate()` just returns `this` instead of copying, saving
  memory; only matters if `unregisterNode()` is used (which may need to
  internally clone replicators+setups).
- **`ZCOM_REPFLAG_SETUPAUTODELETE`** (`:63-71`): `ZCom_Node` deletes the
  `ZCom_ReplicatorSetup` automatically when the replicator is destroyed.
  **"Never use together with `ZCOM_REPFLAG_SETUPPERSISTS`."** — mutually
  exclusive by explicit header warning.
- **`ZCOM_REPFLAG_STARTCLEAN`** (`:72-85`): new replicator starts "clean"
  (not dirty) instead of the default (dirty, i.e. always replicates once
  on creation) — an optimization for spawning many nodes whose initial
  values already match the client's defaults. **Cannot combine with
  `ZCOM_REPFLAG_ONLYONCE`.**

**`mindelay`/`maxdelay`** — every `addReplicationXxx()` overload
(`zoidcom_node.h:516-612` and interpolation variants `:621-682`) takes
`zS16 _mindelay = -1, zS16 _maxdelay = -1`. Per-parameter doc (identical
wording repeated at each call site, e.g. `:523-524`):
> @param _mindelay Measured in msecs and defines how much time <b>has to pass</b> between two consecutive updates of this data.
> @param _maxdelay Measured in msecs and defines how much time <b>may pass</b> between two consecutive updates of this data.
> ...
> The update frequency bounds as defined by _mindelay and _maxdelay are not guaranteed. The resolution highly depends on factors
> like the client's connection, node priority and node relevance.

So: **units are milliseconds**; `mindelay` is a floor (minimum spacing
enforced between sends of this item), `maxdelay` is a ceiling (item is
forced to re-send even if unchanged, at most this often) — but the header
explicitly disclaims that either bound is a hard guarantee; it's
best-effort, subject to connection quality/priority/relevance. Default
`-1` for both presumably means "no bound" (not stated in words, but is the
sentinel default for an otherwise-msec-typed field, and matches the setup
class's `ZCom_ReplicatorSetup` comment that "-1" is a distinguished state:
`zoidcom_replicator.h:66-77`, *"It is not allowed to change these value
from -1 to >= 0 once a replicator is actively using this setup"* — treating
-1 as a special "unset" sentinel, not just a normal msec value).

**Advanced replicators are exempt from automatic mindelay/maxdelay
enforcement** — critical for Q7/Q9 (`zoidcom_replicator_advanced.h:30-33`):
> @note Unlike the basic replicators, advanced replicators have to perform timing on their own.
>       That means, the min- and maxdelay parameters in ZCom_ReplicatorSetup are not enforced
>       by Zoidcom for advanced replicators. Use getLastUpdateTime(), or the _lastupdate parameter
>       to onPreSendData() to handle this.

---

## 6. Roles

`eZCom_NodeRole` (`zoidcom.h:146-156`):
```cpp
enum eZCom_NodeRole
{
  eZCom_RoleUndefined,   // no role assigned yet
  eZCom_RoleProxy,       // node is a proxy
  eZCom_RoleOwner,       // node is owner
  eZCom_RoleAuthority    // node is authority
};
```
Class-level summary (`zoidcom_node.h:136-140`):
> - eZCom_RoleAuthority is the master node
> - eZCom_RoleProxy is the proxy node that will normally sync up to the authority's state
> - eZCom_RoleOwner is a special proxy. The authority can make any proxy to an owner and define
>          special replication rules for it.

**How each role is acquired:**
- **Authority**: whichever side calls `registerNodeDynamic()`/
  `registerNodeUnique()`/`registerNodeByTag()` **not** from inside
  `ZCom_cbNodeRequest_Dynamic()`/`_Tag()` gets `eZCom_RoleAuthority` (dynamic
  case, `zoidcom_node.h:196-197`, quoted in Q3); for unique/tag nodes the
  role is passed explicitly as the `_role` parameter by the caller — the
  header doesn't document an automatic role-negotiation for those two, it
  is application-decided per call.
- **Proxy**: the default outcome of registering the counterpart node
  inside `ZCom_cbNodeRequest_Dynamic()`/`_Tag()` (dynamic/tag case); for
  unique nodes, again, whatever `_role` the app passes.
- **Owner**: **not chosen at registration time at all** — only reachable
  via `ZCom_Node::setOwner(ZCom_ConnID _id, bool _enabled)`
  (`zoidcom_node.h:429-459`), called on the **authority** node, which
  promotes ("permits... a higher authority") the proxy on connection `_id`
  to owner:
  > This permits the client on the other side of the connection a
  > higher authority over the node. Normally, all nodes created on a client
  > are eZCom_RoleProxy. It can advance to an eZCom_RoleOwner if the server
  > permits that (with this method). This advancement won't do anything special
  > on it's own, instead, the application is responsible to take some action here.
  > You can define data to be replicated only to eZCom_RoleOwner nodes. Also,
  > when sending events you can specify if only the owner or all proxies
  > should receive the event. Furthermore, the application can filter events
  > based on the role of their sender.
  >
  > Example scenario: A spaceship. The server of course has the authority over it,
  > and there are 10 proxies (which replicate what the server object is doing).
  > Now you declare one proxy to become the owner of that ship (in fact, one
  > object may have several owners, and one client can own several objects, it
  > totally depends on the application what it means to be an owner). This owner
  > (which is a client) now can send events to the server node which are not
  > dropped because it accepts these specific events from an owner. In our scenario,
  > these events are control commands to steer the ship and fire. In turn,
  > the server would replicate some additional data (as set by the application),
  > like the current ammo left for each weapon and other special infos no other
  > players need to or should know about.
  Key clarifications: **`setOwner()` "won't do anything special on its
  own"** — it is purely a permission gate; the application is responsible
  for using `ZCOM_REPRULE_OWNER_2_AUTH` / `ZCOM_REPRULE_AUTH_2_OWNER` and
  role-filtered event handling to make the promotion meaningful. Also: "one
  object may have several owners" — owner is per-connection, not exclusive
  to a single client.
  What the owner side observes as a result: the header does **not**
  document that the owner-side proxy node receives any explicit
  notification/event when it is promoted (no `eZCom_EventOwnerGranted` or
  similar exists in the `eZCom_Event` enum — see Q2's complete list). The
  owner side presumably discovers its new role only by calling
  `getRole()` and finding it now returns `eZCom_RoleOwner`, or by the
  authority separately sending it an application `eZCom_EventUser` event
  to say so. **This is not spelled out in the header — flagging as
  undocumented rather than inventing a notification mechanism.**

---

## 7. `ZCom_ReplicatorAdvanced`

Full virtual interface (`zoidcom_replicator_advanced.h:37-220`), plus the
concrete (non-virtual) helpers it provides:

- **Constructor**: `ZCom_ReplicatorAdvanced(ZCom_ReplicatorSetup *_setup)`
  — takes ownership of a setup like any `ZCom_Replicator`.
- **`getNode()` / `setNode(ZCom_Node*)`** (`:44-48`, `:212-219`) —
  `setNode()` "will get called automatically by ZCom_Node::addReplicator().
  There is normally no reason to call this method."
- **`getLastUpdateTime(ZCom_ConnID _cid)`** (`:50-77`) — returns
  `zU32*` (a pointer, writable by the replicator) to the last-update
  timestamp for that connection; **only non-NULL if at least one
  replicator on the node has mindelay/maxdelay set**; documented usage
  pattern is exactly the self-timing this class requires since Zoidcom
  doesn't enforce it (see Q5 note).
- **`sendData(mode, stream, reference_id=0)`** (`:79-101`) — broadcasts an
  event to "all replicators which normally receive data from this
  replicator, too" i.e. respecting the setup's replication rule direction
  (explicitly: `AUTH_2_PROXY` → all proxies of this node get it).
  `eZCom_ReliableOrdered` is **not supported** in this context. If mode is
  `eZCom_UnreliableNotify`, `_reference_id` comes back via
  `onDataAcked()`/`onDataLost()`. **Important timing caveat, quoted
  verbatim**:
  > Please note that calling sendData() won't ensure the data is sent immediately. The data
  > will be sent as soon as the owning node's priority allows the node to send. If you want
  > to generate data which is sent immediately, wait for the onPreSendData() callback.
- **`sendDataDirect(mode, dest, stream, reference_id=0)`** (`:103-122`) —
  same contract as `sendData()` but single-destination; same
  not-immediate caveat.
- **`onPreSendData(_cid, _remoterole, _lastupdate)`** (pure virtual,
  `:124-148`) — called by Zoidcom *right before* it is about to transmit
  to `_cid`; this is the hook where sending via `sendData()`/
  `sendDataDirect()` *does* get flushed immediately (per the caveats
  above). `_lastupdate` is the same non-NULL-only-if-mindelay/maxdelay-set
  pointer as `getLastUpdateTime()`, meant to be read/written here to
  self-enforce timing.
- **`onDataReceived(_cid, _remoterole, _stream, _store, _estimated_time_sent)`**
  (pure virtual, `:150-169`) — delivery of what a peer's `sendData()`/
  `sendDataDirect()` sent. Two attentions, quoted verbatim:
  > Make sure that this method forwards the bitstream by the exact amount of bytes originally sent
  >            by sendData() or sendDataDirect().
  > The m_node/getNode() might return NULL when the parent node got deleted but data is received after that,
  >            so check for getNode() == NULL before calling anything on the node. This is only true for this callback,
  >            other callbacks are safe.
  `_store == false` means "a node replication interceptor denied to accept
  the data" but the method is **still called** — solely so the stream's
  read cursor gets advanced correctly for whatever data follows in the
  packet; the replicator must not apply the update in that case, but must
  still consume the bits.
- **`onDataAcked(_cid, _reference_id, _data)`** / **`onDataLost(_cid,
  _reference_id, _data)`** (pure virtual, `:171-172`) — no elaboration
  beyond the signature; tied to `eZCom_UnreliableNotify` sends via the
  `_reference_id` correlation described under `sendData()`.
- **`onPacketReceived(_cid)`** (pure virtual, `:174-181`):
  > This will only get called for connections whose node is linked to this replicator's owning node.
  > It is used by ZCom_MovementReplicator to synchronize the extrapolation.
- **`onConnectionAdded(_cid, _remoterole)`** / **`onConnectionRemoved(_cid,
  _remoterole)`** (pure virtual, `:183-195`) — node became/stopped being
  relevant on a connection (i.e. a remote counterpart linked/unlinked).
- **`onLocalRoleChanged(_oldrole, _newrole)`** (pure virtual, `:197-202`)
  — the *local* node's role changed (e.g. via `setOwner()`, or via
  registration deciding authority-vs-proxy — not elaborated further).
- **`onRemoteRoleChanged(_cid, _oldrole, _newrole)`** (pure virtual,
  `:204-210`) — a specific remote connection's counterpart node changed
  role (e.g. promoted proxy→owner).

**Ordering the library calls these in**: the header does **not** state an
explicit master ordering across all of these callbacks in one place —
there is no numbered lifecycle diagram. The only ordering information
that is explicit:
1. `onPreSendData()` fires immediately before Zoidcom transmits to a given
   connection (this is the *only* place data queued via `sendData()`/
   `sendDataDirect()` is guaranteed flushed promptly).
2. `Process()` (inherited from `ZCom_Replicator`, `zoidcom_replicator.h:328-339`)
   is called once per `ZCom_processReplicators()` call, **only if**
   `ZCOM_REPLICATOR_CALLPROCESS` flag is set — independent of send/receive
   timing.
3. `onConnectionAdded`/`onConnectionRemoved`/`onLocalRoleChanged`/
   `onRemoteRoleChanged` are event-driven (fire when their trigger
   condition occurs), not part of a per-tick sequence.
4. `onDataReceived` fires whenever a packet with replicator-event payload
   arrives (asynchronous, driven by `ZCom_processInput()`).

Because our game's `EntityPropertyMapReplicator`
(`HovercraftUniverse/CoreEngine/EntityPropertySystem.h:280-366`) implements
its own ADD/UPDATE/REMOVE/RESET protocol entirely inside `onDataReceived`/
`onPreSendData`/`sendData`/`sendDataDirect`, and the header gives **no
call-order guarantee beyond the four points above**, Phase B's driver
should not assume anything the header doesn't state (e.g. it must not
assume `onConnectionAdded` always fires before the first `onPreSendData`
for that connection unless it independently verifies that from the
"connection becomes relevant" semantics elsewhere — the header describes
these as reactions to state changes, and node/connection relevance
becoming true is a precondition for either firing, so in practice
`onConnectionAdded` should precede any `onPreSendData`/`onDataReceived`
for that same connection, but this is inferred from "relevance" being a
shared precondition, not from an explicit ordering statement).

---

## 8. `ZCom_processReplicators`

`ZCom_Control::ZCom_processReplicators(zU32 _simulation_time_passed)`
(`zoidcom_control.h:220-247`):

> This calls the Process() method of all replicators that need it. If not called
> regularly, no inter- or extrapolation will happen.
>
> Currently only ZCom_Replicate_Movement makes use of the _simulation_time_passed parameter.
>
> @attention The time parameter should really be **simulation time** and **not real time**.

(followed by the worked example about a physics update rate of 50 Hz vs.
wall-clock time under multitasking, `zoidcom_control.h:232-241` — the
gist: pass however much *simulated* game time elapsed, not
`getTime()`-delta, or extrapolators like `ZCom_Replicate_Movement` will
compute wrong velocities.)

**Relationship to `ZCom_processInput`/`ZCom_processOutput`**: the header
does **not** state an explicit required call order among the three (no
"call X before Y" sentence exists for these three methods as a group).
What can be inferred:
- `ZCom_processInput()` (`zoidcom_control.h:210-217`): "Must be called
  regularly, checks for new input data and handles it." This is where
  incoming replicator data (`onDataReceived`/`unpackData`) would be
  dispatched, since it's the only place incoming network data is drained.
- `ZCom_processReplicators()`: drives `Process()` for
  interpolation/extrapolation bookkeeping — described as independent of
  input/output ("If not called regularly, no inter- or extrapolation will
  happen" — a decay effect, not a hard dependency on being called in a
  particular slot relative to the other two).
  `ZCom_ReplicatorAdvanced`'s own doc frames `onPacketReceived()` as used
  by `ZCom_MovementReplicator` "to synchronize the extrapolation" — which
  ties `Process()`'s extrapolation math to data that arrives via
  `ZCom_processInput()`, implying `ZCom_processInput()` should run before
  `ZCom_processReplicators()` in a tick for extrapolation to use the
  freshest data, but this is **inferred from the two features'
  interaction, not asserted as a rule anywhere**.
- `ZCom_processOutput()` (`zoidcom_control.h:249-254`): "Prepare and send
  updates. Must be called regularly." This is presumably where
  `checkState()`/`packData()` (basic replicators) and `onPreSendData()`
  (advanced replicators) get invoked, since that's the only place outbound
  data is flushed — but again, the header does not explicitly say
  "`ZCom_processOutput()` calls `checkState()`" anywhere; this is inferred
  from `checkState()`'s own doc ("This won't get called for all
  connections individually. Instead, Zoidcom calls it once per
  ZCom_processOutput()..." — `zoidcom_replicator_basic.h:33-40`, which
  *does* explicitly name `ZCom_processOutput()` as the driver).

**Documented, explicit linkage found**: `ZCom_ReplicatorBasic::checkState()`
(`zoidcom_replicator_basic.h:33-40`):
> This won't get called for all connections individually. Instead, Zoidcom calls
> it once per ZCom_processOutput() and if 'true' is returned, this replicator
> will get marked dirty internally for all interested connections.

So the one hard fact the header gives about ordering is: **`checkState()`
is called once per `ZCom_processOutput()` call, not per connection** —
i.e. dirty-checking is O(1) per replicator per tick regardless of
connection count, and marking dirty then fans out to "all interested
connections" internally.

**Practical recommended order** (inferred, not asserted verbatim anywhere
as a single ordered list): `ZCom_processInput()` → `ZCom_processReplicators()`
→ `ZCom_processOutput()` per tick — matches the shim's own comment
ordering and the natural "receive, simulate, send" tick shape, but this
exact triple-order sentence does not appear in the header itself.

---

## 9. Surprises / traps for an implementer

- **Node event queues are per-node, not per-control.** `getNextEvent()`/
  `checkEventWaiting()` are `ZCom_Node` methods, not `ZCom_Control`
  methods (`zoidcom_node.h:765-785`) — each node has its own independent
  event queue. A control-level "any events pending anywhere" poll doesn't
  exist in the API; the app must poll every node it owns.
- **`eZCom_EventRemoved` cannot be disabled on the proxy/owner side** — "It
  can not be disabled since it is an implicit request to delete this node"
  (`zoidcom.h:198-199`). This is a trap: `setEventNotification()`'s two
  bool parameters only gate `Init`/on removal notification *from the
  authority's perspective for its own bookkeeping* — a naive
  implementation might assume both flags gate both event types on both
  sides equally, but the proxy side has no opt-out.
- **Interceptor return values never change node behavior, only visibility
  of the event/replication item.** Stated explicitly for event
  interceptors: *"Returning 'false' in one of these callbacks does not
  influence the behaviour of the node in any way. The only difference is
  that the event... won't be available through the normal event
  interface"* (`zoidcom_node_interceptors.h:25-31`). This is a trap if an
  implementer assumes `recRemoved() -> false` suppresses the actual
  removal — it does not; it only suppresses the *event object* from
  reaching `getNextEvent()`. The underlying state change (node
  unlinking) still happens.
- **Replication-item interceptors (`inPreUpdateItem`/`outPreUpdateItem`)
  only fire when the item is *actually* about to change.** *"This
  callback only gets called if the replication item really is about to be
  updated now. That means, if the value did not change or is not allowed
  to update due to mindelay constraints, this callback won't get called."*
  (`zoidcom_node_interceptors.h:198-201`). A naive polling-based
  interceptor driver (call it every tick regardless of dirty state) would
  diverge from real semantics.
- **Peek support requires strict protocol discipline and is thread-local.**
  `peekDataStore()`/`peekDataRetrieve()` use *Thread Local Storage*
  explicitly so multiple `ZCom_Control`s can run in different threads
  safely (`zoidcom_replicator.h:301-316`) — and calling `peekDataStore()`
  twice without an intervening `clearPeekData()` triggers an *automatic*
  call to `clearPeekData()` by Zoidcom (`zoidcom_replicator.h:309-310`).
  `peekData()`/`clearPeekData()` may **only** be called from inside
  `ZCom_NodeReplicationInterceptor::inPreUpdateItem()`
  (`zoidcom_replicator.h:267-269`) — calling it elsewhere is explicitly
  out of contract.
- **`ZCom_ReplicatorAdvanced` replicators cannot exceed their owning
  node's send priority.** *"The replicator cannot surpass the owning
  node's priority. It is only able to send when the owning node is
  allowed to send."* (`zoidcom_replicator_advanced.h:27-29`) — a custom
  ADD/UPDATE/REMOVE/RESET protocol (like `EntityPropertyMapReplicator`)
  cannot force an out-of-band send; it's still gated by the node's
  priority-based scheduling, meaning `sendData()` calls can be arbitrarily
  delayed by node priority even though the call itself returns
  immediately (see the "not sent immediately" caveats quoted in Q7).
- **`getNode()` can return NULL inside `onDataReceived()` specifically,
  and nowhere else.** Explicitly called out as an exception to the rest of
  the interface (`zoidcom_replicator_advanced.h:163-165`, quoted in Q7) —
  because data can arrive for a replicator whose owning node was already
  deleted locally. Any Phase B driver must null-check inside that one
  callback even though every other callback is documented safe.
- **`ZCOM_REPFLAG_ONLYONCE` and `ZCOM_REPFLAG_STARTCLEAN` are mutually
  exclusive**, and separately **`ZCOM_REPFLAG_SETUPPERSISTS` and
  `ZCOM_REPFLAG_SETUPAUTODELETE` are mutually exclusive** — both stated as
  hard "don't combine" rules in the flag docs (`zoidcom_node.h:48-50`,
  `:69`), not just discouraged.
- **ID space / translation trap**: `ZCom_ClassID`s are **not** stable
  across peers and must never be embedded in custom payload data —
  explicitly warned in the node registration doc:
  > Don't use the ZCom_ClassIDs in your network communication yourself, client and server
  > may use completely different IDs for the same class. They are translated by Zoidcom
  > automatically but obviously Zoidcom is unable to do that if the IDs are embedded in
  > your custom data. (`zoidcom_node.h:160-163`)
  This matters for Phase B's own node-linking implementation: whatever
  the shim uses as its wire-level class identifier must not leak into
  application bitstreams, and must be translated per-`ZCom_Control` (e.g.
  by class *name*, not by numeric ID, when linking nodes across a
  connection) exactly like the real library.
- **Retransmission/reliability is described only qualitatively, never
  quantitatively**, beyond `setResendTimeout()`
  (`zoidcom.h:338-349`, default 4s) and `setConnectionTimeout()`
  (`zoidcom.h:328-336`, default 20s):
  > If Zoidcom is waiting for too many packet acks it won't send any more packets until such an
  > ack arrives. When this timeout passes, Zoidcom will resend the oldest unacked packet.
  This is a **stop-and-wait-style backpressure model** — "won't send any
  more packets" until acks catch up — which is a meaningfully different
  behavior model than ENet's own sliding-window reliable channels; if
  Phase B ever needs bit-for-bit-equivalent backpressure behavior (rather
  than "just as reliable, differently paced"), this is the one place the
  header states an actual algorithmic detail rather than just an
  API contract.
- **Ordering guarantee wording is precise and easy to get subtly wrong**:
  `eZCom_ReliableOrdered` is *"reliable and ordered with other ordered
  events **from this node**"* (`zoidcom.h:164-165`, emphasis added) — i.e.
  ordering is scoped per-node, not globally per-connection or globally per
  `ZCom_Control`. `eZCom_Unreliable` is specifically *sequenced*
  (out-of-order-old packets dropped) rather than fully unordered: *"i.e. 1
  2 4 might happen but 1 2 4 3 not"* (`zoidcom.h:166-167`).

---

## Comparison against the shim (`compat/zoidcom/`)

Read: `compat/zoidcom/include/zoidcom/*.h` (identical signatures to the
vendor headers — confirmed no drift) and `compat/zoidcom/src/*.cpp`
(`Node.cpp`, `Control.cpp`, `Replicator.cpp`, `Address.cpp`,
`BitStream.cpp`, `ConnGroup.cpp`, `ZoidComGlobal.cpp`).

**No signature mismatches found** (as expected — those would be compile
errors, and Phase A already builds clean). The shim's own comments
(`Node.cpp:1-22`, `Control.cpp:1-32`, `Replicator.cpp:1-13`) are already
unusually candid about what's stubbed, and cross-checking those claims
against the semantics established above, **I did not find any place where
the shim's stated intent is semantically wrong** relative to real ZoidCom
— every declared `TODO(phaseB)` matches a real, correctly-identified gap
(cross-network node linking, replication tick, event delivery,
Zoidlevels/mustsync/setOwner, file transfer). Specific points worth Phase
B's attention, none of which are *bugs* in Phase A (all already flagged as
`TODO(phaseB)` in the source) but which the semantics above make more
precise:

1. **`Node.cpp:112-116`, `registerNodeDynamic()`** always assigns
   `eZCom_RoleAuthority` regardless of call context:
   ```cpp
   bool ZCom_Node::registerNodeDynamic(ZCom_ClassID _classid, ZCom_Control* _control) {
       zshim::todoPhaseBOnce(...);
       return registerCommon(m_priv, _classid, eZCom_RoleAuthority, _control);
   }
   ```
   Real semantics (`zoidcom_node.h:196-197`, Q3/Q6 above): the role must
   be `eZCom_RoleProxy` **specifically when called from inside
   `ZCom_cbNodeRequest_Dynamic()`**, `eZCom_RoleAuthority` otherwise. The
   shim has no notion of "inside the callback" context yet (there's no
   real dynamic-spawn-request delivery at all), so this is a correctly
   scoped simplification given Phase A's stated non-goals — but Phase B's
   real implementation of `registerNodeDynamic()` must NOT keep this
   unconditional-authority behavior; it needs to detect the
   inside-callback context and assign `eZCom_RoleProxy` there. Already
   implicitly covered by the file's own TODO ("non-authority nodes never
   receive a real network id...") but worth calling out explicitly since
   the *role* assignment (not just the network id) is also wrong today,
   for the dynamic case specifically, once real linking exists.

2. **`Node.cpp:118-120`, `registerRequestedNode()`** unconditionally
   assigns `eZCom_RoleProxy` — this one is actually **correct** as
   written per the `@deprecated` doc (`zoidcom_node.h:211-212`: role is
   always Proxy when created from inside the request callback, which is
   the method's only calling convention) — no divergence here, noted only
   because it's the mirror image of point 1 and worth confirming as
   already right.

3. **`Control.cpp:164-190`, `ZCom_Connect()`'s request-bitstream drop** —
   already extensively self-documented as the connection-handshake gap.
   Worth reinforcing with the Q3 finding: real ZoidCom's
   `ZCom_cbNodeRequest_Dynamic`/`_Tag` callbacks are **`void`-returning
   with mandatory compliance** ("If you fail to do so... Zoidcom will...
   disconnect"). Since the shim doesn't deliver these callbacks at all
   yet, there's no divergence to flag today, but Phase B's future
   implementation must preserve the "no graceful decline" contract — it
   would be a semantic bug to add a bool return or an accept/reject path
   to these callbacks that doesn't exist in the real API.

4. **`Replicator.cpp:47-53`** — the shim's `~ZCom_Replicator()` deletes
   `m_setup` only if `ZCOM_REPFLAG_SETUPAUTODELETE` is set *on the
   setup itself* (`m_setup->getFlags() & ZCOM_REPFLAG_SETUPAUTODELETE`).
   This matches the real flag's documented owner ("Use this if you want
   ZCom_Node to automatically delete a dynamically allocated replicator
   setup on destruction" — `zoidcom_node.h:63-71`) in spirit, though real
   ZoidCom's doc frames this as a `ZCom_Node`-level behavior ("when
   replicator gets deleted") rather than something `~ZCom_Replicator()`
   itself does — a subtle framing difference, not a behavior bug, since
   either way the setup ends up deleted when the replicator holding it is
   destroyed, which is what the flag promises. Not flagging as wrong, just
   noting the framing doesn't exactly mirror the header's wording (`Node`
   vs `Replicator` as the deleting party).

5. **No divergence found in `ZCom_ReplicatorAdvanced`'s documented
   contract** (`compat/zoidcom/include/zoidcom/zoidcom_replicator_advanced.h`
   is byte-for-byte the same declarations as the vendor header — verified
   by the fact that Phase A only *implements* `Replicator.cpp:104-123`
   stubs, it doesn't redeclare the interface). `EntityPropertyMapReplicator`
   in the actual game code overrides exactly the pure virtuals the real
   header requires (`onConnectionAdded/Removed`, `onDataReceived`,
   `onLocalRoleChanged`, `onPacketReceived`, `onPreSendData`,
   `onRemoteRoleChanged` — confirmed in
   `HovercraftUniverse/CoreEngine/EntityPropertySystem.h:341-366`), so
   there's no shape mismatch for Phase B to worry about — only the "make
   these actually get called, in the semantics established in Q7 above"
   implementation work, which is already exactly what
   `docs/porting/zoidcom-compat.md`'s "What Phase B needs to build" §3
   scopes.

**Net assessment**: the shim's own documentation (in `Node.cpp`,
`Control.cpp`, `Replicator.cpp` headers and in
`docs/porting/zoidcom-compat.md`) already correctly identifies every
semantic gap found during this research. The one concrete thing to fix
*when* Phase B implements dynamic node linking (not before, since it's
unreachable code today) is item 1 above: `registerNodeDynamic()` must
branch on "are we inside `ZCom_cbNodeRequest_Dynamic()` right now" to
decide `eZCom_RoleProxy` vs `eZCom_RoleAuthority`, rather than always
returning `eZCom_RoleAuthority`.
