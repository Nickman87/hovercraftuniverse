// ZoidCom-compatible shim: ZCom_Control.
//
// Real ENet transport underneath: ZCom_initSockets()/ZCom_Connect()/
// ZCom_Disconnect()/ZCom_disconnectAll()/ZCom_processInput()/
// ZCom_processOutput() and raw ZCom_sendData()/ZCom_cbDataReceived() all
// work over an actual ENetHost. Class ID registration is a real (if
// trivial) name->id table.
//
// Real (Phase B, steps 1-2 -- see docs/porting/phase-b-replication.md and
// docs/porting/zoidcom-original-semantics.md): this file now owns the
// per-control node registry (network-id space, netid->node map, and
// class-id->node map for locally-registered unique nodes) and the wire
// dispatch for cross-network node linking and node-to-node events:
//   NODE_CREATE (kind 2)       authority -> proxy: spawn a dynamic node
//   NODE_LINK_UNIQUE (kind 3)  authority -> proxy: link a unique node
//   NODE_REMOVE (kind 4)       authority -> proxy: a node went away
//   NODE_OWNER (kind 5)        authority -> proxy: role promotion/demotion
//   NODE_EVENT (kind 6)        both ways: a ZCom_Node::sendEvent* payload
// See docs/porting/phase-b-replication.md §4 for the full wire table.
//
// IMPORTANT (see phase-b-replication.md §6.1): the node registry and the
// network-id counter below are members of ZCom_Control_Private, i.e.
// scoped per ZCom_Control instance. They must NEVER become static/file-
// global: single-player runs the server and client as two ZCom_Control
// instances in the SAME PROCESS (one on a boost thread, one on the main
// thread), and a shared registry would let the client resolve a network id
// to the server's own ZCom_Node object -- which would appear to work in
// single-player and fail utterly over a real network.
//
// KNOWN SEMANTIC GAP (documented in docs/porting/zoidcom-compat.md):
// real ZoidCom's connection handshake is app-negotiated -- the server's
// ZCom_cbConnectionRequest() can reject a connection *before* the client
// is told it's connected. ENet's handshake is transport-level and
// completes before any application code runs, so:
//   - the client's ZCom_cbConnectResult() fires as soon as the ENet
//     handshake completes, always with eZCom_ConnAccepted (there is no
//     way to carry an app-level "denied" decision back through ENet's
//     connect handshake without a round trip we don't yet implement);
//   - the server's ZCom_cbConnectionRequest() is called immediately after
//     accepting the transport connection, with an always-empty request
//     bitstream (ENet's connect() call only carries a single 32-bit
//     integer, not an arbitrary bitstream) -- if the callback returns
//     false, the shim disconnects the peer immediately, which the client
//     sees as an immediate ZCom_cbConnectionClosed() *after* it already
//     saw ZCom_cbConnectResult(eZCom_ConnAccepted). Real ZoidCom would
//     never have told the client it was accepted in this case.
// TODO(phaseB): a proper app-level handshake (send request/reply as the
// first reliable packets and gate ZCom_cbConnectResult on the reply)
// would close this gap.
//
// TODO(phaseB) stubs: Zoidlevels/ZCom_requestZoidMode, LAN discovery
// (ZCom_Discover/ZCom_setDiscoverListener), lag/loss simulation.
#include "zoidcom_shim_internal.h"
#include <enet/enet.h>
#include <map>
#include <set>
#include <vector>
#include <string>
#include <cstring>

namespace {
const zU8 kMsgRawData        = 0;
const zU8 kMsgDisconnectReason = 1;
const zU8 kMsgNodeCreate      = 2;
const zU8 kMsgNodeLinkUnique  = 3;
const zU8 kMsgNodeRemove      = 4;
const zU8 kMsgNodeOwner       = 5;
const zU8 kMsgNodeEvent       = 6;

const size_t kMaxPeers = 64;
const size_t kChannelCount = 2;
}

class ZCom_Control_Private {
public:
    ENetHost* host;
    std::string debug_name;
    zU8 control_id;

    ZCom_ConnID next_conn_id;
    std::map<ZCom_ConnID, ENetPeer*> conn_to_peer;
    std::map<ENetPeer*, ZCom_ConnID> peer_to_conn;
    std::map<ZCom_ConnID, ZCom_Address> conn_addr;
    std::map<ZCom_ConnID, void*> user_data;
    std::map<ZCom_ConnID, ZCom_ConnStats> stats;
    std::map<ZCom_ConnID, std::string> pending_disconnect_reason;

    std::vector<std::string> class_names;      // index 0 unused (ZCom_Invalid_ID == 0)
    std::map<std::string, ZCom_ClassID> class_ids;

