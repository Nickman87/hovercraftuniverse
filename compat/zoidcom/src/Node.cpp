// ZoidCom-compatible shim: ZCom_Node.
//
// Real (Phase A): node identity bookkeeping -- class id, control, replicator
// list ownership (with autodelete), per-connection and global user data.
//
// Real (Phase B, steps 1-2 -- see docs/porting/phase-b-replication.md and
// docs/porting/zoidcom-original-semantics.md):
//   - Cross-network node linking. registerNodeDynamic()/registerNodeUnique()
//     now genuinely link an authority node to its proxy/owner counterparts
//     on connected ZCom_Controls, via NODE_CREATE / NODE_LINK_UNIQUE wire
//     messages built and sent by ZCom_Control (compat/zoidcom/src/Control.cpp
//     owns the per-control registry and network-id space -- see that file's
//     header for why this must never be static/file-global).
//   - registerNodeDynamic() assigns eZCom_RoleProxy when called from inside
//     ZCom_cbNodeRequest_Dynamic() and eZCom_RoleAuthority otherwise (fixed
//     a Phase A bug where it always assigned Authority -- see
//     zoidcom-original-semantics.md's "Comparison against the shim", item 1).
//   - setAnnounceData() carries real payload now: for a registered authority
//     node it also queues (not sends immediately -- see below) the
//     announcement to every currently-relevant connection.
//   - setOwner() is a real per-connection permission gate: it updates this
//     node's bookkeeping of the connection's role and, once that
//     connection's node has already been announced, sends a NODE_OWNER
//     message; if the announcement is still pending (queued but not yet
//     flushed), the promoted role is simply embedded in that first
//     announcement instead -- this matters because the game routinely calls
//     setOwner() synchronously, right after registration, in the same
//     constructor (e.g. PlayerSettings, Lobby::onConnect), before any
//     ZCom_processOutput() tick has run.
//     Registration-order-independence follow-up: setOwner() also now
//     unconditionally records the requested role in a per-connection
//     pending_owner_intent map on the node, regardless of this node's
//     current role or whether the connection is linked yet.
//     ZCom_shimQueueAnnounce() consults that map whenever it creates a
//     LinkedConn entry, so a setOwner() call made *before* registerNodeDynamic()
//     / networkRegister() (e.g. ServerLoader.cpp's Hovercraft, which sets
//     ownership before calling networkRegister()) is not silently dropped
//     just because this node wasn't the authority yet at call time.
//   - The node event queue (checkEventWaiting()/getNextEvent()) is a real
//     per-node FIFO now. sendEvent()/sendEventDirect()/sendEventToGroup()
//     deliver via NODE_EVENT, filtered by replication rule for sendEvent().
//   - eZCom_EventInit is raised on the authority side when a proxy/owner
//     newly links to it (gated by setEventNotification()'s _oninit flag,
//     which is now real and correctly handles the "linked before
//     setEventNotification(true,...) was called" ordering the game relies
//     on). eZCom_EventRemoved is raised on connection loss (proxy/owner side:
//     mandatory, cannot be gated -- matches real semantics) and on receiving
//     a NODE_REMOVE.
//
// Real (Phase B, steps 3-4 -- see docs/porting/phase-b-replication.md and
// docs/porting/zoidcom-original-semantics.md):
//   - addReplicationInt/Bool/Float/String/StringW now store their _bits/
//     _mantissa_bits/_maxlen/_flags/_rules/_mindelay/_maxdelay parameters
//     (previously discarded via (void) casts) and are driven for real by
//     ZCom_shimTickReplication(), called once per node from
//     ZCom_Control::ZCom_processReplicators(). Dirty detection is a shadow
//     copy per item; MOSTRECENT sends unreliable and additionally resends
//     on maxdelay elapsed (a dropped final update must not leave a
//     permanently stale value); non-MOSTRECENT sends reliable, once, on
//     change only. mindelay throttles resends (but never suppresses a
//     maxdelay-forced resend). This tracking is node-scoped, not
//     per-connection -- see the file's "batch" comment below for how a
//     newly-linked connection is still brought up to date despite that.
//   - addReplicator()-registered ZCom_ReplicatorBasic subclasses
//     (OgreVector3_Replicator, OgreQuaternion_Replicator, String_Replicator)
//     are driven the same tick: checkState() (which does its own dirty
//     comparison internally, see e.g. OgreVector3_Replicator::checkState())
//     gates packData()/unpackData(), honouring the *setup's* flags/rules/
//     mindelay/maxdelay exactly like the primitive paths. Per
//     phase-b-replication.md §3.4 this is not a refinement -- it is the
//     only mechanism that moves mPosition/mOrientation/mVelocity, since the
//     game never uses primitive replication for those fields.
//   - addReplicator()-registered ZCom_ReplicatorAdvanced subclasses (the
//     game has exactly one concrete instance: EntityPropertyMapReplicator)
//     get Process() driven once per tick when ZCOM_REPLICATOR_CALLPROCESS
//     is set, and their sendData()/sendDataDirect() calls (see
//     Replicator.cpp) are delivered immediately via
//     ZCom_shimSendAdvancedData() rather than deferred to onPreSendData().
//     That is a deliberate deviation from real ZoidCom's documented timing
//     ("wait for onPreSendData() ... to generate data which is sent
//     immediately") -- but EntityPropertyMapReplicator's own onPreSendData()
//     override is an empty {} (confirmed in EntityPropertySystem.h), so the
//     game never relies on that hook, and immediate delivery is strictly
//     simpler with no observable behavioral difference. onConnectionAdded/
//     onConnectionRemoved/onLocalRoleChanged/onRemoteRoleChanged are wired
//     to the existing link/owner-change bookkeeping below.
//     onPacketReceived() is deliberately NOT driven: the header states its
//     only real consumer is ZCom_MovementReplicator (which this game does
//     not have), and EntityPropertyMapReplicator's own override is an empty
//     {} -- driving it would add per-packet bookkeeping with zero
//     observable effect. getLastUpdateTime() stays a todoPhaseBOnce stub
//     for the same reason: unused by the one concrete Advanced replicator
//     in this codebase (mindelay/maxdelay are explicitly not enforced for
//     Advanced replicators, so it has no self-timing need).
//   - dependsOn() now records the dependency edge (for API completeness/
//     introspection) but does not reorder anything: our tick already
//     processes every registered node's full item set per call, with no
//     priority/partial-scheduling model for dependsOn() to influence.
//
// Batching: ZCom_shimTickReplication() computes the dirty set once per
// node per tick (matching the real library's documented "checkState() is
// called once per ZCom_processOutput(), not per connection" -- see
// zoidcom-original-semantics.md §8) and then fans the *same* precomputed
// payload out to every relevant connection, at most one reliable and one
// unreliable envelope per connection (see Control.cpp's kMsgNodeReplBatch).
// Because dirty tracking is node-scoped rather than per-connection, a
// MOSTRECENT item's maxdelay-driven resend timer is also node-scoped: with
// more than one linked connection, all of them get resent to on the same
// schedule rather than each independently. This is a documented divergence
// from a hypothetically bit-exact per-connection implementation, but it is
// harmless -- unlike the initial-sync gap below, nothing depends on the
// *timing* of a resend, only on the value eventually arriving.
//
// Initial full sync (was: a real deadlock, not just a divergence). Node-
// scoped dirty tracking used to mean a connection that linked to an already-
// settled node -- one whose fields stopped changing before this connection
// existed -- would never receive that value at all: it isn't dirty, so it's
// never in the per-tick set, for any connection, ever. The assumption this
// shim originally relied on to wave that away -- "a brand-new connection
// gets the current value via its eZCom_EventInit snapshot instead" -- does
// not hold in general: eZCom_EventInit is only raised when the *authority*
// node has called setEventNotification(true, ...), and not every node does
// (RaceState never does; see RaceState.cpp and docs/porting/
// zoidcom-original-semantics.md). For such a node, real ZoidCom must still
// have delivered its current state to a newly-linked connection some other
// way -- that is the contract this shim was breaking, and it deadlocked
// race startup (RaceState::mNumberPlayers never reaching a freshly-linked
// client; see docs/porting/phase-b-replication.md).
//
// Fixed via LinkedConn::needs_full_sync (see ZCom_shimQueueAnnounce()): set
// whenever a connection newly becomes relevant to an authority node, cleared
// the first time ZCom_shimTickReplication() actually sends that connection
// a full snapshot of every eligible item's *current* value (built by
// ZCom_shimBuildFullReplSnapshot() below, independent of node-scoped dirty
// state), sent reliably even for a ZCOM_REPFLAG_MOSTRECENT item (a dropped
// initial state would recreate exactly this bug). The flag is only acted on
// once the connection's LinkedConn::announced is also true, so the full
// sync can never be sent ahead of the NODE_CREATE/NODE_LINK_UNIQUE that
// makes the node exist remotely -- both ride the same reliable-ordered
// channel, so send order is guaranteed. Already-synced connections continue
// to receive only the per-tick dirty set, unaffected.
//
// Remaining TODO(phaseB) stubs, all logged once via zshim::todoPhaseBOnce():
//   - addInterpolationInt/addInterpolationFloat (unused by any current call
//     site -- the one call site is commented out, see Entity.cpp).
//   - registerNodeByTag/Zoidlevels, mustsync/authority migration (beyond
//     setOwner), file transfer.
// See docs/porting/zoidcom-compat.md for the full design rationale.
#include "zoidcom_shim_internal.h"
#include <vector>
#include <map>
#include <set>
#include <deque>
#include <string>
#include <cstring>
#include <cwchar>

