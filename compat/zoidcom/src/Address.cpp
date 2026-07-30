// ZoidCom-compatible shim: ZCom_Address.
//
// Real: string/IP/port storage, "host:port" parsing, synchronous hostname
// resolution (via ENet's own resolver so the wire format used by
// Control.cpp for ENetAddress stays consistent).
// TODO(phaseB): asynchronous resolveHostname(true, ...) is not implemented
// (resolves synchronously regardless of the _async flag); LAN broadcast
// addressing (eZCom_AddressBroadcast) is accepted but ZCom_Discover() in
// Control.cpp is itself a stub.
#include "zoidcom_shim_internal.h"
#include <enet/enet.h>
#include <cstdio>
#include <cstring>
#include <string>

class ZCom_Address_Private {
public:
    eZCom_AddressType type;
    zU8 control_id;
    zU32 ip_host_order; // 0 if unresolved / hostname pending
    zU16 port;
    std::string hostname; // set if constructed from a hostname string
    eZCom_HostnameResult hostname_result;

    ZCom_Address_Private()
        : type(eZCom_AddressUDP), control_id(0), ip_host_order(0), port(0),
          hostname_result(eZCom_HostnameIdle) {}
};

namespace {
// Thread-local scratch buffers for the *Static()/toString() accessors,
// matching the real API's "returns pointer to static buffer" contract.
thread_local char tls_ip_buf[64];
thread_local char tls_hostname_buf[256];
thread_local char tls_tostring_buf[300];
}

ZCom_Address::ZCom_Address(void) : m_priv(new ZCom_Address_Private()) {}

ZCom_Address::ZCom_Address(const ZCom_Address& _o) : m_priv(new ZCom_Address_Private(*_o.m_priv)) {}

ZCom_Address::~ZCom_Address(void) { delete m_priv; }

bool ZCom_Address::setAddress(eZCom_AddressType _type, zU8 _control_id, const char* _addr) {
    m_priv->type = _type;
    m_priv->control_id = _control_id;
    if (!_addr) return false;

    // Accept "host:port" or "a.b.c.d:port".
    const char* colon = strrchr(_addr, ':');
    std::string host;
    zU16 port = 0;
    if (colon) {
        host.assign(_addr, colon - _addr);
        port = (zU16) atoi(colon + 1);
    } else {
        host = _addr;
    }
    m_priv->hostname = host;
    m_priv->port = port;

    ENetAddress ea;
    ea.port = port;
    if (enet_address_set_host(&ea, host.c_str()) == 0) {
        m_priv->ip_host_order = ea.host;
        m_priv->hostname_result = eZCom_HostnameSuccess;
        return true;
    }
    m_priv->hostname_result = eZCom_HostnameFailed;
    return false;
}

const char* ZCom_Address::getAddressIP(eZCom_GetIPAddressOption _with_port) const {
    ENetAddress ea;
    ea.host = m_priv->ip_host_order;
    ea.port = m_priv->port;
    char hostbuf[64] = {0};
    if (enet_address_get_host_ip(&ea, hostbuf, sizeof(hostbuf)) != 0) {
        strcpy(hostbuf, "0.0.0.0");
    }
    if (_with_port == eZCom_AddressWithPort) {
        snprintf(tls_ip_buf, sizeof(tls_ip_buf), "%s:%u", hostbuf, (unsigned) m_priv->port);
    } else {
        snprintf(tls_ip_buf, sizeof(tls_ip_buf), "%s", hostbuf);
    }
    return tls_ip_buf;
}

const char* ZCom_Address::getAddressHostname() const {
    if (m_priv->hostname.empty()) return NULL;
    snprintf(tls_hostname_buf, sizeof(tls_hostname_buf), "%s:%u", m_priv->hostname.c_str(), (unsigned) m_priv->port);
    return tls_hostname_buf;
}

const char* ZCom_Address::toString() const {
    const char* type_str = (m_priv->type == eZCom_AddressLocal) ? "local" :
                           (m_priv->type == eZCom_AddressTCP) ? "tcp" :
                           (m_priv->type == eZCom_AddressBroadcast) ? "broadcast" : "udp";
    if (!m_priv->hostname.empty()) {
        snprintf(tls_tostring_buf, sizeof(tls_tostring_buf), "[%s]:%s:%u", type_str, m_priv->hostname.c_str(), (unsigned) m_priv->port);
    } else {
        snprintf(tls_tostring_buf, sizeof(tls_tostring_buf), "[%s]::%u", type_str, (unsigned) m_priv->port);
    }
    return tls_tostring_buf;
}

void ZCom_Address::setIP(zU8 _a, zU8 _b, zU8 _c, zU8 _d) {
    m_priv->ip_host_order = ((zU32) _a) | ((zU32) _b << 8) | ((zU32) _c << 16) | ((zU32) _d << 24);
    m_priv->hostname.clear();
    m_priv->hostname_result = eZCom_HostnameSuccess;
}

void ZCom_Address::setIP(zU32 _ip) {
    m_priv->ip_host_order = _ip;
    m_priv->hostname.clear();
    m_priv->hostname_result = eZCom_HostnameSuccess;
}

void ZCom_Address::setPort(zU16 _port) { m_priv->port = _port; }
void ZCom_Address::setType(eZCom_AddressType _type) { m_priv->type = _type; }
void ZCom_Address::setControlID(zU8 _id) { m_priv->control_id = _id; }

zU16 ZCom_Address::getPort(void) const { return m_priv->port; }
zU32 ZCom_Address::getIP(void) const { return m_priv->ip_host_order; }
zU8 ZCom_Address::getIP(zU8 _pos) const {
    return (zU8) ((m_priv->ip_host_order >> (8 * (_pos & 3))) & 0xFF);
}
eZCom_AddressType ZCom_Address::getType(void) const { return m_priv->type; }
zU8 ZCom_Address::getControlID(void) const { return m_priv->control_id; }

bool ZCom_Address::operator==(const ZCom_Address& _c) const {
    return m_priv->type == _c.m_priv->type &&
           m_priv->control_id == _c.m_priv->control_id &&
           m_priv->ip_host_order == _c.m_priv->ip_host_order &&
           m_priv->port == _c.m_priv->port;
}

ZCom_Address& ZCom_Address::operator=(const ZCom_Address& _s) {
    if (this == &_s) return *this;
    *m_priv = *_s.m_priv;
    return *this;
}

bool ZCom_Address::resolveHostname(bool _async, zU32 _timeout) {
    (void) _timeout;
    if (_async) {
        zshim::todoPhaseBOnce("ZCom_Address::resolveHostname(async)",
            "async hostname resolution is not implemented; resolves synchronously instead");
    }
    if (m_priv->hostname.empty()) return m_priv->ip_host_order != 0;
    ENetAddress ea;
    ea.port = m_priv->port;
    bool ok = enet_address_set_host(&ea, m_priv->hostname.c_str()) == 0;
    if (ok) {
        m_priv->ip_host_order = ea.host;
        m_priv->hostname_result = eZCom_HostnameSuccess;
    } else {
        m_priv->hostname_result = eZCom_HostnameFailed;
    }
    return ok;
}

eZCom_HostnameResult ZCom_Address::checkHostname() {
    return m_priv->hostname_result;
}

zU32 ZCom_Address::computeHashKey(zU32 _max) const {
    if (_max == 0) return 0;
    zU32 h = m_priv->ip_host_order ^ ((zU32) m_priv->port << 16) ^ m_priv->control_id;
    return h % _max;
}