    ZCom_ConnGroupManager group_mgr;
    ZCom_ConnStats zero_stats;

    // --- Phase B: per-control node registry (see file header) --------------
    ZCom_NodeID next_network_id;
    std::map<ZCom_NodeID, ZCom_Node*> nodes_by_netid;
    std::map<ZCom_ClassID, ZCom_Node*> unique_nodes_by_class;
    std::set<ZCom_Node*> pending_flush_nodes;

    // Valid only while dispatching a NODE_CREATE's ZCom_cbNodeRequest_Dynamic()
    // callback -- lets ZCom_Node::registerNodeDynamic(), called by the game
    // from inside that callback, discover the netid/authority-connection the
    // incoming message assigned.
    bool dispatching_dynamic_request;
    ZCom_NodeID pending_dynamic_netid;
    ZCom_ConnID pending_dynamic_conn;

    ZCom_Control_Private()
        : host(NULL), control_id(0), next_conn_id(1),
          next_network_id(1), dispatching_dynamic_request(false),
          pending_dynamic_netid(0), pending_dynamic_conn(ZCom_Invalid_ID) {
        memset(&zero_stats, 0, sizeof(zero_stats));
        class_names.push_back(""); // ZCom_Invalid_ID placeholder
    }
};

namespace zshim { bool enetAcquire(); void enetRelease(); }

ZCom_Control::ZCom_Control(void) : m_priv(new ZCom_Control_Private()) {
    zshim::enetAcquire();
}

ZCom_Control::~ZCom_Control(void) {
    Shutdown();
    zshim::enetRelease();
    delete m_priv;
}

void ZCom_Control::Shutdown(void) {
    if (m_priv->host) {
        ZCom_disconnectAll(NULL);
        enet_host_destroy(m_priv->host);
        m_priv->host = NULL;
    }
}

void ZCom_Control::ZCom_setDebugName(const char* _name) {
    m_priv->debug_name = _name ? _name : "";
}

bool ZCom_Control::ZCom_initSockets(bool _useudp, zU16 _udpport, zU16 _localport, zU8 _control_id_size) {
    (void) _control_id_size;
    zU16 port = _useudp ? _udpport : _localport;

    ENetAddress addr;
    addr.host = ENET_HOST_ANY;
    addr.port = port;

    m_priv->host = enet_host_create(&addr, kMaxPeers, kChannelCount, 0, 0);
    if (!m_priv->host) {
        zshim::logf("ZCom_Control[%s]::ZCom_initSockets failed to create ENet host on port %u",
                     m_priv->debug_name.c_str(), (unsigned) port);
        return false;
    }
    return true;
}

bool ZCom_Control::ZCom_setDiscoverListener(eZCom_DiscoverOpt _opt, zU16 _discoverport) {
    (void) _opt; (void) _discoverport;
    zshim::todoPhaseBOnce("ZCom_Control::ZCom_setDiscoverListener",
        "LAN discover-request listening is not implemented");
    return false;
}

void ZCom_Control::ZCom_setControlID(zU8 _id) { m_priv->control_id = _id; }

void ZCom_Control::ZCom_setUpstreamLimit(zU32 _total_bps, zU32 _perconn_bps) {
    (void) _perconn_bps; // ENet only exposes a whole-host bandwidth limit
    if (m_priv->host) {
        enet_host_bandwidth_limit(m_priv->host, 0, _total_bps);
    }
}

ZCom_ClassID ZCom_Control::ZCom_registerClass(const char* _name, zU32 _class_flags) {
    (void) _class_flags;
    if (!_name) return ZCom_Invalid_ID;
    std::string name(_name);
    if (m_priv->class_ids.find(name) != m_priv->class_ids.end()) {
        // Real ZoidCom requires class names to be unique per ZCom_Control.
        zshim::logf("ZCom_Control[%s]::ZCom_registerClass: class '%s' already registered",
                     m_priv->debug_name.c_str(), name.c_str());
        return ZCom_Invalid_ID;
    }
    ZCom_ClassID id = (ZCom_ClassID) m_priv->class_names.size();
    m_priv->class_names.push_back(name);
    m_priv->class_ids[name] = id;
    return id;
}

ZCom_ClassID ZCom_Control::ZCom_getClassID(const char* _name) const {
    if (!_name) return ZCom_Invalid_ID;
    std::map<std::string, ZCom_ClassID>::const_iterator it = m_priv->class_ids.find(_name);
    return it == m_priv->class_ids.end() ? ZCom_Invalid_ID : it->second;
}

// --- packet helpers --------------------------------------------------------

