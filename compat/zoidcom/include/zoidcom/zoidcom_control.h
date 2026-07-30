/****************************************
* zoidcom_control.h
* network control class -- ZoidCom-compatible shim
*
* Real ENet transport underneath (see compat/zoidcom/src/Control.cpp):
* socket setup, connect/listen/disconnect, and raw ZCom_sendData()/
* ZCom_cbDataReceived() are implemented and functional. Class ID
* registration is real (simple name->id table). Zoidlevel / discovery /
* bandwidth-shaping / lag-simulation are honest TODO(phaseB) stubs -- see
* docs/porting/zoidcom-compat.md.
*****************************************/

#ifndef _ZOIDCONTROL_H_
#define _ZOIDCONTROL_H_

#include "zoidcom.h"

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
};

#endif