namespace {

struct ReplicationItem {
    enum Kind { Int, Bool, Float, String, StringW, InterpInt, InterpFloat, CustomReplicator } kind;
    void* ptr;
    ZCom_Replicator* replicator;
    bool autodelete;

    // Step 3 metadata -- previously discarded via (void) casts in
    // addReplicationInt/Bool/Float/String/StringW, now used to drive the
    // real replication tick (ZCom_Node::ZCom_shimTickReplication()). For a
    // CustomReplicator item, flags/rules/mindelay/maxdelay are refreshed
    // every tick straight from the replicator's own ZCom_ReplicatorSetup
    // instead (see the CustomReplicator case in ZCom_shimTickReplication()).
    zU8 bits;        // Int: wire width. Float: mantissa bits.
    bool sign;       // Int only: signed vs unsigned wire encoding.
    zU16 maxlen;     // String/StringW only: destination buffer capacity.
    zU8 flags;       // ZCOM_REPFLAG_*
    zU8 rules;       // ZCOM_REPRULE_*
    zS16 mindelay;
    zS16 maxdelay;

    // Shadow/dirty-tracking state. Node-scoped, not per-connection -- see
    // the file header's "Batching" note.
    bool has_shadow;
    zS32 shadow_int;
    bool shadow_bool;
    zFloat shadow_float;
    std::string shadow_str;
    std::wstring shadow_wstr;
    zU32 last_send_time;
};

// Authority-side bookkeeping for one connection this node is relevant to.
struct LinkedConn {
    eZCom_NodeRole role;   // eZCom_RoleProxy or eZCom_RoleOwner, from this node's perspective
    bool announced;        // NODE_CREATE/NODE_LINK_UNIQUE already sent for this connection
    // True until this connection has received a full snapshot of every
    // eligible replication item's *current* value -- see
    // ZCom_shimTickReplication()'s "full sync" pass. Set whenever this entry
    // is created (i.e. whenever the connection newly becomes relevant to
    // this node); cleared once the snapshot has actually been sent, which
    // only happens after `announced` is true (so it can never race ahead of
    // the NODE_CREATE/NODE_LINK_UNIQUE that makes the node exist remotely --
    // see the file header's "Initial full sync" note).
    bool needs_full_sync;
};

struct PendingEvent {
    eZCom_Event type;
    eZCom_NodeRole remote_role;
    ZCom_ConnID conn_id;
    ZCom_BitStream* data;
    zU32 estimated_time_sent;
};

} // namespace

class ZCom_Node_Private {
public:
    ZCom_ClassID class_id;
    ZCom_Control* control;
    eZCom_NodeRole role;
    ZCom_NodeID network_id;
    bool registered;
    bool is_private;
    bool is_unique_kind;
    zU16 update_priority;
    zFloat default_relevance;

    std::vector<ReplicationItem> replication_items;
    ZCom_NodeReplicationInterceptor* replication_interceptor;
    ZCom_NodeEventInterceptor* event_interceptor;

    // dependsOn() bookkeeping -- see file header, recorded but not acted on.
    std::set<ZCom_Node*> dependencies;

    std::map<ZCom_ConnID, void*> conn_user_data;
    void* global_user_data;

    // --- Phase B linking/event state --------------------------------------

    // Authority-side: every connection this node is (or is about to become)
    // relevant to.
    std::map<ZCom_ConnID, LinkedConn> linked_conns;
    // Authority-side: connections queued for the deferred announcement flush
    // (see ZCom_shimFlushPendingAnnouncements()).
    std::vector<ZCom_ConnID> pending_announce;

    // setOwner() intent recorded per connection, independent of this node's
    // current role/registration/link state -- see setOwner()'s comment and
    // ZCom_shimQueueAnnounce() below. The game routinely calls setOwner()
    // before this node is even registered (e.g. ServerLoader.cpp sets the
    // controlling player's ownership on a freshly-created Hovercraft's node
    // before networkRegister() has run at all), so the role/link-based gate
    // that used to live in setOwner() must not be the only place this is
    // remembered.
    std::map<ZCom_ConnID, bool> pending_owner_intent;

    // Proxy/owner-side: the connection this node's authority lives on.
    ZCom_ConnID authority_conn;

    bool notify_oninit;
    bool notify_onremove;
    // Connections that linked before setEventNotification(true, ...) was
    // called -- flushed into real eZCom_EventInit events once it is.
    std::vector<ZCom_ConnID> pending_init_before_notify;

    ZCom_BitStream* announce_data;

    std::deque<PendingEvent> event_queue;
    // getNextEvent() hands out a raw pointer the caller never frees (matches
    // observed game code, which never deletes it -- see
    // docs/porting/zoidcom-original-semantics.md). We free the previously
    // delivered one on the next call/on destruction instead.
    ZCom_BitStream* last_delivered_event;

    ZCom_Node_Private()
        : class_id(ZCom_Invalid_ID), control(NULL), role(eZCom_RoleUndefined),
          network_id(0), registered(false), is_private(false), is_unique_kind(false),
          update_priority(0), default_relevance(1.0f),
          replication_interceptor(NULL), event_interceptor(NULL),
          global_user_data(NULL),
          authority_conn(ZCom_Invalid_ID),
          notify_oninit(false), notify_onremove(false),
          announce_data(NULL), last_delivered_event(NULL) {}

    ~ZCom_Node_Private() {
        for (size_t i = 0; i < replication_items.size(); i++) {
            if (replication_items[i].kind == ReplicationItem::CustomReplicator &&
                replication_items[i].autodelete && replication_items[i].replicator) {
                delete replication_items[i].replicator;
            }
        }
        if (last_delivered_event) delete last_delivered_event;
        for (size_t i = 0; i < event_queue.size(); i++) delete event_queue[i].data;
        if (announce_data) delete announce_data;
    }
};

namespace {

// Calls fn(ZCom_ReplicatorAdvanced*) for every ZCOM_REPLICATOR_ADVANCED item
// registered on this node -- used to drive onConnectionAdded/
// onConnectionRemoved/onLocalRoleChanged/onRemoteRoleChanged "in the
// documented order" (i.e. as their trigger condition occurs) from the
// link/owner-change bookkeeping below. See the file header's step 3-4 note.
template <typename Fn>
void forEachAdvancedReplicator(ZCom_Node_Private* p, Fn fn) {
    for (size_t i = 0; i < p->replication_items.size(); i++) {
        ReplicationItem& item = p->replication_items[i];
        if (item.kind == ReplicationItem::CustomReplicator && item.replicator &&
            (item.replicator->getFlags() & ZCOM_REPLICATOR_ADVANCED)) {
            fn(static_cast<ZCom_ReplicatorAdvanced*>(item.replicator));
        }
    }
}

} // namespace