static void sendKind(ENetPeer* peer, zU8 kind, const char* bytes, zU16 len, bool reliable, zU8 channel = 0) {
    ENetPacket* packet = enet_packet_create(NULL, len + 1, reliable ? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNSEQUENCED);
    packet->data[0] = kind;
    if (len) memcpy(packet->data + 1, bytes, len);
    enet_peer_send(peer, channel, packet);
}

// Serializes _envelope and sends it (kind byte prefixed) to _peer, reliable
// ordered on channel 0. Used by all the Phase B node-linking/event sends
// below, which are all small enough for a fixed on-stack buffer.
static bool sendEnvelope(ENetPeer* _peer, zU8 _kind, ZCom_BitStream& _envelope) {
    char buf[4096];
    zU16 size = 0;
    if (!_envelope.Serialize(buf, &size, sizeof(buf))) return false;
    sendKind(_peer, _kind, buf, size, true, 0);
    return true;
}

// --- connect / disconnect ----------------------------------------------------

ZCom_ConnID ZCom_Control::ZCom_Connect(const ZCom_Address& _target, ZCom_BitStream* _request) {
    if (!m_priv->host) return ZCom_Invalid_ID;

    ENetAddress addr;
    addr.host = _target.getIP();
    addr.port = _target.getPort();

    ENetPeer* peer = enet_host_connect(m_priv->host, &addr, kChannelCount, 0);
    if (!peer) {
        delete _request;
        return ZCom_Invalid_ID;
    }

    ZCom_ConnID id = m_priv->next_conn_id++;
    m_priv->conn_to_peer[id] = peer;
    m_priv->peer_to_conn[peer] = id;
    m_priv->conn_addr[id] = _target;
    memset(&m_priv->stats[id], 0, sizeof(ZCom_ConnStats));

    if (_request) {
        zshim::todoPhaseBOnce("ZCom_Control::ZCom_Connect(request-data)",
            "the connection-request bitstream is not transmitted (ENet's connect handshake carries no application payload); "
            "ZCom_cbConnectionRequest() on the remote side always sees an empty request stream");
        delete _request;
    }
    return id;
}

void ZCom_Control::ZCom_requestDownstreamLimit(ZCom_ConnID _id, zU16 _pps, zU16 _bpp) {
    (void) _pps; (void) _bpp;
    std::map<ZCom_ConnID, ENetPeer*>::iterator it = m_priv->conn_to_peer.find(_id);
    if (it == m_priv->conn_to_peer.end()) return;
    zshim::todoPhaseBOnce("ZCom_Control::ZCom_requestDownstreamLimit",
        "per-connection downstream bandwidth requests are accepted but not enforced");
}

bool ZCom_Control::ZCom_Discover(const ZCom_Address& _address, ZCom_BitStream* _request) {
    (void) _address;
    delete _request;
    zshim::todoPhaseBOnce("ZCom_Control::ZCom_Discover", "LAN discovery broadcast is not implemented");
    return false;
}

bool ZCom_Control::ZCom_Disconnect(const ZCom_ConnID _id, ZCom_BitStream* _reason) {
    std::map<ZCom_ConnID, ENetPeer*>::iterator it = m_priv->conn_to_peer.find(_id);
    if (it == m_priv->conn_to_peer.end()) {
        delete _reason;
        return false;
    }
    if (_reason) {
        char buf[512];
        zU16 size = 0;
        _reason->Serialize(buf, &size, sizeof(buf));
        sendKind(it->second, kMsgDisconnectReason, buf, size, true);
        delete _reason;
    }
    enet_peer_disconnect(it->second, 0);
    return true;
}

void ZCom_Control::ZCom_disconnectAll(ZCom_BitStream* _reason) {
    char buf[512];
    zU16 size = 0;
    if (_reason) _reason->Serialize(buf, &size, sizeof(buf));

    for (std::map<ZCom_ConnID, ENetPeer*>::iterator it = m_priv->conn_to_peer.begin();
         it != m_priv->conn_to_peer.end(); ++it) {
        if (_reason) sendKind(it->second, kMsgDisconnectReason, buf, size, true);
        enet_peer_disconnect(it->second, 0);
    }
    delete _reason;
}

ZCom_ConnGroupManager& ZCom_Control::ZCom_getGroupManager() { return m_priv->group_mgr; }

