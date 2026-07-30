// ZoidCom-compatible shim: ZCom_Node.
//
// Real (Phase A): node identity bookkeeping -- class id, role, owning
// control, network id assignment for authority nodes, per-connection and
// global user data, replicator list ownership (with autodelete).
//
// TODO(phaseB) stubs, all logged once via zshim::todoPhaseBOnce():
//   - Cross-network node linking. Real ZoidCom links an authority node on
//     one ZCom_Control to proxy nodes on connected ZCom_Controls (matching
//     by class id + registration order/tag/dynamic-request), and that
//     link is what actually drives replication and event delivery. This
//     shim does not implement that link at all: every ZCom_Node is
//     islanded on its own ZCom_Control.
//   - Data replication (addReplicationInt/Float/Bool/String/StringW,
//     addInterpolationInt/Float, addReplicator): parameters are recorded
//     for bookkeeping (so autodelete and getSetup() etc. still work) but
//     values are never synced across the network.
//   - Event queue: checkEventWaiting()/getNextEvent() are always empty --
//     sendEvent()/sendEventDirect()/sendEventToGroup() log and drop.
//   - Zoidlevels, mustsync/authority migration, file transfer.
// See docs/porting/zoidcom-compat.md for the full design rationale and
// what Phase B needs to build to make this real.
#include "zoidcom_shim_internal.h"
#include <vector>
#include <map>
#include <atomic>

namespace {
std::atomic<ZCom_NodeID> gNextNetworkId(1);

struct ReplicationItem {
    // Bookkeeping only -- see file header. Kept so the API is faithfully
    // shaped even though nothing consumes this list yet.
    enum Kind { Int, Bool, Float, String, StringW, InterpInt, InterpFloat, CustomReplicator } kind;
    void* ptr;
    ZCom_Replicator* replicator;
    bool autodelete;
};
}

class ZCom_Node_Private {
public:
    ZCom_ClassID class_id;
    ZCom_Control* control;
    eZCom_NodeRole role;
    ZCom_NodeID network_id;
    bool registered;
    bool is_private;
    zU16 update_priority;
    zFloat default_relevance;

    std::vector<ReplicationItem> replication_items;
    ZCom_NodeReplicationInterceptor* replication_interceptor;
    ZCom_NodeEventInterceptor* event_interceptor;

    std::map<ZCom_ConnID, void*> conn_user_data;
    void* global_user_data;

    ZCom_Node_Private()
        : class_id(ZCom_Invalid_ID), control(NULL), role(eZCom_RoleUndefined),
          network_id(0), registered(false), is_private(false),
          update_priority(0), default_relevance(1.0f),
          replication_interceptor(NULL), event_interceptor(NULL),
          global_user_data(NULL) {}