// --- Phase B steps 3-4: replication-tick item representation, and the
// full-sync builder/sender (defined here, ahead of ZCom_shimQueueAnnounce()/
// ZCom_shimFlushPendingAnnouncements() below, because the full sync is sent
// from inside the flush -- see that function's comment for why it cannot
// wait for ZCom_shimTickReplication() to run again). ---------------------
namespace {

// One replication item ready to fan out to a connection: either a dirty item
// computed this tick (see ZCom_shimTickReplication()) or one entry of a full
// initial-state snapshot (see ZCom_shimBuildFullReplSnapshot()). Owns
// `payload` until the caller deletes it after sending.
struct PendingReplItem {
    zU16 index;
    zU8 rules;
    bool unreliable; // ZCOM_REPFLAG_MOSTRECENT -- see file header
    bool intercept;  // ZCOM_REPFLAG_INTERCEPT -- gates outPreUpdateItem() below
    ZCom_Replicator* replicator; // non-NULL only for a CustomReplicator item;
                                 // outPreUpdateItem() needs this to hand to
                                 // the interceptor. Primitive items have no
                                 // such object -- see the file header's
                                 // ZCOM_REPFLAG_INTERCEPT-on-primitive note.
    ZCom_BitStream* payload;
};

// Builds a snapshot of every eligible replication item's CURRENT value,
// regardless of node-scoped dirty state -- used once per connection that
// still needs its initial full sync (see LinkedConn::needs_full_sync).
// Read-only with respect to any item's shadow copy, last_send_time, or (for
// a ZCom_ReplicatorBasic) its checkState()-tracked comparison value: it must
// not suppress or duplicate a legitimate dirty detection on this or a later
// tick. packData() alone is used for a Basic custom replicator instead of
// checkState()+packData() -- packData() already emits the *full* current
// value with no dependency on checkState() having just run (verified
// against OgreVector3_Replicator::packData(), which reads straight from its
// data pointer). Interpolated items and ZCom_ReplicatorAdvanced instances
// are skipped, matching the dirty-tick switch in ZCom_shimTickReplication()
// (neither participates in primitive/Basic replication).
std::vector<PendingReplItem> ZCom_shimBuildFullReplSnapshot(ZCom_Node_Private* p) {
    std::vector<PendingReplItem> snap;
    snap.reserve(p->replication_items.size());

    for (size_t i = 0; i < p->replication_items.size(); i++) {
        ReplicationItem& item = p->replication_items[i];
        ZCom_BitStream* payload = NULL;
        ZCom_Replicator* rep_ptr = NULL;
        zU8 rules = item.rules;
        zU8 flags = item.flags;

        switch (item.kind) {
        case ReplicationItem::Int: {
            zS32 cur = *(zS32*) item.ptr;
            payload = new ZCom_BitStream();
            if (item.sign) payload->addSignedInt(cur, item.bits);
            else payload->addInt((zU32) cur, item.bits);
            break;
        }
        case ReplicationItem::Bool: {
            bool cur = *(bool*) item.ptr;
            payload = new ZCom_BitStream();
            payload->addBool(cur);
            break;
        }
        case ReplicationItem::Float: {
            zFloat cur = *(zFloat*) item.ptr;
            payload = new ZCom_BitStream();
            payload->addFloat(cur, item.bits);
            break;
        }
        case ReplicationItem::String: {
            const char* cur = (const char*) item.ptr;
            payload = new ZCom_BitStream();
            payload->addString(cur);
            break;
        }
        case ReplicationItem::StringW: {
            const wchar_t* cur = (const wchar_t*) item.ptr;
            payload = new ZCom_BitStream();
            payload->addStringW(cur);
            break;
        }
        case ReplicationItem::CustomReplicator: {
            if (!item.replicator || !(item.replicator->getFlags() & ZCOM_REPLICATOR_BASIC)) break;
            ZCom_ReplicatorBasic* rb = static_cast<ZCom_ReplicatorBasic*>(item.replicator);
            ZCom_ReplicatorSetup* setup = rb->getSetup();
            // Refresh from the setup, same as the dirty-tick pass -- the
            // setup is the authoritative source for a custom replicator.
            rules = setup ? setup->getRules() : item.rules;
            flags = setup ? setup->getFlags() : 0;
            payload = new ZCom_BitStream();
            rb->packData(payload);
            rep_ptr = item.replicator;
            break;
        }
        default: break;
        }

        if (!payload) continue;

        PendingReplItem entry;
        entry.index = (zU16) i;
        entry.rules = rules;
        entry.unreliable = false; // full sync always goes reliably -- see caller
        entry.intercept = (flags & ZCOM_REPFLAG_INTERCEPT) != 0;
        entry.replicator = rep_ptr;
        entry.payload = payload;
        snap.push_back(entry);
    }
    return snap;
}

// Sends `source` (either this tick's dirty set or a full-sync snapshot) to
// one connection, honouring direction rules (AUTH_2_PROXY/AUTH_2_OWNER/
// OWNER_2_AUTH per item), the ZCOM_REPFLAG_INTERCEPT/outPreUpdateItem() gate,
// and outPreUpdate()/outPostUpdate() -- shared by ZCom_shimTickReplication()
// (force_reliable=false, so a MOSTRECENT item still goes unreliable) and
// ZCom_shimFlushPendingAnnouncements()'s full sync (force_reliable=true: a
// dropped initial state would recreate the very bug this exists to fix).
// Does not delete any `source[i].payload` -- the caller owns those.
void ZCom_shimSendReplItemsToConn(ZCom_Node* node, ZCom_Node_Private* p, ZCom_ConnID conn,
                                   eZCom_NodeRole remote_role,
                                   const std::vector<PendingReplItem>& source,
                                   bool force_reliable) {
    if (source.empty()) return;

    // outPreUpdate(): "Should return 'false' to prevent sending updates now,
    // 'true' otherwise" -- a per-call, per-connection veto, not a permanent
    // one (zoidcom_node_interceptors.h).
    if (p->replication_interceptor && !p->replication_interceptor->outPreUpdate(node, conn, remote_role)) {
        return;
    }

    ZCom_BitStream reliable_env, unreliable_env;
    zU16 reliable_count = 0, unreliable_count = 0;
    zU32 rep_bits = 0;

    for (size_t i = 0; i < source.size(); i++) {
        const PendingReplItem& item = source[i];
        bool applies;
        if (p->role == eZCom_RoleAuthority) {
            bool to_proxy = (item.rules & ZCOM_REPRULE_AUTH_2_PROXY) != 0 && remote_role == eZCom_RoleProxy;
            bool to_owner = (item.rules & ZCOM_REPRULE_AUTH_2_OWNER) != 0 && remote_role == eZCom_RoleOwner;
            applies = to_proxy || to_owner;
        } else {
            applies = (item.rules & ZCOM_REPRULE_OWNER_2_AUTH) != 0;
        }
        if (!applies) continue;

        // outPreUpdateItem(): "This callback only gets called if the
        // replication item really is about to be updated now" -- matches
        // `source` already being exactly the set of items being sent to
        // this connection (dirty set, or the full snapshot for a
        // newly-linked connection). Gated by ZCOM_REPFLAG_INTERCEPT per the
        // flag's own doc. No current game code sets that flag, so the
        // primitive-item (no ZCom_Replicator to pass) branch below is
        // unreachable today but handled per the file header.
        if (item.intercept && p->replication_interceptor) {
            if (item.replicator) {
                if (!p->replication_interceptor->outPreUpdateItem(node, conn, remote_role, item.replicator)) {
                    continue;
                }
            } else {
                zshim::todoPhaseBOnce("ZCom_Node::ZCom_shimSendReplItemsToConn(intercept-primitive)",
                    "ZCOM_REPFLAG_INTERCEPT set on a primitive replication item; outPreUpdateItem() "
                    "can't be called without a ZCom_Replicator instance to pass, sending the update "
                    "unfiltered (unused by any current game code)");
            }
        }

        // A full sync (force_reliable) must go reliably even for a
        // ZCOM_REPFLAG_MOSTRECENT item -- a dropped initial state is
        // exactly the failure this mechanism exists to fix (see file
        // header).
        bool unreliable_ok = item.unreliable && !force_reliable;
        ZCom_BitStream& env = unreliable_ok ? unreliable_env : reliable_env;
        zU16& count = unreliable_ok ? unreliable_count : reliable_count;
        env.addInt(item.index, 16);
        zU32 bits = item.payload->getBitCount();
        env.addInt(bits, 16);
        env.addBitStream(item.payload, true);
        count++;
        rep_bits += 32 + bits; // index + length fields, plus the payload itself
    }

    if (reliable_count > 0) {
        ZCom_BitStream out;
        out.addInt(reliable_count, 16);
        out.addBitStream(&reliable_env, true);
        p->control->ZCom_shimSendNodeReplBatch(conn, p->network_id, true, out);
    }
    if (unreliable_count > 0) {
        ZCom_BitStream out;
        out.addInt(unreliable_count, 16);
        out.addBitStream(&unreliable_env, true);
        p->control->ZCom_shimSendNodeReplBatch(conn, p->network_id, false, out);
    }

    // outPostUpdate(): only fires when something was actually included in
    // this connection's update this call -- matches "The node has included
    // it's update into the current packet."
    if ((reliable_count > 0 || unreliable_count > 0) && p->replication_interceptor) {
        p->replication_interceptor->outPostUpdate(node, conn, remote_role, rep_bits, 0, 0);
    }
}

} // namespace

ZCom_Node::ZCom_Node(void) : m_priv(new ZCom_Node_Private()) {}

ZCom_Node::~ZCom_Node(void) {
    disconnectAll();
    delete m_priv;
}

static bool registerCommon(ZCom_Node_Private* p, ZCom_ClassID _classid, eZCom_NodeRole _role, ZCom_Control* _control) {
    if (!_control) return false;
    p->class_id = _classid;
    p->control = _control;
    p->role = _role;
    p->registered = true;
    return true;
}

void ZCom_Node::pushEvent(eZCom_Event _type, eZCom_NodeRole _remote_role, ZCom_ConnID _conn_id,
                          ZCom_BitStream* _data, zU32 _estimated_time_sent) {
    PendingEvent e;
    e.type = _type;
    e.remote_role = _remote_role;
    e.conn_id = _conn_id;
    e.data = _data;
    e.estimated_time_sent = _estimated_time_sent;
    m_priv->event_queue.push_back(e);
}

void ZCom_Node::noteConnectionLinkedEventInit(ZCom_ConnID _conn) {
    // See file header: notify_oninit may not be set yet at the moment this
    // connection actually became linked (the game commonly registers, then
    // calls setEventNotification() afterwards, in the same constructor) --
    // in that case the connection is remembered and flushed once
    // setEventNotification(true, ...) is called. remote_role is always
    // Proxy here: any later Owner promotion is a separate
    // ZCom_shimSetOwnerRole()/NODE_OWNER event, not retroactively folded
    // into an Init event that's already been queued (eZCom_EventInit
    // carries no payload either way -- see zoidcom-original-semantics.md §1).
    if (m_priv->notify_oninit) {
        pushEvent(eZCom_EventInit, eZCom_RoleProxy, _conn, NULL, zshim::currentTimeMillis());
    } else {
        m_priv->pending_init_before_notify.push_back(_conn);
    }
}

bool ZCom_Node::registerNodeUnique(ZCom_ClassID _classid, eZCom_NodeRole _role, ZCom_Control* _control) {
    if (!registerCommon(m_priv, _classid, _role, _control)) return false;
    m_priv->is_unique_kind = true;
    _control->ZCom_shimBindUniqueClass(_classid, this);
    if (_role == eZCom_RoleAuthority) {
        ZCom_NodeID netid = _control->ZCom_shimAllocNetworkId();
        ZCom_shimBindSelf(netid, ZCom_Invalid_ID);
        _control->ZCom_shimBindNetId(netid, this);
        // Link any connections that already exist (registration normally
        // happens before any connection does, e.g. HUServerCore's Lobby --
        // but this keeps the linking correct regardless of order).
        std::vector<ZCom_ConnID> conns = _control->ZCom_shimAllConnections();
        for (size_t i = 0; i < conns.size(); i++) ZCom_shimQueueAnnounce(conns[i]);
    } else {
        ZCom_shimBindSelf(0, ZCom_Invalid_ID);
    }
    return true;
}

