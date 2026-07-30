/****************************************
* zoidcom_node.h
* node object -- ZoidCom-compatible shim
*
* Real (Phase A): node identity bookkeeping (class id, role, control
* pointer), per-connection and global user data storage, replicator list
* ownership/autodelete.
*
* Real (Phase B, steps 1-2 -- see docs/porting/phase-b-replication.md):
* cross-network node linking (registerNodeDynamic/registerNodeUnique now
* actually link an authority node on one ZCom_Control to proxy/owner
* counterparts on connected ZCom_Controls, via NODE_CREATE/NODE_LINK_UNIQUE/
* NODE_OWNER/NODE_REMOVE wire messages -- see Control.cpp), setAnnounceData
* (now carries real payload, delivered with NODE_CREATE), setOwner (real
* permission gate + NODE_OWNER wire message), the node event queue
* (checkEventWaiting()/getNextEvent(), a real per-node queue),
* sendEvent()/sendEventDirect()/sendEventToGroup() (real delivery via
* NODE_EVENT, replication-rule-filtered), eZCom_EventInit (raised on the
* authority when a proxy/owner newly links, per
* docs/porting/zoidcom-original-semantics.md §1) and eZCom_EventRemoved
* (raised on connection loss / NODE_REMOVE, mandatory on the proxy/owner
* side per the same doc's §9).
*
* Real (Phase B, steps 3-4 -- see docs/porting/phase-b-replication.md and
* docs/porting/zoidcom-original-semantics.md): data replication is now
* actually driven. addReplicationInt/Bool/Float/String/StringW and
* addReplicator()-registered ZCom_ReplicatorBasic/ZCom_ReplicatorAdvanced
* instances are all polled/dispatched for real by
* ZCom_shimTickReplication(), called once per node from
* ZCom_Control::ZCom_processReplicators(); incoming data is applied by
* ZCom_shimApplyReplBatch()/ZCom_shimDeliverReplAdvanced(), called from
* ZCom_processInput(). See Node.cpp's file header for the full design,
* including where it deliberately diverges from the real library's
* documented timing (Advanced replicators' sendData()/sendDataDirect() are
* delivered immediately rather than deferred to onPreSendData()).
*
* Real (Phase B, connect-handshake-follow-up pass -- see
* docs/porting/phase-b-replication.md): dependsOn() now actually orders
* deferred announcements -- ZCom_shimGetDependencies() exposes the recorded
* edges so Control.cpp's flush can topologically sort same-cycle pending
* nodes (see ZCom_shimFlushPendingAnnouncements()/Control.cpp's
* topoOrderPendingFlush()). The replication interceptor
* (setReplicationInterceptor()) is now driven for real: outPreUpdate/
* outPreUpdateItem/outPostUpdate from ZCom_shimTickReplication(),
* outPreReplicateNode from ZCom_shimFlushPendingAnnouncements(),
* outPreDereplicateNode from disconnectAll()/ZCom_shimNoteConnectionClosed(),
* and inPreUpdate/inPreUpdateItem/inPostUpdate from the (now _from_conn/
* _remote_role-aware) ZCom_shimApplyReplBatch().
*
* Still TODO(phaseB) stubs: addInterpolationInt/Float (unused by any current
* call site), Zoidlevel membership (applyForZoidLevel/registerNodeByTag),
* mustsync/Zoidlevel authority migration, file transfer. ZCOM_REPFLAG_INTERCEPT
* on a *primitive* replication item also stays a todoPhaseBOnce (no current
* game code sets it there; only ZCom_Replicator-backed items can supply the
* ZCom_Replicator* the interceptor's *UpdateItem() callbacks require).
*
* This header also declares a handful of `ZCom_shim*` methods that are NOT
* part of the real ZoidCom API -- they exist purely so
* compat/zoidcom/src/Control.cpp (which owns the per-ZCom_Control node
* registry and the ENet wire encode/decode) can drive this node's linking/
* event-queue/role state from incoming network traffic. Game code must
* never call them.
*
* See docs/porting/zoidcom-compat.md for the Phase A rationale and
* docs/porting/phase-b-replication.md / docs/porting/
* zoidcom-original-semantics.md for the Phase B design and the real
* library's documented semantics this implementation follows.
*****************************************/

#ifndef _ZOIDNODE_H_
#define _ZOIDNODE_H_

#include "zoidcom.h"
#include <vector>

class ZCom_Node_Private;
class ZCom_Control;
class ZCom_BitStream;
class ZCom_NodeEventInterceptor;
class ZCom_NodeReplicationInterceptor;
class ZCom_Replicator;