bool ZCom_Control::ZCom_sendData(const ZCom_ConnID _id, ZCom_BitStream* _stream, eZCom_SendMode _mode) {
    std::map<ZCom_ConnID, ENetPeer*>::iterator it = m_priv->conn_to_peer.find(_id);
    if (it == m_priv->conn_to_peer.end()) {
        delete _stream;
        return false;
    }
    char buf[4096];
    zU16 size = 0;
    bool ok = _stream ? _stream->Serialize(buf, &size, sizeof(buf)) : true;
    delete _stream;
    if (!ok) return false;

    bool reliable = (_mode == eZCom_ReliableOrdered || _mode == eZCom_ReliableUnordered);
    ENetPacket* packet = enet_packet_create(NULL, size + 1, reliable ? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNSEQUENCED);
    packet->data[0] = kMsgRawData;
    if (size) memcpy(packet->data + 1, buf, size);

    // eZCom_ReliableOrdered uses channel 0 (ENet channels are ordered by
    // default), eZCom_ReliableUnordered/Unreliable use channel 1.
    zU8 channel = (_mode == eZCom_ReliableOrdered) ? 0 : 1;
    return enet_peer_send(it->second, channel, packet) == 0;
}

bool ZCom_Control::ZCom_sendDataToGroup(const ZCom_GroupID _gid, ZCom_BitStream* _stream, eZCom_SendMode _mode) {
    if (_gid == ZCOM_CONNGROUP_ALL) {
        bool any_ok = false;
        char buf[4096];
        zU16 size = 0;
        bool ok = _stream ? _stream->Serialize(buf, &size, sizeof(buf)) : true;
        delete _stream;
        if (!ok) return false;
        bool reliable = (_mode == eZCom_ReliableOrdered || _mode == eZCom_ReliableUnordered);
        zU8 channel = (_mode == eZCom_ReliableOrdered) ? 0 : 1;
        for (std::map<ZCom_ConnID, ENetPeer*>::iterator it = m_priv->conn_to_peer.begin();
             it != m_priv->conn_to_peer.end(); ++it) {
            ENetPacket* packet = enet_packet_create(NULL, size + 1, reliable ? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNSEQUENCED);
            packet->data[0] = kMsgRawData;
            if (size) memcpy(packet->data + 1, buf, size);
            if (enet_peer_send(it->second, channel, packet) == 0) any_ok = true;
        }
        return any_ok;
    }
    zshim::todoPhaseBOnce("ZCom_Control::ZCom_sendDataToGroup", "custom connection groups are not populated by anything yet");
    delete _stream;
    return false;
}

bool ZCom_Control::ZCom_sendDataRaw(ZCom_Address& _dest, void* _data, zU32 _size) {
    if (!m_priv->host) return false;
    ENetAddress addr;
    addr.host = _dest.getIP();
    addr.port = _dest.getPort();
    ENetBuffer buf;
    buf.data = _data;
    buf.dataLength = _size;
    // ENet doesn't expose a "send one datagram to arbitrary address" call
    // outside of enet_socket_send on the raw socket; use that directly.
    return enet_socket_send(m_priv->host->socket, &addr, &buf, 1) >= 0;
}

bool ZCom_Control::ZCom_requestZoidMode(const ZCom_ConnID _id, zU8 _level) {
    (void) _id; (void) _level;
    zshim::todoPhaseBOnce("ZCom_Control::ZCom_requestZoidMode", "Zoidlevel migration is not implemented");
    return false;
}

ZCom_Node* ZCom_Control::ZCom_getNode(ZCom_NodeID _nid) const {
    return ZCom_shimFindNetId(_nid);
}

const ZCom_Address* ZCom_Control::ZCom_getPeer(ZCom_ConnID _id) const {
    std::map<ZCom_ConnID, ZCom_Address>::const_iterator it = m_priv->conn_addr.find(_id);
    return it == m_priv->conn_addr.end() ? NULL : &it->second;
}

const ZCom_ConnStats& ZCom_Control::ZCom_getConnectionStats(ZCom_ConnID _id) const {
    std::map<ZCom_ConnID, ENetPeer*>::const_iterator peer_it = m_priv->conn_to_peer.find(_id);
    if (peer_it == m_priv->conn_to_peer.end()) return m_priv->zero_stats;

    ZCom_ConnStats& s = m_priv->stats[_id];
    ENetPeer* peer = peer_it->second;
    s.ping = (zU16) peer->roundTripTime;
    s.avg_ping = (zU16) peer->roundTripTime;
    s.last_sec_loss_percent = (zU8) (peer->packetLoss / 100 > 100 ? 100 : peer->packetLoss / 100);
    s.total_out = peer->packetsSent;
    s.last_sec_loss_count = 0;
    s.current_loss_count = (zU8) (peer->packetsLost > 255 ? 255 : peer->packetsLost);
    return s;
}

void ZCom_Control::ZCom_setUserData(ZCom_ConnID _id, void* _data) { m_priv->user_data[_id] = _data; }
void* ZCom_Control::ZCom_getUserData(ZCom_ConnID _id) const {
    std::map<ZCom_ConnID, void*>::const_iterator it = m_priv->user_data.find(_id);
    return it == m_priv->user_data.end() ? NULL : it->second;
}