bool ZCom_Node::registerNodeByTag(ZCom_ClassID _classid, zU32 _tag, eZCom_NodeRole _role, ZCom_Control* _control) {
    (void) _tag;
    zshim::todoPhaseBOnce("ZCom_Node::registerNodeByTag",
        "tag-based node linking across connections is not implemented (unused by current game code)");
    return registerCommon(m_priv, _classid, _role, _control);
}

bool ZCom_Node::registerNodeDynamic(ZCom_ClassID _classid, ZCom_Control* _control) {
    if (!_control) return false;
    // Real semantics: eZCom_RoleProxy if called from inside
    // ZCom_cbNodeRequest_Dynamic(), eZCom_RoleAuthority otherwise -- see
    // zoidcom-original-semantics.md §3 and the Phase A bug this fixes
    // (previously this unconditionally assigned eZCom_RoleAuthority).
    // ZCom_shimRegisterDynamicNode() determines which, and binds this
    // node's network id (and, for the proxy case, its authority connection)
    // into the control's registry.
    eZCom_NodeRole role = _control->ZCom_shimRegisterDynamicNode(this);
    return registerCommon(m_priv, _classid, role, _control);
}

bool ZCom_Node::registerRequestedNode(ZCom_ClassID _classid, ZCom_Control* _control) {
    // Deprecated in the real API in favor of registerNodeDynamic(), and
    // unused by any current game code (confirmed by grep) -- kept as a
    // simple, unlinked Proxy registration, matching its documented contract.
    return registerCommon(m_priv, _classid, eZCom_RoleProxy, _control);
}

bool ZCom_Node::unregisterNode() {
    disconnectAll();
    m_priv->registered = false;
    return true;
}

void ZCom_Node::disconnectAll() {
    if (!m_priv->control) return;

    if (m_priv->role == eZCom_RoleAuthority && m_priv->registered) {
        for (std::map<ZCom_ConnID, LinkedConn>::iterator it = m_priv->linked_conns.begin();
             it != m_priv->linked_conns.end(); ++it) {
            if (it->second.announced) {
                if (m_priv->replication_interceptor) {
                    m_priv->replication_interceptor->outPreDereplicateNode(this, it->first, it->second.role);
                }
                m_priv->control->ZCom_shimSendNodeRemove(it->first, m_priv->network_id);
            }
        }
    }
    m_priv->linked_conns.clear();
    m_priv->pending_announce.clear();
    m_priv->control->ZCom_shimForgetPendingFlush(this);

    if (m_priv->network_id != 0) {
        m_priv->control->ZCom_shimUnbindNetId(m_priv->network_id);
    }
    if (m_priv->is_unique_kind) {
        m_priv->control->ZCom_shimUnbindUniqueClass(m_priv->class_id, this);
    }
}

void ZCom_Node::setUpdatePriority(zU16 _prio) { m_priv->update_priority = _prio; }
void ZCom_Node::setDefaultRelevance(zFloat _default_relevance) { m_priv->default_relevance = _default_relevance; }

void ZCom_Node::setConnectionSpecificRelevance(ZCom_ConnID _conn, zFloat _rel) {
    (void) _conn; (void) _rel;
    zshim::todoPhaseBOnce("ZCom_Node::setConnectionSpecificRelevance", "per-connection relevance is not tracked");
}

zU32 ZCom_Node::getRelevantConnectionCount() const { return (zU32) m_priv->linked_conns.size(); }

zS32 ZCom_Node::getRelevantConnections(ZCom_ConnID* _conns, zU32 _max, zU32* _count) const {
    zU32 n = 0;
    for (std::map<ZCom_ConnID, LinkedConn>::const_iterator it = m_priv->linked_conns.begin();
         it != m_priv->linked_conns.end() && n < _max; ++it) {
        if (_conns) _conns[n] = it->first;
        n++;
    }
    if (_count) *_count = n;
    return 1;
}

void ZCom_Node::dependsOn(ZCom_Node* _othernode, eZCom_DependencyOpt _opt) {
    // Recorded, and (see ZCom_shimGetDependencies() / Control.cpp's
    // topoOrderPendingFlush()) now actually acted on: the deferred
    // announcement flush topologically orders same-cycle pending nodes so
    // _othernode is announced to a given connection before this node is --
    // see the file header and docs/porting/phase-b-replication.md.
    if (!_othernode) return;
    if (_opt == eZCom_AddDependency) {
        m_priv->dependencies.insert(_othernode);
    } else {
        m_priv->dependencies.erase(_othernode);
    }
}

std::vector<ZCom_Node*> ZCom_Node::ZCom_shimGetDependencies() const {
    return std::vector<ZCom_Node*>(m_priv->dependencies.begin(), m_priv->dependencies.end());
}

void ZCom_Node::applyForZoidLevel(zU8 _level) {
    (void) _level;
    zshim::todoPhaseBOnce("ZCom_Node::applyForZoidLevel", "Zoidlevels are not implemented");
}

void ZCom_Node::removeFromZoidLevel(zU8 _level) {
    (void) _level;
    zshim::todoPhaseBOnce("ZCom_Node::removeFromZoidLevel", "Zoidlevels are not implemented");
}

zU32 ZCom_Node::getZoidLevelCount() const { return 0; }
zU8 ZCom_Node::getZoidLevel(zU32 _index) const { (void) _index; return 0; }

void ZCom_Node::setMustSync(bool _enabled, zU16 _order) {
    (void) _enabled; (void) _order;
    zshim::todoPhaseBOnce("ZCom_Node::setMustSync", "Zoidlevel sync gating is not implemented");
}

void ZCom_Node::setSyncResult(ZCom_ConnID _conn, bool _success, ZCom_BitStream* _errormsg) {
    (void) _conn; (void) _success;
    delete _errormsg;
    zshim::todoPhaseBOnce("ZCom_Node::setSyncResult", "Zoidlevel sync gating is not implemented");
}

void ZCom_Node::setSyncResultAutoSuccess(bool _enabled) {
    (void) _enabled;
    zshim::todoPhaseBOnce("ZCom_Node::setSyncResultAutoSuccess", "Zoidlevel sync gating is not implemented");
}

void ZCom_Node::setOwner(ZCom_ConnID _id, bool _enabled) {
    // Real semantics: a pure permission gate on the authority node; it
    // "won't do anything special on its own" beyond changing the role the
    // remote connection's counterpart node observes -- see
    // zoidcom-original-semantics.md §6.
    //
    // Registration-order fix: record the intent for _id unconditionally,
    // regardless of whether this node is (yet) the authority or _id is (yet)
    // linked. The game calls setOwner() in three shapes we've observed:
    //   1. Authority + already-linked connection (RaceState.cpp, Lobby.cpp
    //      admin promotion) -- handled below exactly as before.
    //   2. Authority + not-yet-linked connection, i.e. still queued in
    //      pending_announce (PlayerSettings, Lobby::onConnect) -- also
    //      handled below via pending_announce/ZCom_shimFlushPendingAnnouncements.
    //   3. Not yet even registered as authority at all (ServerLoader.cpp
    //      calls setOwner() on a Hovercraft's node before networkRegister()
    //      has run; ChatServer.cpp calls it against a node registered with
    //      announce=false whose connections are queued lazily). Real ZoidCom
    //      shipped against this exact pattern in 2010, so it must have
    //      remembered the intent and applied it once knowable -- our old
    //      early-out here just silently dropped it (see
    //      docs/porting/phase-b-replication.md §6.1). ZCom_shimQueueAnnounce()
    //      consults this map whenever a LinkedConn entry for _id is created,
    //      whenever that ends up happening.
    m_priv->pending_owner_intent[_id] = _enabled;

    if (m_priv->role != eZCom_RoleAuthority) {
        // Not (yet) the authority -- nothing more to do until/unless this
        // node registers as authority and _id becomes linked (case 3 above).
        // If that never happens, this is a legitimate no-op, matching real
        // semantics ("only has an effect ... on the authority node").
        return;
    }
    std::map<ZCom_ConnID, LinkedConn>::iterator it = m_priv->linked_conns.find(_id);
    if (it == m_priv->linked_conns.end()) {
        // _id isn't (yet) relevant to this node -- legitimate no-op; the
        // intent above will be picked up if/when it becomes relevant via
        // ZCom_shimQueueAnnounce().
        return;
    }
    eZCom_NodeRole oldrole = it->second.role;
    eZCom_NodeRole newrole = _enabled ? eZCom_RoleOwner : eZCom_RoleProxy;
    it->second.role = newrole;
    if (oldrole != newrole) {
        forEachAdvancedReplicator(m_priv, [_id, oldrole, newrole](ZCom_ReplicatorAdvanced* rep) {
            rep->onRemoteRoleChanged(_id, oldrole, newrole);
        });
    }
    if (it->second.announced && m_priv->control) {
        m_priv->control->ZCom_shimSendNodeOwner(_id, m_priv->network_id, _enabled);
    }
    // else: still queued in pending_announce -- the promoted role will be
    // embedded directly in the deferred NODE_CREATE/NODE_LINK_UNIQUE send,
    // see ZCom_shimFlushPendingAnnouncements(). This matters because the
    // game routinely calls setOwner() synchronously right after
    // registration (PlayerSettings's constructor, Lobby::onConnect()),
    // before any ZCom_processOutput() tick has had a chance to flush.
}

void ZCom_Node::setPrivate(bool _enabled) { m_priv->is_private = _enabled; }

