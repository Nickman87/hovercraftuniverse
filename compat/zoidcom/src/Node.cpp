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
// Still TODO(phaseB) stubs, all logged once via zshim::todoPhaseBOnce():
//   - Data replication (addReplicationInt/Float/Bool/String/StringW,
//     addInterpolationInt/Float, addReplicator): parameters are recorded
//     for bookkeeping (so autodelete and getSetup() etc. still work) but
//     values are never synced across the network. This is step 3 of
//     docs/porting/phase-b-replication.md, deliberately out of scope here.
//   - registerNodeByTag/Zoidlevels, mustsync/authority migration (beyond
//     setOwner), file transfer.
// See docs/porting/zoidcom-compat.md for the full design rationale.
#include "zoidcom_shim_internal.h"
#include <vector>
#include <map>
#include <deque>

namespace {

struct ReplicationItem {
    // Bookkeeping only -- see file header. Kept so the API is faithfully
    // shaped even though nothing consumes this list yet (step 3).
    enum Kind { Int, Bool, Float, String, StringW, InterpInt, InterpFloat, CustomReplicator } kind;
    void* ptr;
    ZCom_Replicator* replicator;
    bool autodelete;
};

// Authority-side bookkeeping for one connection this node is relevant to.
struct LinkedConn {
    eZCom_NodeRole role;   // eZCom_RoleProxy or eZCom_RoleOwner, from this node's perspective
    bool announced;        // NODE_CREATE/NODE_LINK_UNIQUE already sent for this connection
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

    std::map<ZCom_ConnID, void*> conn_user_data;
    void* global_user_data;

    // --- Phase B linking/event state --------------------------------------

    // Authority-side: every connection this node is (or is about to become)
    // relevant to.
    std::map<ZCom_ConnID, LinkedConn> linked_conns;
    // Authority-side: connections queued for the deferred announcement flush
    // (see ZCom_shimFlushPendingAnnouncements()).
    std::vector<ZCom_ConnID> pending_announce;

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
    (void) _othernode; (void) _opt;
    zshim::todoPhaseBOnce("ZCom_Node::dependsOn", "replication ordering dependencies are not tracked (step 3 item)");
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
    if (m_priv->role != eZCom_RoleAuthority) {
        zshim::todoPhaseBOnce("ZCom_Node::setOwner(non-authority)",
            "setOwner() only has an effect when called on the authority node; called on a non-authority node");
        return;
    }
    std::map<ZCom_ConnID, LinkedConn>::iterator it = m_priv->linked_conns.find(_id);
    if (it == m_priv->linked_conns.end()) {
        // _id isn't (yet) relevant to this node -- legitimate no-op.
        return;
    }
    it->second.role = _enabled ? eZCom_RoleOwner : eZCom_RoleProxy;
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
    (void) _bits; (void) _sign; (void) _flags; (void) _rules; (void) _mindelay; (void) _maxdelay;
    ReplicationItem item = { ReplicationItem::Int, _ptr, NULL, false };
    m_priv->replication_items.push_back(item);
    zshim::todoPhaseBOnce("ZCom_Node::addReplicationInt", "registered replication fields are never synced across the network (step 3)");
}

void ZCom_Node::addReplicationBool(bool* _ptr, zU8 _flags, zU8 _rules, zS16 _mindelay, zS16 _maxdelay) {
    (void) _flags; (void) _rules; (void) _mindelay; (void) _maxdelay;
    ReplicationItem item = { ReplicationItem::Bool, _ptr, NULL, false };
    m_priv->replication_items.push_back(item);
    zshim::todoPhaseBOnce("ZCom_Node::addReplicationBool", "registered replication fields are never synced across the network (step 3)");
}

void ZCom_Node::addReplicationFloat(zFloat* _ptr, zU8 _mantissa_bits, zU8 _flags, zU8 _rules, zS16 _mindelay, zS16 _maxdelay) {
    (void) _mantissa_bits; (void) _flags; (void) _rules; (void) _mindelay; (void) _maxdelay;
    ReplicationItem item = { ReplicationItem::Float, _ptr, NULL, false };
    m_priv->replication_items.push_back(item);
    zshim::todoPhaseBOnce("ZCom_Node::addReplicationFloat", "registered replication fields are never synced across the network (step 3)");
}

void ZCom_Node::addReplicationString(char* _str, zU16 _maxlen, zU8 _flags, zU8 _rules, zS16 _mindelay, zS16 _maxdelay) {
    (void) _maxlen; (void) _flags; (void) _rules; (void) _mindelay; (void) _maxdelay;
    ReplicationItem item = { ReplicationItem::String, _str, NULL, false };
    m_priv->replication_items.push_back(item);
    zshim::todoPhaseBOnce("ZCom_Node::addReplicationString", "registered replication fields are never synced across the network (step 3)");
}

void ZCom_Node::addReplicationStringW(wchar_t* _str, zU16 _maxlen, zU8 _flags, zU8 _rules, zS16 _mindelay, zS16 _maxdelay) {
    (void) _maxlen; (void) _flags; (void) _rules; (void) _mindelay; (void) _maxdelay;
    ReplicationItem item = { ReplicationItem::StringW, _str, NULL, false };
    m_priv->replication_items.push_back(item);
    zshim::todoPhaseBOnce("ZCom_Node::addReplicationStringW", "registered replication fields are never synced across the network (step 3)");
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
    ReplicationItem item = { ReplicationItem::CustomReplicator, NULL, _rep, _autodelete };
    m_priv->replication_items.push_back(item);
    zshim::todoPhaseBOnce("ZCom_Node::addReplicator",
        "custom replicators are stored (and autodeleted with the node) but never driven by a replication tick (step 3)");
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
    }
}

void ZCom_Node::ZCom_shimSetOwnerRole(bool _is_owner) {
    if (m_priv->role == eZCom_RoleAuthority) return; // meaningless on the authority itself
    m_priv->role = _is_owner ? eZCom_RoleOwner : eZCom_RoleProxy;
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
            m_priv->linked_conns.erase(it);
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