/** \name Data replication flags (how to replicate) */
#define ZCOM_REPFLAG_NONE             0
#define ZCOM_REPFLAG_UNRELIABLE       (1L << 0)
#define ZCOM_REPFLAG_MOSTRECENT       (1L << 1)
#define ZCOM_REPFLAG_RARELYCHANGED    (1L << 2)
#define ZCOM_REPFLAG_ONLYONCE         (1L << 3)
#define ZCOM_REPFLAG_INTERCEPT        (1L << 4)
#define ZCOM_REPFLAG_SETUPPERSISTS    (1L << 5)
#define ZCOM_REPFLAG_SETUPAUTODELETE  (1L << 6)
#define ZCOM_REPFLAG_STARTCLEAN       (1L << 7)

/** \name Data & event replication rules (when to replicate) */
#define ZCOM_REPRULE_NONE             0
#define ZCOM_REPRULE_AUTH_2_PROXY     (1L << 0)
#define ZCOM_REPRULE_AUTH_2_OWNER     (1L << 1)
#define ZCOM_REPRULE_AUTH_2_ALL       (ZCOM_REPRULE_AUTH_2_PROXY|ZCOM_REPRULE_AUTH_2_OWNER)
#define ZCOM_REPRULE_OWNER_2_AUTH     (1L << 2)

/** \name Size constants for bitstream embedded data */
#define ZCOM_FTRANS_ID_BITS            32
#define ZCOM_FTRANS_SIZE_BITS          32
#define ZCOM_FTRANS_CHUNK_BITS         16

/** @brief ZCom_Node represents one networkable object. */
class ZCOM_API ZCom_Node
{
protected:
  ZCom_Node_Private *m_priv;

private:
  // Phase B internal helpers (compat/zoidcom/src/Node.cpp). Not part of the
  // real API surface -- see the ZCom_shim* methods below for what Control.cpp
  // is allowed to call from outside this class.
  void pushEvent( eZCom_Event _type, eZCom_NodeRole _remote_role, ZCom_ConnID _conn_id,
    ZCom_BitStream* _data, zU32 _estimated_time_sent );
  void noteConnectionLinkedEventInit( ZCom_ConnID _conn );

public:
  ZCom_Node( void );
  ~ZCom_Node( void );

  /* node setup */
  bool registerNodeUnique( ZCom_ClassID _classid, eZCom_NodeRole _role, ZCom_Control *_control );
  bool registerNodeByTag( ZCom_ClassID _classid, zU32 _tag, eZCom_NodeRole _role, ZCom_Control *_control );
  bool registerNodeDynamic( ZCom_ClassID _classid, ZCom_Control *_control );
  bool registerRequestedNode( ZCom_ClassID _classid, ZCom_Control *_control );
  bool unregisterNode();
  void disconnectAll();

  /* node parameters */
  void setUpdatePriority( zU16 _prio );
  void setDefaultRelevance( zFloat _default_relevance );
  void setConnectionSpecificRelevance( ZCom_ConnID _conn, zFloat _rel );
  zU32 getRelevantConnectionCount() const;
  zS32 getRelevantConnections( ZCom_ConnID *_conns, zU32 _max, zU32 *_count ) const;
  void dependsOn( ZCom_Node *_othernode, eZCom_DependencyOpt _opt = eZCom_AddDependency );
  void applyForZoidLevel( zU8 _level );
  void removeFromZoidLevel( zU8 _level );
  zU32 getZoidLevelCount( ) const;
  zU8 getZoidLevel( zU32 _index ) const;
  void setMustSync( bool _enabled, zU16 _order );
  void setSyncResult( ZCom_ConnID _conn, bool _success, ZCom_BitStream *_errormsg );
  void setSyncResultAutoSuccess( bool _enabled );
  void setOwner( ZCom_ConnID _id, bool _enabled );
  void setPrivate( bool _enabled );
  void setAnnounceData( ZCom_BitStream *_data );