void ZCom_Node::setAnnounceData(ZCom_BitStream* _data) {
    // Real semantics: ownership transfers to us; the real library
    // re-evaluates this per target client via outPreReplicateNode() (not
    // implemented here -- no ZCom_NodeReplicationInterceptor driving exists
    // yet, so every currently/future-relevant connection gets the same
    // blob). See docs/porting/zoidcom-original-semantics.md §4.
    if (m_priv->announce_data) delete m_priv->announce_data;
    m_priv->announce_data = _data;

    if (m_priv->role == eZCom_RoleAuthority && m_priv->registered && m_priv->control && !m_priv->is_unique_kind) {
        std::vector<ZCom_ConnID> conns = m_priv->control->ZCom_shimAllConnections();
        for (size_t i = 0; i < conns.size(); i++) ZCom_shimQueueAnnounce(conns[i]);
    }
}

bool ZCom_Node::beginReplicationSetup(zU16 _replicators_max) {
    if (_replicators_max > 0) m_priv->replication_items.reserve(_replicators_max);
    return true;
}

void ZCom_Node::setInterceptID(ZCom_InterceptID _id) { (void) _id; }

void ZCom_Node::addReplicationInt(zS32* _ptr, zU8 _bits, bool _sign, zU8 _flags, zU8 _rules, zS16 _mindelay, zS16 _maxdelay) {
    ReplicationItem item = { ReplicationItem::Int, _ptr, NULL, false };
    item.bits = _bits; item.sign = _sign; item.flags = _flags; item.rules = _rules;
    item.mindelay = _mindelay; item.maxdelay = _maxdelay;
    m_priv->replication_items.push_back(item);
}

void ZCom_Node::addReplicationBool(bool* _ptr, zU8 _flags, zU8 _rules, zS16 _mindelay, zS16 _maxdelay) {
    ReplicationItem item = { ReplicationItem::Bool, _ptr, NULL, false };
    item.flags = _flags; item.rules = _rules; item.mindelay = _mindelay; item.maxdelay = _maxdelay;
    m_priv->replication_items.push_back(item);
}

void ZCom_Node::addReplicationFloat(zFloat* _ptr, zU8 _mantissa_bits, zU8 _flags, zU8 _rules, zS16 _mindelay, zS16 _maxdelay) {
    ReplicationItem item = { ReplicationItem::Float, _ptr, NULL, false };
    item.bits = _mantissa_bits; item.flags = _flags; item.rules = _rules;
    item.mindelay = _mindelay; item.maxdelay = _maxdelay;
    m_priv->replication_items.push_back(item);
}

void ZCom_Node::addReplicationString(char* _str, zU16 _maxlen, zU8 _flags, zU8 _rules, zS16 _mindelay, zS16 _maxdelay) {
    ReplicationItem item = { ReplicationItem::String, _str, NULL, false };
    item.maxlen = _maxlen; item.flags = _flags; item.rules = _rules;
    item.mindelay = _mindelay; item.maxdelay = _maxdelay;
    m_priv->replication_items.push_back(item);
}

void ZCom_Node::addReplicationStringW(wchar_t* _str, zU16 _maxlen, zU8 _flags, zU8 _rules, zS16 _mindelay, zS16 _maxdelay) {
    ReplicationItem item = { ReplicationItem::StringW, _str, NULL, false };
    item.maxlen = _maxlen; item.flags = _flags; item.rules = _rules;
    item.mindelay = _mindelay; item.maxdelay = _maxdelay;
    m_priv->replication_items.push_back(item);
}

void ZCom_Node::addInterpolationInt(zS32* _ptr, zU8 _bits, bool _sign, zU8 _flags, zU8 _rules, zS32 _treshold, zS32* _dst,
                                     zS16 _mindelay, zS16 _maxdelay, zFloat _ipolfac) {
    (void) _bits; (void) _sign; (void) _flags; (void) _rules; (void) _treshold; (void) _dst;
    (void) _mindelay; (void) _maxdelay; (void) _ipolfac;
    ReplicationItem item = { ReplicationItem::InterpInt, _ptr, NULL, false };
    m_priv->replication_items.push_back(item);
    zshim::todoPhaseBOnce("ZCom_Node::addInterpolationInt", "interpolated replication is not implemented");
}

void ZCom_Node::addInterpolationFloat(zFloat* _ptr, zU8 _mantissa_bits, zU8 _flags, zU8 _rules, zFloat _treshold, zFloat* _dst,
                                       zS16 _mindelay, zS16 _maxdelay, zFloat _ipolfac) {
    (void) _mantissa_bits; (void) _flags; (void) _rules; (void) _treshold; (void) _dst;
    (void) _mindelay; (void) _maxdelay; (void) _ipolfac;
    ReplicationItem item = { ReplicationItem::InterpFloat, _ptr, NULL, false };
    m_priv->replication_items.push_back(item);
    zshim::todoPhaseBOnce("ZCom_Node::addInterpolationFloat", "interpolated replication is not implemented");
}

void ZCom_Node::addReplicator(ZCom_Replicator* _rep, bool _autodelete) {
    // NOTE: real ZoidCom would mark the replicator initialized here, but
    // m_flags is protected on ZCom_Replicator and ZCom_Node is not a
    // friend of it in this shim. In practice this is a non-issue: every
    // concrete replicator subclass in the game (OgreVector3_Replicator,
    // OgreQuaternion_Replicator, String_Replicator,
    // EntityPropertyMapReplicator) already ORs in
    // ZCOM_REPLICATOR_INITIALIZED itself in its own constructor.
    if (_rep && (_rep->getFlags() & ZCOM_REPLICATOR_ADVANCED)) {
        // Real contract: "will get called automatically by
        // ZCom_Node::addReplicator()" -- see zoidcom_replicator_advanced.h.
        static_cast<ZCom_ReplicatorAdvanced*>(_rep)->setNode(this);
    }
    ReplicationItem item = { ReplicationItem::CustomReplicator, NULL, _rep, _autodelete };
    m_priv->replication_items.push_back(item);
}

bool ZCom_Node::endReplicationSetup(void) { return true; }

void ZCom_Node::setReplicationInterceptor(ZCom_NodeReplicationInterceptor* _interceptor) {
    m_priv->replication_interceptor = _interceptor;
}

bool ZCom_Node::sendEvent(eZCom_SendMode _mode, zU8 _rules, ZCom_BitStream* _data) {
    bool any = false;
    if (!m_priv->control) {
        delete _data;
        return false;
    }
    if (m_priv->role == eZCom_RoleAuthority) {
        for (std::map<ZCom_ConnID, LinkedConn>::iterator it = m_priv->linked_conns.begin();
             it != m_priv->linked_conns.end(); ++it) {
            bool to_proxy = (_rules & ZCOM_REPRULE_AUTH_2_PROXY) != 0 && it->second.role == eZCom_RoleProxy;
            bool to_owner = (_rules & ZCOM_REPRULE_AUTH_2_OWNER) != 0 && it->second.role == eZCom_RoleOwner;
            if (to_proxy || to_owner) {
                if (m_priv->control->ZCom_shimSendNodeEvent(it->first, m_priv->network_id, _mode, _data)) any = true;
            }
        }
    } else if (m_priv->role == eZCom_RoleOwner) {
        if ((_rules & ZCOM_REPRULE_OWNER_2_AUTH) != 0 && m_priv->authority_conn != ZCom_Invalid_ID) {
            if (m_priv->control->ZCom_shimSendNodeEvent(m_priv->authority_conn, m_priv->network_id, _mode, _data)) any = true;
        }
    }
    // Real semantics: there is no ZCOM_REPRULE_PROXY_2_AUTH -- a plain
    // (non-owner) proxy has no rule that lets it send back to the
    // authority (see zoidcom-original-semantics.md §5); sendEvent() from a
    // plain proxy is therefore a legitimate no-op, not a TODO(phaseB) gap.
    delete _data;
    return any;
}

bool ZCom_Node::sendEventDirect(eZCom_SendMode _mode, ZCom_BitStream* _data, ZCom_ConnID _destconn) {
    bool ok = false;
    if (m_priv->control) {
        ok = m_priv->control->ZCom_shimSendNodeEvent(_destconn, m_priv->network_id, _mode, _data);
    }
    delete _data;
    return ok;
}

bool ZCom_Node::sendEventToGroup(eZCom_SendMode _mode, ZCom_BitStream* _data, ZCom_GroupID _destgroup) {
    bool any = false;
    if (_destgroup == ZCOM_CONNGROUP_ALL && m_priv->control) {
        for (std::map<ZCom_ConnID, LinkedConn>::iterator it = m_priv->linked_conns.begin();
             it != m_priv->linked_conns.end(); ++it) {
            if (m_priv->control->ZCom_shimSendNodeEvent(it->first, m_priv->network_id, _mode, _data)) any = true;
        }
    } else if (_destgroup != ZCOM_CONNGROUP_ALL) {
        zshim::todoPhaseBOnce("ZCom_Node::sendEventToGroup(named-group)",
            "custom connection groups are not populated by anything yet (mirrors ZCom_sendDataToGroup's existing stub); unused by current game code");
    }
    delete _data;
    return any;
}

void ZCom_Node::setEventNotification(bool _oninit, bool _onremove) {
    m_priv->notify_onremove = _onremove;
    if (_oninit && !m_priv->notify_oninit) {
        m_priv->notify_oninit = true;
        for (size_t i = 0; i < m_priv->pending_init_before_notify.size(); i++) {
            pushEvent(eZCom_EventInit, eZCom_RoleProxy, m_priv->pending_init_before_notify[i], NULL, zshim::currentTimeMillis());
        }
        m_priv->pending_init_before_notify.clear();
    } else {
        m_priv->notify_oninit = _oninit;
    }
}

