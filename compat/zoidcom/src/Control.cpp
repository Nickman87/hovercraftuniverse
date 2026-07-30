// ZoidCom-compatible shim: ZCom_Control.
//
// Real ENet transport underneath: ZCom_initSockets()/ZCom_Connect()/
// ZCom_Disconnect()/ZCom_disconnectAll()/ZCom_processInput()/
// ZCom_processOutput() and raw ZCom_sendData()/ZCom_cbDataReceived() all
// work over an actual ENetHost. Class ID registration is a real (if
// trivial) name->id table.
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
// (ZCom_Discover/ZCom_setDiscoverListener), ZCom_getNode() (needs the node
// registry Phase B builds), lag/loss simulation.
#include "zoidcom_shim_internal.h"
#include <enet/enet.h>
#include <map>
#include <vector>
#include <string>
#include <cstring>

namespace {
const zU8 kMsgRawData = 0;
const zU8 kMsgDisconnectReason = 1;
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

    ZCom_Control_Private()
        : host(NULL), control_id(0), next_conn_id(1) {
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

static void sendKind(ENetPeer* peer, zU8 kind, const char* bytes, zU16 len, bool reliable) {
    ENetPacket* packet = enet_packet_create(NULL, len + 1, reliable ? ENET_PACKET_FLAG_RELIABLE : 0);
    packet->data[0] = kind;
    if (len) memcpy(packet->data + 1, bytes, len);
    enet_peer_send(peer, 0, packet);
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
    (void) _nid;
    zshim::todoPhaseBOnce("ZCom_Control::ZCom_getNode", "node registry lookup by network id is not implemented");
    return NULL;
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
                    if (kind == kMsgRawData) {
                        ZCom_BitStream data;
                        data.Deserialize((char*) event.packet->data + 1, payload_len);
                        ZCom_cbDataReceived(conn_it->second, data);
                    } else if (kind == kMsgDisconnectReason) {
                        std::string bytes((const char*) event.packet->data + 1, payload_len);
                        m_priv->pending_disconnect_reason[conn_it->second] = bytes;
                    } else {
                        zshim::todoPhaseBOnce("ZCom_Control::ZCom_processInput(unknown-kind)",
                            "received a message kind reserved for node/replicator traffic, which Phase A doesn't route yet");
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
        "(checkState()/packData()/unpackData()/Process() are not called)");
}

void ZCom_Control::ZCom_processOutput() {
    if (m_priv->host) enet_host_flush(m_priv->host);
}
