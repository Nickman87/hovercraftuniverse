/****************************************
* zoidcom_conngroup.h
* connection group manager -- ZoidCom-compatible shim
*
* TODO(phaseB): real ZoidCom lets applications group connections and
* address a whole group with one send call. The game code enumerated in
* docs/porting/zoidcom-compat.md does not call ZCom_getGroupManager() or
* any ZCom_ConnGroupManager method, so this is kept minimal: enough of an
* API surface for ZCom_Control::ZCom_getGroupManager()/ZCom_sendDataToGroup()
* to compile, backed by a trivial always-empty-groups implementation. Not
* wired to anything real yet.
*****************************************/

#ifndef _ZOIDCOM_CONNGROUP_H_
#define _ZOIDCOM_CONNGROUP_H_

#include "zoidcom.h"
#include <vector>
#include <map>

/// Pseudo group id meaning "all current connections".
#define ZCOM_CONNGROUP_ALL ((ZCom_GroupID)0xFFFFFFFF)

class ZCOM_API ZCom_ConnGroupManager
{
public:
  ZCom_ConnGroupManager();
  ~ZCom_ConnGroupManager();

  // TODO(phaseB): real group membership management (add/remove connection
  // to/from group, create/destroy groups). Not used by current game code.
  ZCom_GroupID createGroup();
  void destroyGroup(ZCom_GroupID _gid);
  bool addToGroup(ZCom_GroupID _gid, ZCom_ConnID _conn);
  bool removeFromGroup(ZCom_GroupID _gid, ZCom_ConnID _conn);
  const std::vector<ZCom_ConnID>& getMembers(ZCom_GroupID _gid) const;

private:
  std::map<ZCom_GroupID, std::vector<ZCom_ConnID> > m_groups;
  ZCom_GroupID m_next_id;
  std::vector<ZCom_ConnID> m_empty;
};

#endif