bool ZCom_Node::checkEventWaiting() const {
    return !m_priv->event_queue.empty();
}

ZCom_BitStream* ZCom_Node::getNextEvent(eZCom_Event* _type, eZCom_NodeRole* _remote_role, ZCom_ConnID* _connid, zU32* _estimated_time_sent) {
    if (m_priv->last_delivered_event) {
        delete m_priv->last_delivered_event;
        m_priv->last_delivered_event = NULL;
    }
    if (m_priv->event_queue.empty()) {
        if (_type) *_type = eZCom_EventNoEvent;
        if (_remote_role) *_remote_role = eZCom_RoleUndefined;
        if (_connid) *_connid = ZCom_Invalid_ID;
        if (_estimated_time_sent) *_estimated_time_sent = 0;
        return NULL;
    }
    PendingEvent e = m_priv->event_queue.front();
    m_priv->event_queue.pop_front();
    if (_type) *_type = e.type;
    if (_remote_role) *_remote_role = e.remote_role;
    if (_connid) *_connid = e.conn_id;
    if (_estimated_time_sent) *_estimated_time_sent = e.estimated_time_sent;
    m_priv->last_delivered_event = e.data;
    return e.data;
}

ZCom_FileTransID ZCom_Node::sendFile(const char* _path, const char* _pathtosend, ZCom_ConnID _destconn, ZCom_BitStream* _data, zFloat _aggressivenes) {
    (void) _path; (void) _pathtosend; (void) _destconn; (void) _aggressivenes;
    delete _data;
    zshim::todoPhaseBOnce("ZCom_Node::sendFile", "file transfer is not implemented");
    return ZCom_Invalid_ID;
}

void ZCom_Node::acceptFile(ZCom_ConnID _src_id, ZCom_FileTransID _ftrans_id, const char* _path, bool _accept) {
    (void) _src_id; (void) _ftrans_id; (void) _path; (void) _accept;
    zshim::todoPhaseBOnce("ZCom_Node::acceptFile", "file transfer is not implemented");
}

const ZCom_FileTransInfo& ZCom_Node::getFileInfo(ZCom_ConnID _conn_id, ZCom_FileTransID _ftrans_id) const {
    (void) _conn_id; (void) _ftrans_id;
    static ZCom_FileTransInfo invalid = { ZCom_Invalid_ID, 0, 0, NULL, 0 };
    return invalid;
}

void ZCom_Node::setEventInterceptor(ZCom_NodeEventInterceptor* _interceptor) {
    m_priv->event_interceptor = _interceptor;
}

bool ZCom_Node::setUserData(ZCom_ConnID _conn, void* _data) {
    m_priv->conn_user_data[_conn] = _data;
    return true;
}

void* ZCom_Node::getUserData(ZCom_ConnID _conn) const {
    std::map<ZCom_ConnID, void*>::const_iterator it = m_priv->conn_user_data.find(_conn);
    return it == m_priv->conn_user_data.end() ? NULL : it->second;
}

void ZCom_Node::setUserData(void* _data) { m_priv->global_user_data = _data; }
void* ZCom_Node::getUserData() const { return m_priv->global_user_data; }

zS32 ZCom_Node::getUpdateDelta() const { return -1; }
zU32 ZCom_Node::getCurrentUpdateRate() const { return 0; }
zS32 ZCom_Node::getEstimatedTimeUntilPossibleUpdate() const { return -1; }

ZCom_Control* ZCom_Node::getControl() const { return m_priv->control; }
eZCom_NodeRole ZCom_Node::getRole() const { return m_priv->role; }
bool ZCom_Node::isPrivate() const { return m_priv->is_private; }
ZCom_ClassID ZCom_Node::getClassID() const { return m_priv->class_id; }
ZCom_NodeID ZCom_Node::getNetworkID() const { return m_priv->network_id; }

// --- Phase B shim-internal hooks (see zoidcom_node.h) -----------------------

void ZCom_Node::ZCom_shimBindSelf(ZCom_NodeID _netid, ZCom_ConnID _authority_conn) {
    m_priv->network_id = _netid;
    m_priv->authority_conn = _authority_conn;
}

void ZCom_Node::ZCom_shimQueueAnnounce(ZCom_ConnID _conn) {
    LinkedConn entry;
    entry.role = eZCom_RoleProxy;
    entry.announced = false;
    entry.needs_full_sync = true;
    // Apply any setOwner() intent already recorded for _conn -- see
    // setOwner()'s comment. This is what makes setOwner() registration-order
    // independent: it may have been called long before this connection ever
    // became linked (e.g. ServerLoader.cpp's Hovercraft, whose setOwner()
    // call happens before networkRegister()/setAnnounceData() even runs the
    // loop that reaches this function), and the queued NODE_CREATE must
    // still carry the correct is_owner.
    std::map<ZCom_ConnID, bool>::const_iterator owner_it = m_priv->pending_owner_intent.find(_conn);
    if (owner_it != m_priv->pending_owner_intent.end()) {
        entry.role = owner_it->second ? eZCom_RoleOwner : eZCom_RoleProxy;
    }
    m_priv->linked_conns[_conn] = entry;
    m_priv->pending_announce.push_back(_conn);
    if (m_priv->control) m_priv->control->ZCom_shimNotePendingFlush(this);
}

void ZCom_Node::ZCom_shimFlushPendingAnnouncements() {
    if (m_priv->pending_announce.empty()) return;
    std::vector<ZCom_ConnID> conns = m_priv->pending_announce;
    m_priv->pending_announce.clear();

    for (size_t i = 0; i < conns.size(); i++) {
        ZCom_ConnID conn = conns[i];
        std::map<ZCom_ConnID, LinkedConn>::iterator it = m_priv->linked_conns.find(conn);
        if (it == m_priv->linked_conns.end()) continue; // connection dropped before we flushed
        LinkedConn& entry = it->second;

        if (m_priv->replication_interceptor) {
            m_priv->replication_interceptor->outPreReplicateNode(this, conn, entry.role);
        }

        if (m_priv->is_unique_kind) {
            m_priv->control->ZCom_shimSendNodeLinkUnique(conn, m_priv->class_id, m_priv->network_id);
            if (entry.role == eZCom_RoleOwner) {
                // Unique nodes are constructed independently (there is no
                // ZCom_cbNodeRequest_Unique callback to embed a role into),
                // so Owner promotion always needs this explicit follow-up.
                m_priv->control->ZCom_shimSendNodeOwner(conn, m_priv->network_id, true);
            }
        } else {
            m_priv->control->ZCom_shimSendNodeCreate(conn, m_priv->class_id, m_priv->network_id, entry.role, m_priv->announce_data);
        }
        entry.announced = true;
        noteConnectionLinkedEventInit(conn);

        // Full initial sync: send it right here, immediately after the
        // NODE_CREATE/NODE_LINK_UNIQUE just queued above, rather than
        // waiting for the next ZCom_shimTickReplication() call -- see that
        // function's comment for why the wait would be a real (if narrow)
        // race: ZCom_processReplicators() runs before ZCom_processOutput()
        // in the per-frame call order, so a dependent node announced in
        // this same flush cycle (e.g. RacePlayer, right after RaceState via
        // dependsOn()) could already be constructed and acted on
        // client-side before a tick-deferred sync ever arrived. Sending it
        // inline here guarantees it goes out on the wire immediately after
        // the announcement, both reliable-ordered on the same channel, so
        // order is preserved with no extra delay.
        if (entry.needs_full_sync) {
            std::vector<PendingReplItem> snapshot = ZCom_shimBuildFullReplSnapshot(m_priv);
            ZCom_shimSendReplItemsToConn(this, m_priv, conn, entry.role, snapshot, /*force_reliable=*/true);
            for (size_t s = 0; s < snapshot.size(); s++) delete snapshot[s].payload;
            entry.needs_full_sync = false;
        }

        eZCom_NodeRole role_for_conn = entry.role;
        forEachAdvancedReplicator(m_priv, [conn, role_for_conn](ZCom_ReplicatorAdvanced* rep) {
            rep->onConnectionAdded(conn, role_for_conn);
        });
    }
}

void ZCom_Node::ZCom_shimSetOwnerRole(bool _is_owner) {
    if (m_priv->role == eZCom_RoleAuthority) return; // meaningless on the authority itself
    eZCom_NodeRole oldrole = m_priv->role;
    eZCom_NodeRole newrole = _is_owner ? eZCom_RoleOwner : eZCom_RoleProxy;
    if (oldrole == newrole) return;
    m_priv->role = newrole;
    forEachAdvancedReplicator(m_priv, [oldrole, newrole](ZCom_ReplicatorAdvanced* rep) {
        rep->onLocalRoleChanged(oldrole, newrole);
    });
}

void ZCom_Node::ZCom_shimDeliverEvent(eZCom_Event _type, eZCom_NodeRole _remote_role, ZCom_ConnID _conn_id,
                                      ZCom_BitStream* _data, zU32 _estimated_time_sent) {
    pushEvent(_type, _remote_role, _conn_id, _data, _estimated_time_sent);
}

void ZCom_Node::ZCom_shimDeliverRemove(ZCom_ConnID _from_conn) {
    // Mandatory -- cannot be gated by setEventNotification() on this
    // (proxy/owner) side. See zoidcom-original-semantics.md §9.
    pushEvent(eZCom_EventRemoved, eZCom_RoleAuthority, _from_conn, NULL, zshim::currentTimeMillis());
}

