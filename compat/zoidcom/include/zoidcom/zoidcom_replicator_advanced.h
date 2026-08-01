/****************************************
* zoidcom_replicator_advanced.h
* ZoidCom-compatible shim
*
* Real (Phase B, step 4): sendData()/sendDataDirect() route through
* ZCom_Node::ZCom_shimSendAdvancedData() for real delivery; onDataReceived()
* is driven by ZCom_Node::ZCom_shimDeliverReplAdvanced(). See
* docs/porting/phase-b-replication.md and compat/zoidcom/src/Node.cpp's
* file header.
*
* TODO(phaseB): getLastUpdateTime() stays a stub -- unused by the one
* concrete ZCom_ReplicatorAdvanced in this codebase (see Replicator.cpp).
*****************************************/

#ifndef _ZOIDREPLICATORADVANCED_H_
#define _ZOIDREPLICATORADVANCED_H_

#include "zoidcom.h"

class ZCom_Node;

class ZCOM_API ZCom_ReplicatorAdvanced : public ZCom_Replicator
{
protected:
  ZCom_Node* m_node;
public:
  ZCom_ReplicatorAdvanced(ZCom_ReplicatorSetup *_setup);

  ZCom_Node* getNode() const;

  zU32* getLastUpdateTime(ZCom_ConnID _cid);

  void sendData(eZCom_SendMode _mode, ZCom_BitStream *_stream, zU32 _reference_id = 0);
  void sendDataDirect(eZCom_SendMode _mode, ZCom_ConnID _dest, ZCom_BitStream *_stream, zU32 _reference_id = 0);

  virtual void onPreSendData(ZCom_ConnID _cid, eZCom_NodeRole _remoterole, zU32 *_lastupdate) = 0;
  virtual void onDataReceived(ZCom_ConnID _cid, eZCom_NodeRole _remoterole, ZCom_BitStream &_stream, bool _store,
                                       zU32 _estimated_time_sent) = 0;

  virtual void onDataAcked(ZCom_ConnID _cid, zU32 _reference_id, ZCom_BitStream *_data) = 0;
  virtual void onDataLost(ZCom_ConnID _cid, zU32 _reference_id, ZCom_BitStream *_data) = 0;

  virtual void onPacketReceived(ZCom_ConnID _cid) = 0;

  virtual void onConnectionAdded(ZCom_ConnID _cid, eZCom_NodeRole _remoterole) = 0;
  virtual void onConnectionRemoved(ZCom_ConnID _cid, eZCom_NodeRole _remoterole) = 0;

  virtual void onLocalRoleChanged(eZCom_NodeRole _oldrole, eZCom_NodeRole _newrole) = 0;
  virtual void onRemoteRoleChanged(ZCom_ConnID _cid, eZCom_NodeRole _oldrole, eZCom_NodeRole _newrole) = 0;

  void setNode(ZCom_Node *_node);
};

#endif
