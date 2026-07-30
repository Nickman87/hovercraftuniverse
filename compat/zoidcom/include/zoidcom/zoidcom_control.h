/****************************************
* zoidcom_control.h
* network control class -- ZoidCom-compatible shim
*
* Real ENet transport underneath (see compat/zoidcom/src/Control.cpp):
* socket setup, connect/listen/disconnect, and raw ZCom_sendData()/
* ZCom_cbDataReceived() are implemented and functional. Class ID
* registration is real (simple name->id table).
*
* Real (Phase B, steps 1-2 -- see docs/porting/phase-b-replication.md):
* ZCom_getNode() (backed by a real per-control node registry), and the
* wire-level dispatch of NODE_CREATE/NODE_LINK_UNIQUE/NODE_OWNER/
* NODE_REMOVE/NODE_EVENT messages inside ZCom_processInput(), plus their
* deferred-announcement flush inside ZCom_processOutput(). The node
* registry and network-id space are members of ZCom_Control_Private --
* i.e. scoped per ZCom_Control instance, never static/file-global. This
* matters because single-player runs the server and client as two
* ZCom_Control instances in the SAME PROCESS (see phase-b-replication.md
* §6.1); a shared registry would let a client resolve a network id to the
* server's own node object, which would work by accident in single-player
* and fail over a real network.
*
* Zoidlevel / discovery / bandwidth-shaping / lag-simulation remain honest
* TODO(phaseB) stubs -- see docs/porting/zoidcom-compat.md.
*****************************************/

#ifndef _ZOIDCONTROL_H_
#define _ZOIDCONTROL_H_

#include "zoidcom.h"
#include <vector>

class ZCom_Control_Private;
class ZCom_ConnGroupManager;
class ZCom_BitStream;
class ZCom_Address;
class ZCom_Node;

/**
 * @name Class Flags
 * Flags used with ZCom_Control::ZCom_registerClass().
 */
#define ZCOM_CLASSFLAG_ANNOUNCEDATA   (1L << 0)

/** @brief Network host. */
class ZCOM_API ZCom_Control
{
protected:
  ZCom_Control_Private *m_priv;

public:
  ZCom_Control( void );
  virtual ~ZCom_Control( void );

  void Shutdown( void );

  void ZCom_setDebugName( const char *_name );

  bool ZCom_initSockets( bool _useudp, zU16 _udpport, zU16 _localport, zU8 _control_id_size = 0);

  bool ZCom_setDiscoverListener(  eZCom_DiscoverOpt _opt, zU16 _discoverport );

  void ZCom_setControlID( zU8 _id );

  void ZCom_setUpstreamLimit( zU32 _total_bps, zU32 _perconn_bps );

  ZCom_ClassID ZCom_registerClass( const char *_name , zU32 _class_flags = 0);
  ZCom_ClassID ZCom_getClassID( const char *_name ) const;

  void ZCom_processInput( eZCom_BlockMode _block = eZCom_NoBlock );
  void ZCom_processReplicators( zU32 _simulation_time_passed );
  void ZCom_processOutput();

  static ZCom_BitStream* ZCom_createBitStream();
  static void ZCom_deleteBitStream(ZCom_BitStream* _bs);

  ZCom_ConnID ZCom_Connect( const ZCom_Address &_target, ZCom_BitStream *_request );
  void ZCom_requestDownstreamLimit( ZCom_ConnID _id, zU16 _pps, zU16 _bpp );
  bool ZCom_Discover( const ZCom_Address &_address , ZCom_BitStream *_request);
  bool ZCom_Disconnect( const ZCom_ConnID  _id, ZCom_BitStream *_reason );
  void ZCom_disconnectAll( ZCom_BitStream *_reason );

  ZCom_ConnGroupManager& ZCom_getGroupManager();

  bool ZCom_sendData( const ZCom_ConnID _id, ZCom_BitStream *_stream, eZCom_SendMode _mode = eZCom_ReliableOrdered );
  bool ZCom_sendDataToGroup( const ZCom_GroupID _gid, ZCom_BitStream *_stream, eZCom_SendMode _mode );
  bool ZCom_sendDataRaw( ZCom_Address& _dest, void* _data, zU32 _size);

  bool ZCom_requestZoidMode( const ZCom_ConnID _id, zU8 _level );

  ZCom_Node* ZCom_getNode( ZCom_NodeID _nid ) const;
  const ZCom_Address* ZCom_getPeer( ZCom_ConnID _id ) const;
  const ZCom_ConnStats& ZCom_getConnectionStats( ZCom_ConnID _id ) const;

  void ZCom_setUserData( ZCom_ConnID _id, void *_data );
  void* ZCom_getUserData( ZCom_ConnID _id ) const;

  static zU32 ZCom_getCurrentTime();

  void ZCom_simulateLag( ZCom_ConnID _id, zU32 _lagmsec );
  void ZCom_simulateLoss( ZCom_ConnID _id, zFloat _amount );

  /******************************************************/
  /*                     callbacks                      */