void ZCom_Node::ZCom_shimNoteConnectionClosed(ZCom_ConnID _conn) {
    if (m_priv->role == eZCom_RoleAuthority) {
        std::map<ZCom_ConnID, LinkedConn>::iterator it = m_priv->linked_conns.find(_conn);
        if (it != m_priv->linked_conns.end()) {
            eZCom_NodeRole was = it->second.role;
            bool was_announced = it->second.announced;
            if (was_announced && m_priv->replication_interceptor) {
                m_priv->replication_interceptor->outPreDereplicateNode(this, _conn, was);
            }
            m_priv->linked_conns.erase(it);
            forEachAdvancedReplicator(m_priv, [_conn, was](ZCom_ReplicatorAdvanced* rep) {
                rep->onConnectionRemoved(_conn, was);
            });
            if (m_priv->notify_onremove) {
                pushEvent(eZCom_EventRemoved, was, _conn, NULL, zshim::currentTimeMillis());
            }
        }
        for (std::vector<ZCom_ConnID>::iterator pit = m_priv->pending_announce.begin();
             pit != m_priv->pending_announce.end(); ) {
            if (*pit == _conn) pit = m_priv->pending_announce.erase(pit); else ++pit;
        }
    } else if (m_priv->authority_conn == _conn && _conn != ZCom_Invalid_ID) {
        // Mandatory -- cannot be gated. See zoidcom-original-semantics.md §9.
        pushEvent(eZCom_EventRemoved, eZCom_RoleAuthority, _conn, NULL, zshim::currentTimeMillis());
        m_priv->authority_conn = ZCom_Invalid_ID;
    }
}

eZCom_NodeRole ZCom_Node::ZCom_shimRemoteRoleFor(ZCom_ConnID _conn) const {
    std::map<ZCom_ConnID, LinkedConn>::const_iterator it = m_priv->linked_conns.find(_conn);
    return it == m_priv->linked_conns.end() ? eZCom_RoleProxy : it->second.role;
}

// --- Phase B steps 3-4: the replication tick --------------------------------
// (PendingReplItem, ZCom_shimBuildFullReplSnapshot(), and
// ZCom_shimSendReplItemsToConn() -- shared with the full-sync send in
// ZCom_shimFlushPendingAnnouncements() above -- now live earlier in this
// file, in the anonymous namespace right after ZCom_Node_Private's
// definition, so both can see them.)

void ZCom_Node::ZCom_shimTickReplication(zU32 _simulation_time_passed) {
    if (!m_priv->registered || !m_priv->control || m_priv->network_id == 0) return;

    zU32 now = zshim::currentTimeMillis();

    // --- Advanced replicators: Process() once per tick, only if the
    // replicator opted into ZCOM_REPLICATOR_CALLPROCESS. Any sendData()/
    // sendDataDirect() call the replicator makes from inside Process() is
    // delivered synchronously via ZCom_shimSendAdvancedData() -- see the
    // file header for why this doesn't wait for onPreSendData().
    for (size_t i = 0; i < m_priv->replication_items.size(); i++) {
        ReplicationItem& item = m_priv->replication_items[i];
        if (item.kind != ReplicationItem::CustomReplicator || !item.replicator) continue;
        if (!(item.replicator->getFlags() & ZCOM_REPLICATOR_ADVANCED)) continue;
        if (item.replicator->callProcess()) {
            item.replicator->Process(m_priv->role, _simulation_time_passed);
        }
    }

    // A plain (non-owner) proxy has no rule that lets it send anything back
    // -- matches sendEvent()'s existing "no ZCOM_REPRULE_PROXY_2_AUTH" logic.
    if (m_priv->role != eZCom_RoleAuthority && m_priv->role != eZCom_RoleOwner) return;

    // --- Compute the dirty set once for this tick (primitives + Basic
    // custom replicators). Advanced replicators are excluded here -- they
    // are driven entirely through Process()/sendData() above.
    std::vector<PendingReplItem> pending;
    pending.reserve(m_priv->replication_items.size());

    for (size_t i = 0; i < m_priv->replication_items.size(); i++) {
        ReplicationItem& item = m_priv->replication_items[i];
        bool mostrecent = (item.flags & ZCOM_REPFLAG_MOSTRECENT) != 0;
        bool throttled = item.mindelay >= 0 && item.has_shadow &&
                         (now - item.last_send_time < (zU32) item.mindelay);
        bool resend = mostrecent && item.maxdelay >= 0 && item.has_shadow &&
                      (now - item.last_send_time >= (zU32) item.maxdelay);
        ZCom_BitStream* payload = NULL;
        bool dirty = false;

        switch (item.kind) {
        case ReplicationItem::Int: {
            zS32 cur = *(zS32*) item.ptr;
            bool changed = !item.has_shadow || cur != item.shadow_int;
            if ((changed || resend) && !(throttled && !resend)) {
                payload = new ZCom_BitStream();
                if (item.sign) payload->addSignedInt(cur, item.bits);
                else payload->addInt((zU32) cur, item.bits);
                item.shadow_int = cur; item.has_shadow = true; item.last_send_time = now;
                dirty = true;
            }
            break;
        }
        case ReplicationItem::Bool: {
            bool cur = *(bool*) item.ptr;
            bool changed = !item.has_shadow || cur != item.shadow_bool;
            if ((changed || resend) && !(throttled && !resend)) {
                payload = new ZCom_BitStream();
                payload->addBool(cur);
                item.shadow_bool = cur; item.has_shadow = true; item.last_send_time = now;
                dirty = true;
            }
            break;
        }
        case ReplicationItem::Float: {
            zFloat cur = *(zFloat*) item.ptr;
            bool changed = !item.has_shadow || cur != item.shadow_float;
            if ((changed || resend) && !(throttled && !resend)) {
                payload = new ZCom_BitStream();
                payload->addFloat(cur, item.bits);
                item.shadow_float = cur; item.has_shadow = true; item.last_send_time = now;
                dirty = true;
            }
            break;
        }
        case ReplicationItem::String: {
            const char* cur = (const char*) item.ptr;
            bool changed = !item.has_shadow || item.shadow_str != cur;
            if ((changed || resend) && !(throttled && !resend)) {
                payload = new ZCom_BitStream();
                payload->addString(cur);
                item.shadow_str = cur; item.has_shadow = true; item.last_send_time = now;
                dirty = true;
            }
            break;
        }
        case ReplicationItem::StringW: {
            const wchar_t* cur = (const wchar_t*) item.ptr;
            bool changed = !item.has_shadow || item.shadow_wstr != cur;
            if ((changed || resend) && !(throttled && !resend)) {
                payload = new ZCom_BitStream();
                payload->addStringW(cur);
                item.shadow_wstr = cur; item.has_shadow = true; item.last_send_time = now;
                dirty = true;
            }
            break;
        }
        case ReplicationItem::CustomReplicator: {
            if (!item.replicator || !(item.replicator->getFlags() & ZCOM_REPLICATOR_BASIC)) break;
            ZCom_ReplicatorBasic* rb = static_cast<ZCom_ReplicatorBasic*>(item.replicator);
            ZCom_ReplicatorSetup* setup = rb->getSetup();
            // The setup is the authoritative source of flags/rules/delays
            // for a custom replicator (the addReplicator() caller passes
            // them there, not to ZCom_Node) -- refresh the item's copies
            // every tick so pending-item construction below can treat all
            // kinds uniformly.
            item.flags = setup ? setup->getFlags() : 0;
            item.rules = setup ? setup->getRules() : item.rules;
            item.mindelay = setup ? setup->getMinDelay() : (zS16) -1;
            item.maxdelay = setup ? setup->getMaxDelay() : (zS16) -1;
            mostrecent = (item.flags & ZCOM_REPFLAG_MOSTRECENT) != 0;
            throttled = item.mindelay >= 0 && item.has_shadow &&
                        (now - item.last_send_time < (zU32) item.mindelay);
            resend = mostrecent && item.maxdelay >= 0 && item.has_shadow &&
                     (now - item.last_send_time >= (zU32) item.maxdelay);
            if (throttled && !resend) break;
            // checkState() has its own dirty-tracking side effect (see e.g.
            // OgreVector3_Replicator::checkState()'s mCompare) and must run
            // to keep that internal state correct -- but only when we are
            // not throttled, so a value that keeps changing during a
            // mindelay window is still only detected/sent at most that
            // often (delayed detection, not lost detection).
            bool changed = rb->checkState();
            if (changed || resend) {
                payload = new ZCom_BitStream();
                rb->packData(payload);
                item.has_shadow = true; item.last_send_time = now;
                dirty = true;
            }
            break;
        }
        default: break;
        }

        if (dirty && payload) {
            PendingReplItem p;
            p.index = (zU16) i;
            p.rules = item.rules;
            p.unreliable = mostrecent;
            p.intercept = (item.flags & ZCOM_REPFLAG_INTERCEPT) != 0;
            p.replicator = (item.kind == ReplicationItem::CustomReplicator) ? item.replicator : NULL;
            p.payload = payload;
            pending.push_back(p);
        } else if (payload) {
            delete payload;
        }
    }

    // Note: the one-time full sync for a newly-linked connection (see
    // LinkedConn::needs_full_sync) is NOT handled here. It is sent
    // synchronously from ZCom_shimFlushPendingAnnouncements(), immediately
    // after that connection's NODE_CREATE/NODE_LINK_UNIQUE goes out, rather
    // than waiting for this function's next invocation -- see that
    // function's comment for why: ZCom_Control::ZCom_processReplicators()
    // (which drives this function) runs *before* ZCom_processOutput() (which
    // drives the announcement flush) in the game's per-frame call order, so
    // gating a full sync here on "already announced" would always be one
    // full frame late -- late enough for a dependent node (e.g. RacePlayer,
    // announced in the same flush right after RaceState) to already be
    // constructed and act on stale data client-side before the sync ever
    // arrives. Sending it inline in the flush avoids that race entirely
    // while still guaranteeing wire order (both ride the same reliable-
    // ordered channel, sent back-to-back in the same function call).
    if (!pending.empty()) {
        // --- Distribute to every relevant connection, per direction rules,
        // batched into at most one reliable + one unreliable envelope per
        // connection (see phase-b-replication.md §4's "batch per node per
        // tick" guidance and Control.cpp's kMsgNodeReplBatch).
        std::vector<ZCom_ConnID> targets;
        std::map<ZCom_ConnID, eZCom_NodeRole> target_role;
        if (m_priv->role == eZCom_RoleAuthority) {
            for (std::map<ZCom_ConnID, LinkedConn>::iterator it = m_priv->linked_conns.begin();
                 it != m_priv->linked_conns.end(); ++it) {
                targets.push_back(it->first);
                target_role[it->first] = it->second.role;
            }
        } else { // eZCom_RoleOwner
            if (m_priv->authority_conn != ZCom_Invalid_ID) targets.push_back(m_priv->authority_conn);
        }

        for (size_t t = 0; t < targets.size(); t++) {
            ZCom_ConnID conn = targets[t];
            eZCom_NodeRole remote_role = (m_priv->role == eZCom_RoleAuthority) ? target_role[conn] : eZCom_RoleAuthority;
            ZCom_shimSendReplItemsToConn(this, m_priv, conn, remote_role, pending, /*force_reliable=*/false);
        }
    }

    for (size_t p = 0; p < pending.size(); p++) delete pending[p].payload;
}

