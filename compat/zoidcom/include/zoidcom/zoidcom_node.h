/****************************************
* zoidcom_node.h
* node object -- ZoidCom-compatible shim
*
* Real (Phase A): node identity bookkeeping (class id, role, control
* pointer, network id assignment for authority nodes), per-connection and
* global user data storage, replicator list ownership/autodelete.
*
* TODO(phaseB) stubs: cross-network node linking (a proxy node on one
* ZCom_Control never actually links up with its authority counterpart on
* another ZCom_Control), data replication (addReplicationInt/Float/Bool/
* String/StringW, addInterpolationInt/Float -- recorded but never synced),
* event queue (checkEventWaiting()/getNextEvent() -- always empty),
* sendEvent()/sendEventDirect() (log + drop), Zoidlevel membership,
* mustsync/authority migration, file transfer. See
* docs/porting/zoidcom-compat.md for the full rationale and what Phase B
* needs to build.
*****************************************/

#ifndef _ZOIDNODE_H_
#define _ZOIDNODE_H_

#include "zoidcom.h"

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
};

#endif
