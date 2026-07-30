// ZoidCom-compatible shim: ZCom_ConnGroupManager.
// TODO(phaseB): trivial bookkeeping only; not wired to ZCom_Control's
// connection list or ZCom_sendDataToGroup(). See zoidcom_conngroup.h.
#include "zoidcom_shim_internal.h"

ZCom_ConnGroupManager::ZCom_ConnGroupManager() : m_next_id(1) {}
ZCom_ConnGroupManager::~ZCom_ConnGroupManager() {}

ZCom_GroupID ZCom_ConnGroupManager::createGroup() {
    ZCom_GroupID id = m_next_id++;
    m_groups[id] = std::vector<ZCom_ConnID>();
    return id;
}

void ZCom_ConnGroupManager::destroyGroup(ZCom_GroupID _gid) {
    m_groups.erase(_gid);
}

bool ZCom_ConnGroupManager::addToGroup(ZCom_GroupID _gid, ZCom_ConnID _conn) {
    std::map<ZCom_GroupID, std::vector<ZCom_ConnID> >::iterator it = m_groups.find(_gid);
    if (it == m_groups.end()) return false;
    it->second.push_back(_conn);
    return true;
}

bool ZCom_ConnGroupManager::removeFromGroup(ZCom_GroupID _gid, ZCom_ConnID _conn) {
    std::map<ZCom_GroupID, std::vector<ZCom_ConnID> >::iterator it = m_groups.find(_gid);
    if (it == m_groups.end()) return false;
    std::vector<ZCom_ConnID>& v = it->second;
    for (size_t i = 0; i < v.size(); i++) {
        if (v[i] == _conn) { v.erase(v.begin() + i); return true; }
    }
    return false;
}

const std::vector<ZCom_ConnID>& ZCom_ConnGroupManager::getMembers(ZCom_GroupID _gid) const {
    std::map<ZCom_GroupID, std::vector<ZCom_ConnID> >::const_iterator it = m_groups.find(_gid);
    if (it == m_groups.end()) return m_empty;
    return it->second;
}