  /* data replication setup */
  bool beginReplicationSetup( zU16 _replicators_max );
  void setInterceptID( ZCom_InterceptID _id );
  void addReplicationInt( zS32 *_ptr, zU8 _bits, bool _sign, zU8 _flags, zU8 _rules, zS16 _mindelay = -1, zS16 _maxdelay = -1 );
  void addReplicationBool( bool *_ptr, zU8 _flags, zU8 _rules, zS16 _mindelay = -1, zS16 _maxdelay = -1 );
  void addReplicationFloat(zFloat *_ptr, zU8 _mantissa_bits, zU8 _flags, zU8 _rules, zS16 _mindelay = -1, zS16 _maxdelay = -1);
  void addReplicationString( char *_str, zU16 _maxlen, zU8 _flags, zU8 _rules, zS16 _mindelay = -1, zS16 _maxdelay = -1);
  void addReplicationStringW( wchar_t *_str, zU16 _maxlen, zU8 _flags, zU8 _rules, zS16 _mindelay = -1, zS16 _maxdelay = -1);
  void addInterpolationInt( zS32 *_ptr, zU8 _bits, bool _sign, zU8 _flags, zU8 _rules, zS32 _treshold, zS32 *_dst = NULL,
    zS16 _mindelay = -1, zS16 _maxdelay = -1, zFloat _ipolfac = 0.4f );
  void addInterpolationFloat( zFloat *_ptr, zU8 _mantissa_bits, zU8 _flags, zU8 _rules, zFloat _treshold, zFloat *_dst = NULL,
    zS16 _mindelay = -1, zS16 _maxdelay = -1, zFloat _ipolfac = 0.4f );
  void addReplicator(ZCom_Replicator *_rep, bool _autodelete);
  bool endReplicationSetup( void );
  void setReplicationInterceptor(ZCom_NodeReplicationInterceptor *_interceptor);

  /* event replication */
  bool sendEvent( eZCom_SendMode _mode, zU8 _rules, ZCom_BitStream *_data );
  bool sendEventDirect( eZCom_SendMode _mode, ZCom_BitStream *_data, ZCom_ConnID _destconn );
  bool sendEventToGroup( eZCom_SendMode _mode, ZCom_BitStream *_data, ZCom_GroupID _destgroup );
  void setEventNotification( bool _oninit, bool _onremove );
  bool checkEventWaiting() const;
  ZCom_BitStream* getNextEvent( eZCom_Event* _type, eZCom_NodeRole* _remote_role, ZCom_ConnID* _connid, zU32* _estimated_time_sent = NULL );

  ZCom_FileTransID sendFile( const char *_path, const char *_pathtosend, ZCom_ConnID _destconn, ZCom_BitStream *_data, zFloat _aggressivenes );
  void acceptFile( ZCom_ConnID _src_id, ZCom_FileTransID _ftrans_id, const char *_path, bool _accept );
  const ZCom_FileTransInfo& getFileInfo( ZCom_ConnID _conn_id, ZCom_FileTransID _ftrans_id ) const;

  void setEventInterceptor(ZCom_NodeEventInterceptor *_interceptor);

  /* user data */
  bool setUserData( ZCom_ConnID _conn, void *_data );
  void* getUserData( ZCom_ConnID _conn ) const;
  void setUserData( void *_data );
  void* getUserData() const;

  /* misc */
  zS32 getUpdateDelta() const;
  zU32 getCurrentUpdateRate() const;
  zS32 getEstimatedTimeUntilPossibleUpdate() const;
  ZCom_Control* getControl() const;
  eZCom_NodeRole getRole() const;
  bool isPrivate() const;
  ZCom_ClassID getClassID() const;
  ZCom_NodeID getNetworkID() const;

  /* ---------------------------------------------------------------------
   * Phase B shim-internal hooks. NOT part of the real ZoidCom API --
   * used only by compat/zoidcom/src/Control.cpp to deliver wire-level
   * linking, ownership changes and events into this node. Game code must
   * never call these. See the file header above and
   * docs/porting/phase-b-replication.md.
   * ------------------------------------------------------------------ */

  /// Bind this node's own network id (and, for a proxy/owner node, the
  /// connection its authority lives on) once it is known -- either
  /// self-assigned (authority) or received from the peer (proxy/owner).
  void ZCom_shimBindSelf(ZCom_NodeID _netid, ZCom_ConnID _authority_conn);

  /// Authority-side only: mark _conn as newly relevant to this node and
  /// queue the deferred NODE_CREATE/NODE_LINK_UNIQUE announcement (flushed
  /// from ZCom_processOutput(), see ZCom_shimFlushPendingAnnouncements()).
  void ZCom_shimQueueAnnounce(ZCom_ConnID _conn);

  /// Authority-side only: send the deferred announcement(s) queued by
  /// ZCom_shimQueueAnnounce() for every connection still pending, using
  /// whatever role (Proxy/Owner) is current at flush time -- this is what
  /// lets a setOwner() call made synchronously after registration (as the
  /// game commonly does) land in the very first announcement instead of a
  /// separate, later message.
  void ZCom_shimFlushPendingAnnouncements();