    ~ZCom_Node_Private() {
        for (size_t i = 0; i < replication_items.size(); i++) {
            if (replication_items[i].kind == ReplicationItem::CustomReplicator &&
                replication_items[i].autodelete && replication_items[i].replicator) {
                delete replication_items[i].replicator;
            }
        }
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
    if (_role == eZCom_RoleAuthority) {
        p->network_id = gNextNetworkId++;
    } else {
        p->network_id = 0; // see TODO below
        zshim::todoPhaseBOnce("ZCom_Node::registerNode*(non-authority)",
            "non-authority nodes never receive a real network id because cross-network node linking is not implemented");
    }
    return true;
}

bool ZCom_Node::registerNodeUnique(ZCom_ClassID _classid, eZCom_NodeRole _role, ZCom_Control* _control) {
    zshim::todoPhaseBOnce("ZCom_Node::registerNodeUnique",
        "node is bookkept locally but never linked to a counterpart node on a connected ZCom_Control");
    return registerCommon(m_priv, _classid, _role, _control);
}

bool ZCom_Node::registerNodeByTag(ZCom_ClassID _classid, zU32 _tag, eZCom_NodeRole _role, ZCom_Control* _control) {
    (void) _tag;
    zshim::todoPhaseBOnce("ZCom_Node::registerNodeByTag",
        "tag-based node linking across connections is not implemented");
    return registerCommon(m_priv, _classid, _role, _control);
}

bool ZCom_Node::registerNodeDynamic(ZCom_ClassID _classid, ZCom_Control* _control) {
    zshim::todoPhaseBOnce("ZCom_Node::registerNodeDynamic",
        "dynamic node spawn requests are never sent to connected peers");
    return registerCommon(m_priv, _classid, eZCom_RoleAuthority, _control);
}

bool ZCom_Node::registerRequestedNode(ZCom_ClassID _classid, ZCom_Control* _control) {
    return registerCommon(m_priv, _classid, eZCom_RoleProxy, _control);
}

bool ZCom_Node::unregisterNode() {
    disconnectAll();
    m_priv->registered = false;
    return true;
}

void ZCom_Node::disconnectAll() {
    // No real cross-connection link exists yet to tear down (see file
    // header); kept as a real no-op rather than a logged stub since it's
    // genuinely a correct no-op given the current design.
}

void ZCom_Node::setUpdatePriority(zU16 _prio) { m_priv->update_priority = _prio; }
void ZCom_Node::setDefaultRelevance(zFloat _default_relevance) { m_priv->default_relevance = _default_relevance; }

void ZCom_Node::setConnectionSpecificRelevance(ZCom_ConnID _conn, zFloat _rel) {
    (void) _conn; (void) _rel;
    zshim::todoPhaseBOnce("ZCom_Node::setConnectionSpecificRelevance", "per-connection relevance is not tracked");
}

zU32 ZCom_Node::getRelevantConnectionCount() const { return 0; }

zS32 ZCom_Node::getRelevantConnections(ZCom_ConnID* _conns, zU32 _max, zU32* _count) const {
    (void) _conns; (void) _max;
    if (_count) *_count = 0;
    return 1; // "ok", zero connections -- matches real semantics for an unlinked node
}

void ZCom_Node::dependsOn(ZCom_Node* _othernode, eZCom_DependencyOpt _opt) {
    (void) _othernode; (void) _opt;
    zshim::todoPhaseBOnce("ZCom_Node::dependsOn", "replication ordering dependencies are not tracked");
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
    (void) _id; (void) _enabled;
    zshim::todoPhaseBOnce("ZCom_Node::setOwner",
        "authority migration (granting eZCom_RoleOwner to a remote proxy) is not implemented");
}

void ZCom_Node::setPrivate(bool _enabled) { m_priv->is_private = _enabled; }

void ZCom_Node::setAnnounceData(ZCom_BitStream* _data) {
    delete _data;
    zshim::todoPhaseBOnce("ZCom_Node::setAnnounceData",
        "announcement data is accepted and discarded (no dynamic-node announcement protocol is implemented)");
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
    zshim::todoPhaseBOnce("ZCom_Node::addReplicationInt", "registered replication fields are never synced across the network");
}

void ZCom_Node::addReplicationBool(bool* _ptr, zU8 _flags, zU8 _rules, zS16 _mindelay, zS16 _maxdelay) {
    (void) _flags; (void) _rules; (void) _mindelay; (void) _maxdelay;
    ReplicationItem item = { ReplicationItem::Bool, _ptr, NULL, false };
    m_priv->replication_items.push_back(item);
    zshim::todoPhaseBOnce("ZCom_Node::addReplicationBool", "registered replication fields are never synced across the network");
}

void ZCom_Node::addReplicationFloat(zFloat* _ptr, zU8 _mantissa_bits, zU8 _flags, zU8 _rules, zS16 _mindelay, zS16 _maxdelay) {
    (void) _mantissa_bits; (void) _flags; (void) _rules; (void) _mindelay; (void) _maxdelay;
    ReplicationItem item = { ReplicationItem::Float, _ptr, NULL, false };
    m_priv->replication_items.push_back(item);
    zshim::todoPhaseBOnce("ZCom_Node::addReplicationFloat", "registered replication fields are never synced across the network");
}

void ZCom_Node::addReplicationString(char* _str, zU16 _maxlen, zU8 _flags, zU8 _rules, zS16 _mindelay, zS16 _maxdelay) {
    (void) _maxlen; (void) _flags; (void) _rules; (void) _mindelay; (void) _maxdelay;
    ReplicationItem item = { ReplicationItem::String, _str, NULL, false };
    m_priv->replication_items.push_back(item);
    zshim::todoPhaseBOnce("ZCom_Node::addReplicationString", "registered replication fields are never synced across the network");
}

void ZCom_Node::addReplicationStringW(wchar_t* _str, zU16 _maxlen, zU8 _flags, zU8 _rules, zS16 _mindelay, zS16 _maxdelay) {
    (void) _maxlen; (void) _flags; (void) _rules; (void) _mindelay; (void) _maxdelay;
    ReplicationItem item = { ReplicationItem::StringW, _str, NULL, false };
    m_priv->replication_items.push_back(item);
    zshim::todoPhaseBOnce("ZCom_Node::addReplicationStringW", "registered replication fields are never synced across the network");
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
        "custom replicators are stored (and autodeleted with the node) but never driven by a replication tick");
}

bool ZCom_Node::endReplicationSetup(void) { return true; }

void ZCom_Node::setReplicationInterceptor(ZCom_NodeReplicationInterceptor* _interceptor) {
    m_priv->replication_interceptor = _interceptor;
}

bool ZCom_Node::sendEvent(eZCom_SendMode _mode, zU8 _rules, ZCom_BitStream* _data) {
    (void) _mode; (void) _rules;
    delete _data;
    zshim::todoPhaseBOnce("ZCom_Node::sendEvent",
        "node-to-node event delivery is not implemented (no cross-network node link exists)");
    return false;
}

bool ZCom_Node::sendEventDirect(eZCom_SendMode _mode, ZCom_BitStream* _data, ZCom_ConnID _destconn) {
    (void) _mode; (void) _destconn;
    delete _data;
    zshim::todoPhaseBOnce("ZCom_Node::sendEventDirect",
        "node-to-node event delivery is not implemented (no cross-network node link exists)");
    return false;
}

bool ZCom_Node::sendEventToGroup(eZCom_SendMode _mode, ZCom_BitStream* _data, ZCom_GroupID _destgroup) {
    (void) _mode; (void) _destgroup;
    delete _data;
    zshim::todoPhaseBOnce("ZCom_Node::sendEventToGroup",
        "node-to-node event delivery is not implemented (no cross-network node link exists)");
    return false;
}

void ZCom_Node::setEventNotification(bool _oninit, bool _onremove) {
    (void) _oninit; (void) _onremove;
}

bool ZCom_Node::checkEventWaiting() const {
    return false;
}

ZCom_BitStream* ZCom_Node::getNextEvent(eZCom_Event* _type, eZCom_NodeRole* _remote_role, ZCom_ConnID* _connid, zU32* _estimated_time_sent) {
    if (_type) *_type = eZCom_EventNoEvent;
    if (_remote_role) *_remote_role = eZCom_RoleUndefined;
    if (_connid) *_connid = ZCom_Invalid_ID;
    if (_estimated_time_sent) *_estimated_time_sent = 0;
    return NULL;
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