void ZCom_Node::ZCom_shimApplyReplBatch(ZCom_BitStream& _envelope, ZCom_ConnID _from_conn,
                                         eZCom_NodeRole _remote_role, zU32 _estimated_time_sent) {
    zU16 count = (zU16) _envelope.getInt(16);

    // inPreUpdate(): "Should return 'false' to prevent applying the
    // replication data updates. Events will be received nevertheless."
    // (zoidcom_node_interceptors.h). We still fully parse the envelope below
    // regardless -- this shim's NODE_REPL_BATCH message is self-contained
    // (its own length-prefixed item list), so skipping application never
    // risks desyncing a read cursor the way it might in a combined packet.
    bool allow_apply = true;
    if (m_priv->replication_interceptor) {
        allow_apply = m_priv->replication_interceptor->inPreUpdate(this, _from_conn, _remote_role);
    }

    zU32 rep_bits = 16; // the count field itself
    for (zU16 i = 0; i < count; i++) {
        zU16 index = (zU16) _envelope.getInt(16);
        zU32 bits = _envelope.getInt(16);
        ZCom_BitStream* sub = _envelope.getBitStream(bits, true);
        rep_bits += 32 + bits; // index + length fields, plus the payload itself

        if (index < m_priv->replication_items.size()) {
            ReplicationItem& item = m_priv->replication_items[index];

            // inPreUpdateItem(): gated by ZCOM_REPFLAG_INTERCEPT, same
            // reasoning as outPreUpdateItem() in ZCom_shimTickReplication()
            // -- no current game code sets the flag, so the primitive-item
            // branch is unreachable today.
            bool allow_item = allow_apply;
            if (allow_item && (item.flags & ZCOM_REPFLAG_INTERCEPT) && m_priv->replication_interceptor) {
                if (item.kind == ReplicationItem::CustomReplicator && item.replicator) {
                    allow_item = m_priv->replication_interceptor->inPreUpdateItem(
                        this, _from_conn, _remote_role, item.replicator, _estimated_time_sent);
                } else {
                    zshim::todoPhaseBOnce("ZCom_Node::ZCom_shimApplyReplBatch(intercept-primitive)",
                        "ZCOM_REPFLAG_INTERCEPT set on a primitive replication item; inPreUpdateItem() "
                        "can't be called without a ZCom_Replicator instance to pass, applying the update "
                        "unfiltered (unused by any current game code)");
                }
            }

            if (allow_item) {
                switch (item.kind) {
                case ReplicationItem::Int:
                    if (item.sign) *(zS32*) item.ptr = sub->getSignedInt(item.bits);
                    else *(zS32*) item.ptr = (zS32) sub->getInt(item.bits);
                    break;
                case ReplicationItem::Bool:
                    *(bool*) item.ptr = sub->getBool();
                    break;
                case ReplicationItem::Float:
                    *(zFloat*) item.ptr = sub->getFloat(item.bits);
                    break;
                case ReplicationItem::String:
                    sub->getString((char*) item.ptr, item.maxlen);
                    break;
                case ReplicationItem::StringW:
                    sub->getStringW((wchar_t*) item.ptr, item.maxlen);
                    break;
                case ReplicationItem::CustomReplicator:
                    if (item.replicator && (item.replicator->getFlags() & ZCOM_REPLICATOR_BASIC)) {
                        static_cast<ZCom_ReplicatorBasic*>(item.replicator)->unpackData(sub, true, _estimated_time_sent);
                    }
                    break;
                default: break;
                }
            }
        }
        delete sub;
    }

    // inPostUpdate(): "Incoming data has updated the node" -- only fires
    // when an update was actually (attempted to be) applied, i.e. when
    // inPreUpdate() didn't veto the whole batch.
    if (allow_apply && m_priv->replication_interceptor) {
        m_priv->replication_interceptor->inPostUpdate(this, _from_conn, _remote_role, rep_bits, 0, 0);
    }
}

void ZCom_Node::ZCom_shimSendAdvancedData(ZCom_Replicator* _rep, eZCom_SendMode _mode, ZCom_BitStream* _stream,
                                           zU32 _reference_id, ZCom_ConnID _direct_dest) {
    if (!m_priv->control || !_rep) { delete _stream; return; }

    // Find this replicator's item index -- both peers built their
    // replication_items list identically via the same setupReplication(),
    // so the index doubles as a stable cross-peer identifier (see
    // phase-b-replication.md §3), letting the receiver's onDataReceived()
    // dispatch to the matching replicator instance.
    zU16 index = 0xFFFF;
    for (size_t i = 0; i < m_priv->replication_items.size(); i++) {
        if (m_priv->replication_items[i].replicator == _rep) { index = (zU16) i; break; }
    }
    if (index == 0xFFFF) {
        zshim::todoPhaseBOnce("ZCom_Node::ZCom_shimSendAdvancedData(unregistered-replicator)",
            "sendData()/sendDataDirect() called from a ZCom_ReplicatorAdvanced not found in its node's replication_items");
        delete _stream;
        return;
    }

    if (_direct_dest != ZCom_Invalid_ID) {
        m_priv->control->ZCom_shimSendNodeReplAdvanced(_direct_dest, m_priv->network_id, index, _mode, _stream, _reference_id);
        delete _stream;
        return;
    }

    // Broadcast, filtered per the setup's replication rule direction -- the
    // real contract for sendData(): "all replicators which normally
    // receive data from this replicator, too" (see
    // zoidcom-original-semantics.md §7). Same direction semantics as the
    // primitive/basic replication tick and sendEvent().
    ZCom_ReplicatorSetup* setup = _rep->getSetup();
    zU8 rules = setup ? setup->getRules() : 0;

    if (m_priv->role == eZCom_RoleAuthority) {
        for (std::map<ZCom_ConnID, LinkedConn>::iterator it = m_priv->linked_conns.begin();
             it != m_priv->linked_conns.end(); ++it) {
            bool to_proxy = (rules & ZCOM_REPRULE_AUTH_2_PROXY) != 0 && it->second.role == eZCom_RoleProxy;
            bool to_owner = (rules & ZCOM_REPRULE_AUTH_2_OWNER) != 0 && it->second.role == eZCom_RoleOwner;
            if (to_proxy || to_owner) {
                m_priv->control->ZCom_shimSendNodeReplAdvanced(it->first, m_priv->network_id, index, _mode, _stream, _reference_id);
            }
        }
    } else if (m_priv->role == eZCom_RoleOwner) {
        if ((rules & ZCOM_REPRULE_OWNER_2_AUTH) != 0 && m_priv->authority_conn != ZCom_Invalid_ID) {
            m_priv->control->ZCom_shimSendNodeReplAdvanced(m_priv->authority_conn, m_priv->network_id, index, _mode, _stream, _reference_id);
        }
    }
    delete _stream;
}

void ZCom_Node::ZCom_shimDeliverReplAdvanced(zU16 _item_index, ZCom_ConnID _from_conn, eZCom_NodeRole _remote_role,
                                              ZCom_BitStream* _payload, zU32 _estimated_time_sent) {
    if (_item_index < m_priv->replication_items.size()) {
        ReplicationItem& item = m_priv->replication_items[_item_index];
        if (item.kind == ReplicationItem::CustomReplicator && item.replicator &&
            (item.replicator->getFlags() & ZCOM_REPLICATOR_ADVANCED)) {
            static_cast<ZCom_ReplicatorAdvanced*>(item.replicator)->onDataReceived(
                _from_conn, _remote_role, *_payload, true, _estimated_time_sent);
        }
    }
    delete _payload;
}