  /// Proxy/owner-side: apply a role change received via a NODE_OWNER wire
  /// message (or embedded directly in a NODE_CREATE for a node that was
  /// already an owner at announce time).
  void ZCom_shimSetOwnerRole(bool _is_owner);

  /// Deliver an incoming NODE_EVENT (or a locally-synthesized
  /// eZCom_EventInit/eZCom_EventRemoved) into this node's event queue.
  void ZCom_shimDeliverEvent(eZCom_Event _type, eZCom_NodeRole _remote_role, ZCom_ConnID _conn_id,
    ZCom_BitStream* _data, zU32 _estimated_time_sent);

  /// A NODE_REMOVE arrived for this node from _from_conn. Mandatory
  /// eZCom_EventRemoved delivery (cannot be gated by setEventNotification()
  /// on the receiving/proxy side -- see zoidcom-original-semantics.md §9).
  void ZCom_shimDeliverRemove(ZCom_ConnID _from_conn);

  /// _conn just disconnected from the control this node lives on. Cleans
  /// up authority-side bookkeeping (optionally raising eZCom_EventRemoved
  /// if setEventNotification(_, true) was requested) or, if this node is a
  /// proxy/owner whose authority connection just closed, raises the
  /// mandatory eZCom_EventRemoved.
  void ZCom_shimNoteConnectionClosed(ZCom_ConnID _conn);

  /// Authority-side only: the role (Proxy/Owner) this node currently
  /// considers _conn to have, defaulting to Proxy if _conn isn't linked.
  /// Used to fill in remote_role when delivering an incoming NODE_EVENT.
  eZCom_NodeRole ZCom_shimRemoteRoleFor(ZCom_ConnID _conn) const;

  /* ---------------------------------------------------------------------
   * Phase B steps 3-4 shim-internal hooks (the replication tick). NOT part
   * of the real ZoidCom API. See docs/porting/phase-b-replication.md and
   * compat/zoidcom/src/Node.cpp's file header.
   * ------------------------------------------------------------------ */

  /// Called once per node, once per ZCom_Control::ZCom_processReplicators()
  /// call: drives Process() on every CALLPROCESS-flagged
  /// ZCom_ReplicatorAdvanced item, then computes this tick's dirty set
  /// across every primitive (addReplicationInt/Bool/Float/String/StringW)
  /// and ZCom_ReplicatorBasic item and fans it out to every relevant
  /// connection per the item's replication rule/flags.
  void ZCom_shimTickReplication(zU32 _simulation_time_passed);

  /// Applies an incoming primitive/ZCom_ReplicatorBasic batch (a
  /// kMsgNodeReplBatch payload, already unwrapped down to the item list) to
  /// this node's registered fields/replicators. _from_conn/_remote_role
  /// identify the sender, so the replication interceptor's inPreUpdate()/
  /// inPreUpdateItem()/inPostUpdate() (if one is registered) can be driven
  /// with correct arguments -- see zoidcom_node_interceptors.h and
  /// Node.cpp's file header.
  void ZCom_shimApplyReplBatch(ZCom_BitStream& _envelope, ZCom_ConnID _from_conn,
    eZCom_NodeRole _remote_role, zU32 _estimated_time_sent);

  /// This node's dependsOn() edges (see dependsOn() above), exposed so
  /// Control.cpp's deferred-announcement flush can topologically order
  /// nodes pending announcement in the same ZCom_processOutput() cycle --
  /// see ZCom_shimFlushPendingAnnouncements() and Control.cpp's
  /// topoOrderPendingFlush().
  std::vector<ZCom_Node*> ZCom_shimGetDependencies() const;

  /// Backs ZCom_ReplicatorAdvanced::sendData()/sendDataDirect() (see
  /// Replicator.cpp): finds _rep's item index on this node and sends
  /// _stream to either _direct_dest alone, or (if _direct_dest is
  /// ZCom_Invalid_ID) every connection the setup's replication rule
  /// permits. Always takes ownership of (and eventually deletes) _stream.
  void ZCom_shimSendAdvancedData(ZCom_Replicator* _rep, eZCom_SendMode _mode, ZCom_BitStream* _stream,
    zU32 _reference_id, ZCom_ConnID _direct_dest);

  /// Delivers an incoming kMsgReplAdvanced payload to the
  /// ZCom_ReplicatorAdvanced at _item_index via onDataReceived(). Always
  /// deletes _payload.
  void ZCom_shimDeliverReplAdvanced(zU16 _item_index, ZCom_ConnID _from_conn, eZCom_NodeRole _remote_role,
    ZCom_BitStream* _payload, zU32 _estimated_time_sent);
};

#endif