zU32 ZCom_Control::ZCom_getCurrentTime() { return zshim::currentTimeMillis(); }

void ZCom_Control::ZCom_simulateLag(ZCom_ConnID _id, zU32 _lagmsec) {
    (void) _id; (void) _lagmsec;
    zshim::todoPhaseBOnce("ZCom_Control::ZCom_simulateLag", "artificial lag simulation is not implemented");
}

void ZCom_Control::ZCom_simulateLoss(ZCom_ConnID _id, zFloat _amount) {
    (void) _id; (void) _amount;
    zshim::todoPhaseBOnce("ZCom_Control::ZCom_simulateLoss", "artificial packet loss simulation is not implemented");
}

// --- ZCom_createBitStream / ZCom_deleteBitStream (deprecated helpers) -------

ZCom_BitStream* ZCom_Control::ZCom_createBitStream() { return new ZCom_BitStream(); }
void ZCom_Control::ZCom_deleteBitStream(ZCom_BitStream* _bs) { delete _bs; }

// --- Phase B: node registry / linking / event wire sends --------------------

ZCom_NodeID ZCom_Control::ZCom_shimAllocNetworkId() {
    return m_priv->next_network_id++;
}

void ZCom_Control::ZCom_shimBindNetId(ZCom_NodeID _netid, ZCom_Node* _node) {
    m_priv->nodes_by_netid[_netid] = _node;
}

void ZCom_Control::ZCom_shimUnbindNetId(ZCom_NodeID _netid) {
    m_priv->nodes_by_netid.erase(_netid);
}

ZCom_Node* ZCom_Control::ZCom_shimFindNetId(ZCom_NodeID _netid) const {
    std::map<ZCom_NodeID, ZCom_Node*>::const_iterator it = m_priv->nodes_by_netid.find(_netid);
    return it == m_priv->nodes_by_netid.end() ? NULL : it->second;
}

void ZCom_Control::ZCom_shimBindUniqueClass(ZCom_ClassID _classid, ZCom_Node* _node) {
    m_priv->unique_nodes_by_class[_classid] = _node;
}

void ZCom_Control::ZCom_shimUnbindUniqueClass(ZCom_ClassID _classid, ZCom_Node* _node) {
    std::map<ZCom_ClassID, ZCom_Node*>::iterator it = m_priv->unique_nodes_by_class.find(_classid);
    if (it != m_priv->unique_nodes_by_class.end() && it->second == _node) {
        m_priv->unique_nodes_by_class.erase(it);
    }
}

ZCom_Node* ZCom_Control::ZCom_shimFindUniqueClass(ZCom_ClassID _classid) const {
    std::map<ZCom_ClassID, ZCom_Node*>::const_iterator it = m_priv->unique_nodes_by_class.find(_classid);
    return it == m_priv->unique_nodes_by_class.end() ? NULL : it->second;
}

std::vector<ZCom_ConnID> ZCom_Control::ZCom_shimAllConnections() const {
    std::vector<ZCom_ConnID> conns;
    conns.reserve(m_priv->conn_to_peer.size());
    for (std::map<ZCom_ConnID, ENetPeer*>::const_iterator it = m_priv->conn_to_peer.begin();
         it != m_priv->conn_to_peer.end(); ++it) {
        conns.push_back(it->first);
    }
    return conns;
}

eZCom_NodeRole ZCom_Control::ZCom_shimRegisterDynamicNode(ZCom_Node* _node) {
    if (m_priv->dispatching_dynamic_request) {
        _node->ZCom_shimBindSelf(m_priv->pending_dynamic_netid, m_priv->pending_dynamic_conn);
        m_priv->nodes_by_netid[m_priv->pending_dynamic_netid] = _node;
        return eZCom_RoleProxy;
    }
    ZCom_NodeID netid = m_priv->next_network_id++;
    _node->ZCom_shimBindSelf(netid, ZCom_Invalid_ID);
    m_priv->nodes_by_netid[netid] = _node;
    return eZCom_RoleAuthority;
}

void ZCom_Control::ZCom_shimNotePendingFlush(ZCom_Node* _node) {
    m_priv->pending_flush_nodes.insert(_node);
}

void ZCom_Control::ZCom_shimForgetPendingFlush(ZCom_Node* _node) {
    m_priv->pending_flush_nodes.erase(_node);
}

