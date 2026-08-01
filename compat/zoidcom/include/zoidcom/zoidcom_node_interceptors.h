/****************************************
* zoidcom_node_interceptors.h
* interceptor interface -- ZoidCom-compatible shim
*
* TODO(phaseB): these interfaces compile and can be registered
* (ZCom_Node::setEventInterceptor()/setReplicationInterceptor()), but since
* Phase A does not implement the live replication/event engine, nothing
* currently calls into an installed interceptor. Real behavior needs the
* replication tick from Phase B.
*****************************************/

#ifndef _ZOIDNODEINTERCEPT_H_
#define _ZOIDNODEINTERCEPT_H_

#include "zoidcom.h"

class ZCom_Replicator;

class ZCom_NodeEventInterceptor
{
public:
	ZCOM_API virtual ~ZCom_NodeEventInterceptor() {}

  ZCOM_API virtual bool
    recUserEvent(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remoterole, ZCom_BitStream &_data, zU32 _estimated_time_sent) = 0;

  ZCOM_API virtual bool
    recInit(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remoterole) = 0;

  ZCOM_API virtual bool
    recSyncRequest(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remoterole) = 0;

  ZCOM_API virtual bool
    recRemoved(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remoterole) = 0;

  ZCOM_API virtual bool
    recFileIncoming(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remoterole, ZCom_FileTransID _fid, ZCom_BitStream &_request) = 0;

  ZCOM_API virtual bool
    recFileData(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remoterole, ZCom_FileTransID _fid) = 0;

  ZCOM_API virtual bool
    recFileAborted(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remoterole, ZCom_FileTransID _fid) = 0;

  ZCOM_API virtual bool
    recFileComplete(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remoterole, ZCom_FileTransID _fid) = 0;
};

class ZCom_NodeReplicationInterceptor
{
public:
	ZCOM_API virtual ~ZCom_NodeReplicationInterceptor() {}

  ZCOM_API virtual void
    outPreReplicateNode(ZCom_Node *_node, ZCom_ConnID _to, eZCom_NodeRole _remote_role) = 0;

  ZCOM_API virtual void
    outPreDereplicateNode(ZCom_Node *_node, ZCom_ConnID _to, eZCom_NodeRole _remote_role) = 0;

  ZCOM_API virtual bool
    outPreUpdate(ZCom_Node *_node, ZCom_ConnID _to, eZCom_NodeRole _remote_role) = 0;

  ZCOM_API virtual bool
    outPreUpdateItem(ZCom_Node *_node, ZCom_ConnID _to, eZCom_NodeRole _remote_role, ZCom_Replicator *_replicator) = 0;

  ZCOM_API virtual void
    outPostUpdate(ZCom_Node *_node, ZCom_ConnID _to, eZCom_NodeRole _remote_role, zU32 _rep_bits, zU32 _event_bits, zU32 _meta_bits) = 0;

  ZCOM_API virtual bool
    inPreUpdate(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remote_role) = 0;

  ZCOM_API virtual bool
    inPreUpdateItem(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remote_role, ZCom_Replicator *_replicator, zU32 _estimated_time_sent) = 0;

  ZCOM_API virtual void
    inPostUpdate(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remote_role, zU32 _rep_bits, zU32 _event_bits, zU32 _meta_bits) = 0;
};

#endif