  virtual void ZCom_cbConnectResult( ZCom_ConnID _id, eZCom_ConnectResult _result, ZCom_BitStream &_reply ) = 0;
  virtual bool ZCom_cbConnectionRequest( ZCom_ConnID  _id, ZCom_BitStream &_request, ZCom_BitStream &_reply ) = 0;
  virtual void ZCom_cbConnectionSpawned( ZCom_ConnID _id ) = 0;
  virtual void ZCom_cbConnectionClosed( ZCom_ConnID _id, eZCom_CloseReason _reason, ZCom_BitStream &_reasondata ) = 0;
  virtual bool ZCom_cbZoidRequest( ZCom_ConnID _id, zU8 _requested_level, ZCom_BitStream &_reason ) = 0;
  virtual void ZCom_cbZoidResult( ZCom_ConnID _id, eZCom_ZoidResult _result, zU8 _new_level, ZCom_BitStream &_reason ) = 0;
  virtual void ZCom_cbNodeRequest_Dynamic( ZCom_ConnID _id, ZCom_ClassID _requested_class, ZCom_BitStream *_announcedata,
                                           eZCom_NodeRole _role, ZCom_NodeID _net_id ) = 0;
  virtual void ZCom_cbNodeRequest_Tag( ZCom_ConnID _id, ZCom_ClassID _requested_class, ZCom_BitStream *_announcedata,
                                       eZCom_NodeRole _role, zU32 _tag ) = 0;
  virtual void ZCom_cbDataReceived( ZCom_ConnID _id, ZCom_BitStream &_data ) = 0;
  virtual bool ZCom_cbDiscoverRequest( const ZCom_Address &_addr, ZCom_BitStream &_request, ZCom_BitStream &_reply) = 0;
  virtual void ZCom_cbDiscovered( const ZCom_Address & _addr, ZCom_BitStream &_reply ) = 0;

  /* ---------------------------------------------------------------------
   * Phase B shim-internal hooks. NOT part of the real ZoidCom API -- this
   * control's own node registry / network-id space / ENet peer map, used
   * only by compat/zoidcom/src/Node.cpp so ZCom_Node can drive cross-
   * network linking and event delivery without reaching into
   * ZCom_Control_Private directly. Game code must never call these. See
   * the file header above and docs/porting/phase-b-replication.md.
   * ------------------------------------------------------------------ */

  /// Allocate the next network id from this control's own id space.
  ZCom_NodeID ZCom_shimAllocNetworkId();

  /// Bind/unbind/find a node by network id in this control's registry.
  void ZCom_shimBindNetId( ZCom_NodeID _netid, ZCom_Node* _node );
  void ZCom_shimUnbindNetId( ZCom_NodeID _netid );
  ZCom_Node* ZCom_shimFindNetId( ZCom_NodeID _netid ) const;

  /// Bind/unbind/find a locally-registered *unique* node by class id (used
  /// to link unique nodes across a connection by class id, see
  /// phase-b-replication.md §3.1).
  void ZCom_shimBindUniqueClass( ZCom_ClassID _classid, ZCom_Node* _node );
  void ZCom_shimUnbindUniqueClass( ZCom_ClassID _classid, ZCom_Node* _node );
  ZCom_Node* ZCom_shimFindUniqueClass( ZCom_ClassID _classid ) const;

  /// All currently-connected peer connection ids on this control.
  std::vector<ZCom_ConnID> ZCom_shimAllConnections() const;

  /// Called by ZCom_Node::registerNodeDynamic(). Determines the correct
  /// role (eZCom_RoleProxy if called from inside
  /// ZCom_cbNodeRequest_Dynamic(), eZCom_RoleAuthority otherwise -- see
  /// zoidcom-original-semantics.md §3) and binds the node's network id
  /// (self-allocated for authority, taken from the in-flight NODE_CREATE
  /// for proxy) into this control's registry.
  eZCom_NodeRole ZCom_shimRegisterDynamicNode( ZCom_Node* _node );

  /// Track/forget a node that has entries in its deferred-announcement
  /// queue, so ZCom_processOutput() knows which nodes to flush.
  void ZCom_shimNotePendingFlush( ZCom_Node* _node );
  void ZCom_shimForgetPendingFlush( ZCom_Node* _node );

  /// Wire sends for node linking/ownership/removal/events.
  void ZCom_shimSendNodeCreate( ZCom_ConnID _conn, ZCom_ClassID _classid, ZCom_NodeID _netid,
    eZCom_NodeRole _role, ZCom_BitStream* _announce_data );
  void ZCom_shimSendNodeLinkUnique( ZCom_ConnID _conn, ZCom_ClassID _classid, ZCom_NodeID _netid );
  void ZCom_shimSendNodeOwner( ZCom_ConnID _conn, ZCom_NodeID _netid, bool _enabled );
  void ZCom_shimSendNodeRemove( ZCom_ConnID _conn, ZCom_NodeID _netid );
  bool ZCom_shimSendNodeEvent( ZCom_ConnID _conn, ZCom_NodeID _netid, eZCom_SendMode _mode, ZCom_BitStream* _data );

  /* ---------------------------------------------------------------------
   * Phase B steps 3-4: wire sends for the replication tick. NOT part of
   * the real ZoidCom API. See docs/porting/phase-b-replication.md and this
   * file's header.
   * ------------------------------------------------------------------ */

  /// Sends a pre-built primitive/ZCom_ReplicatorBasic item batch (see
  /// ZCom_Node::ZCom_shimTickReplication()) to _conn, wrapped with _netid
  /// and a send timestamp, reliable or unreliable per _reliable.
  bool ZCom_shimSendNodeReplBatch( ZCom_ConnID _conn, ZCom_NodeID _netid, bool _reliable, ZCom_BitStream& _payload );

  /// Sends one ZCom_ReplicatorAdvanced::sendData()/sendDataDirect() payload
  /// to _conn, wrapped with _netid, _item_index (this node's
  /// replication_items position of the sending replicator), a send
  /// timestamp and _reference_id, at the reliability _mode implies.
  bool ZCom_shimSendNodeReplAdvanced( ZCom_ConnID _conn, ZCom_NodeID _netid, zU16 _item_index,
    eZCom_SendMode _mode, ZCom_BitStream* _stream, zU32 _reference_id );
};

#endif