void ZCom_Control::ZCom_shimSendNodeCreate(ZCom_ConnID _conn, ZCom_ClassID _classid, ZCom_NodeID _netid,
                                            eZCom_NodeRole _role, ZCom_BitStream* _announce_data) {
    std::map<ZCom_ConnID, ENetPeer*>::iterator it = m_priv->conn_to_peer.find(_conn);
    if (it == m_priv->conn_to_peer.end()) return;

    ZCom_BitStream envelope;
    envelope.addInt(_classid, 32);
    envelope.addInt(_netid, 32);
    envelope.addBool(_role == eZCom_RoleOwner);
    zU32 bits = _announce_data ? _announce_data->getBitCount() : 0;
    envelope.addInt(bits, 32);
    if (bits) envelope.addBitStream(_announce_data, true);

    sendEnvelope(it->second, kMsgNodeCreate, envelope);
}

void ZCom_Control::ZCom_shimSendNodeLinkUnique(ZCom_ConnID _conn, ZCom_ClassID _classid, ZCom_NodeID _netid) {
    std::map<ZCom_ConnID, ENetPeer*>::iterator it = m_priv->conn_to_peer.find(_conn);
    if (it == m_priv->conn_to_peer.end()) return;

    ZCom_BitStream envelope;
    envelope.addInt(_classid, 32);
    envelope.addInt(_netid, 32);

    sendEnvelope(it->second, kMsgNodeLinkUnique, envelope);
}

void ZCom_Control::ZCom_shimSendNodeOwner(ZCom_ConnID _conn, ZCom_NodeID _netid, bool _enabled) {
    std::map<ZCom_ConnID, ENetPeer*>::iterator it = m_priv->conn_to_peer.find(_conn);
    if (it == m_priv->conn_to_peer.end()) return;

    ZCom_BitStream envelope;
    envelope.addInt(_netid, 32);
    envelope.addBool(_enabled);

    sendEnvelope(it->second, kMsgNodeOwner, envelope);
}

void ZCom_Control::ZCom_shimSendNodeRemove(ZCom_ConnID _conn, ZCom_NodeID _netid) {
    std::map<ZCom_ConnID, ENetPeer*>::iterator it = m_priv->conn_to_peer.find(_conn);
    if (it == m_priv->conn_to_peer.end()) return;

    ZCom_BitStream envelope;
    envelope.addInt(_netid, 32);

    sendEnvelope(it->second, kMsgNodeRemove, envelope);
}

bool ZCom_Control::ZCom_shimSendNodeEvent(ZCom_ConnID _conn, ZCom_NodeID _netid, eZCom_SendMode _mode, ZCom_BitStream* _data) {
    std::map<ZCom_ConnID, ENetPeer*>::iterator it = m_priv->conn_to_peer.find(_conn);
    if (it == m_priv->conn_to_peer.end()) return false;

    ZCom_BitStream envelope;
    envelope.addInt(_netid, 32);
    envelope.addInt(zshim::currentTimeMillis(), 32);
    zU32 bits = _data ? _data->getBitCount() : 0;
    envelope.addInt(bits, 32);
    if (bits) envelope.addBitStream(_data, true);

    char buf[4096];
    zU16 size = 0;
    if (!envelope.Serialize(buf, &size, sizeof(buf))) return false;

    bool reliable = (_mode != eZCom_Unreliable);
    zU8 channel = (_mode == eZCom_ReliableOrdered) ? 0 : 1;
    ENetPacket* packet = enet_packet_create(NULL, size + 1, reliable ? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNSEQUENCED);
    packet->data[0] = kMsgNodeEvent;
    if (size) memcpy(packet->data + 1, buf, size);
    return enet_peer_send(it->second, channel, packet) == 0;
}

// --- the actual ENet pump ----------------------------------------------------

void ZCom_Control::ZCom_processInput(eZCom_BlockMode _block) {
    if (!m_priv->host) return;

    ENetEvent event;
    zU32 timeout = (_block == eZCom_Block) ? 50 : 0;

    while (enet_host_service(m_priv->host, &event, timeout) > 0) {
        timeout = 0; // only the first wait (if any) should block

        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT: {
                std::map<ENetPeer*, ZCom_ConnID>::iterator existing = m_priv->peer_to_conn.find(event.peer);
                if (existing != m_priv->peer_to_conn.end()) {
                    // Our own outgoing ZCom_Connect() completed.
                    ZCom_BitStream reply;
                    ZCom_cbConnectResult(existing->second, eZCom_ConnAccepted, reply);
                } else {
                    // Unsolicited incoming connection.
                    ZCom_ConnID id = m_priv->next_conn_id++;
                    m_priv->conn_to_peer[id] = event.peer;
                    m_priv->peer_to_conn[event.peer] = id;

                    ZCom_Address addr;
                    addr.setType(eZCom_AddressUDP);
                    addr.setIP(event.peer->address.host);
                    addr.setPort(event.peer->address.port);
                    m_priv->conn_addr[id] = addr;
                    memset(&m_priv->stats[id], 0, sizeof(ZCom_ConnStats));

                    ZCom_BitStream request, reply;
                    bool accept = ZCom_cbConnectionRequest(id, request, reply);
                    if (accept) {
                        // Phase B: link any locally-registered authority
                        // *unique* nodes to this newly-accepted connection
                        // BEFORE notifying the game via
                        // ZCom_cbConnectionSpawned() -- so that if the game
                        // synchronously calls setOwner() in response (e.g.
                        // Lobby::onConnect() granting admin), the connection
                        // is already linked and the promotion isn't silently
                        // dropped. See phase-b-replication.md §3.1/§6.1.
                        for (std::map<ZCom_ClassID, ZCom_Node*>::iterator uit = m_priv->unique_nodes_by_class.begin();
                             uit != m_priv->unique_nodes_by_class.end(); ++uit) {
                            if (uit->second->getRole() == eZCom_RoleAuthority) {
                                uit->second->ZCom_shimQueueAnnounce(id);
                            }
                        }
                        ZCom_cbConnectionSpawned(id);
                    } else {
                        m_priv->conn_to_peer.erase(id);
                        m_priv->peer_to_conn.erase(event.peer);
                        m_priv->conn_addr.erase(id);
                        enet_peer_disconnect_now(event.peer, 0);
                    }
                }
                break;
            }
            case ENET_EVENT_TYPE_RECEIVE: {
                std::map<ENetPeer*, ZCom_ConnID>::iterator conn_it = m_priv->peer_to_conn.find(event.peer);
                if (conn_it != m_priv->peer_to_conn.end() && event.packet->dataLength >= 1) {
                    zU8 kind = event.packet->data[0];
                    zU16 payload_len = (zU16) (event.packet->dataLength - 1);
                    ZCom_ConnID from_conn = conn_it->second;

                    if (kind == kMsgRawData) {
                        ZCom_BitStream data;
                        data.Deserialize((char*) event.packet->data + 1, payload_len);
                        ZCom_cbDataReceived(from_conn, data);
                    } else if (kind == kMsgDisconnectReason) {
                        std::string bytes((const char*) event.packet->data + 1, payload_len);
                        m_priv->pending_disconnect_reason[from_conn] = bytes;
                    } else if (kind == kMsgNodeCreate) {
                        ZCom_BitStream envelope;
                        envelope.Deserialize((char*) event.packet->data + 1, payload_len);
                        ZCom_ClassID classid = envelope.getInt(32);
                        ZCom_NodeID netid = envelope.getInt(32);
                        bool is_owner = envelope.getBool();
                        zU32 bits = envelope.getInt(32);
                        ZCom_BitStream* announce = bits ? envelope.getBitStream(bits, true) : NULL;

                        m_priv->pending_dynamic_netid = netid;
                        m_priv->pending_dynamic_conn = from_conn;
                        m_priv->dispatching_dynamic_request = true;
                        ZCom_cbNodeRequest_Dynamic(from_conn, classid, announce,
                            is_owner ? eZCom_RoleOwner : eZCom_RoleProxy, netid);
                        m_priv->dispatching_dynamic_request = false;

                        if (is_owner) {
                            ZCom_Node* node = ZCom_shimFindNetId(netid);
                            if (node) node->ZCom_shimSetOwnerRole(true);
                        }
                        delete announce;
                    } else if (kind == kMsgNodeLinkUnique) {
                        ZCom_BitStream envelope;
                        envelope.Deserialize((char*) event.packet->data + 1, payload_len);
                        ZCom_ClassID classid = envelope.getInt(32);
                        ZCom_NodeID netid = envelope.getInt(32);

                        ZCom_Node* node = ZCom_shimFindUniqueClass(classid);
                        if (node && node->getNetworkID() == 0) {
                            node->ZCom_shimBindSelf(netid, from_conn);
                            ZCom_shimBindNetId(netid, node);
                        }
                    } else if (kind == kMsgNodeOwner) {
                        ZCom_BitStream envelope;
                        envelope.Deserialize((char*) event.packet->data + 1, payload_len);
                        ZCom_NodeID netid = envelope.getInt(32);
                        bool enabled = envelope.getBool();

                        ZCom_Node* node = ZCom_shimFindNetId(netid);
                        if (node) node->ZCom_shimSetOwnerRole(enabled);
                    } else if (kind == kMsgNodeRemove) {
                        ZCom_BitStream envelope;
                        envelope.Deserialize((char*) event.packet->data + 1, payload_len);
                        ZCom_NodeID netid = envelope.getInt(32);

                        ZCom_Node* node = ZCom_shimFindNetId(netid);
                        if (node) {
                            node->ZCom_shimDeliverRemove(from_conn);
                            ZCom_shimUnbindNetId(netid);
                        }
                    } else if (kind == kMsgNodeEvent) {
                        ZCom_BitStream envelope;
                        envelope.Deserialize((char*) event.packet->data + 1, payload_len);
                        ZCom_NodeID netid = envelope.getInt(32);
                        zU32 sent_time = envelope.getInt(32);
                        zU32 bits = envelope.getInt(32);
                        ZCom_BitStream* payload = bits ? envelope.getBitStream(bits, true) : new ZCom_BitStream();

                        ZCom_Node* node = ZCom_shimFindNetId(netid);
                        if (node) {
                            eZCom_NodeRole remote_role = (node->getRole() == eZCom_RoleAuthority)
                                ? node->ZCom_shimRemoteRoleFor(from_conn)
                                : eZCom_RoleAuthority;
                            node->ZCom_shimDeliverEvent(eZCom_EventUser, remote_role, from_conn, payload, sent_time);
                        } else {
                            delete payload;
                        }
                    } else {
                        zshim::todoPhaseBOnce("ZCom_Control::ZCom_processInput(unknown-kind)",
                            "received a message kind this shim doesn't recognize");
                    }
                }
                enet_packet_destroy(event.packet);
                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT: {
                std::map<ENetPeer*, ZCom_ConnID>::iterator conn_it = m_priv->peer_to_conn.find(event.peer);
                if (conn_it != m_priv->peer_to_conn.end()) {
                    ZCom_ConnID id = conn_it->second;
                    ZCom_BitStream reason;
                    std::map<ZCom_ConnID, std::string>::iterator reason_it = m_priv->pending_disconnect_reason.find(id);
                    if (reason_it != m_priv->pending_disconnect_reason.end()) {
                        reason.Deserialize(&reason_it->second[0], (zU16) reason_it->second.size());
                        m_priv->pending_disconnect_reason.erase(reason_it);
                    }

                    // Phase B: tell every node registered on this control
                    // that _id is gone (drives eZCom_EventRemoved -- see
                    // ZCom_Node::ZCom_shimNoteConnectionClosed()). Copy the
                    // node list first since these calls don't touch the map
                    // but this keeps iteration safe regardless.
                    std::vector<ZCom_Node*> nodes;
                    nodes.reserve(m_priv->nodes_by_netid.size());
                    for (std::map<ZCom_NodeID, ZCom_Node*>::iterator nit = m_priv->nodes_by_netid.begin();
                         nit != m_priv->nodes_by_netid.end(); ++nit) {
                        nodes.push_back(nit->second);
                    }
                    for (size_t i = 0; i < nodes.size(); i++) nodes[i]->ZCom_shimNoteConnectionClosed(id);

                    ZCom_cbConnectionClosed(id, eZCom_ClosedDisconnect, reason);

                    m_priv->conn_to_peer.erase(id);
                    m_priv->peer_to_conn.erase(event.peer);
                    m_priv->conn_addr.erase(id);
                    m_priv->user_data.erase(id);
                    m_priv->stats.erase(id);
                }
                break;
            }
            default:
                break;
        }
    }
}

void ZCom_Control::ZCom_processReplicators(zU32 _simulation_time_passed) {
    (void) _simulation_time_passed;
    zshim::todoPhaseBOnce("ZCom_Control::ZCom_processReplicators",
        "no live replication tick yet: registered ZCom_ReplicatorBasic/Advanced instances are never polled "
        "(checkState()/packData()/unpackData()/Process() are not called) -- this is step 3, out of scope for "
        "the node-linking/event-delivery pass");
}

void ZCom_Control::ZCom_processOutput() {
    // Phase B: flush any deferred node announcements (NODE_CREATE /
    // NODE_LINK_UNIQUE), now that any setOwner() calls made since they were
    // queued have had a chance to land -- see
    // ZCom_Node::ZCom_shimFlushPendingAnnouncements() and this file's header.
    if (!m_priv->pending_flush_nodes.empty()) {
        std::vector<ZCom_Node*> nodes(m_priv->pending_flush_nodes.begin(), m_priv->pending_flush_nodes.end());
        m_priv->pending_flush_nodes.clear();
        for (size_t i = 0; i < nodes.size(); i++) {
            nodes[i]->ZCom_shimFlushPendingAnnouncements();
        }
    }

    if (m_priv->host) enet_host_flush(m_priv->host);
}
