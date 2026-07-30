/****************************************
* zoidcom.h
* ZoidCom-compatible shim -- main umbrella header
*
* Drop-in, API-compatible replacement for the dead closed-source ZoidCom
* networking library (vendor gone, only VC9 binaries survive). Written for
* the "revival" MSVC v143 porting effort.
*
* PHASE A SCOPE: this header (and the rest of compat/zoidcom/**) exists to
* get all ZoidCom-dependent game code to COMPILE AND LINK. A real ENet
* transport is wired underneath for connection lifecycle and raw data
* exchange. Node registration, data/event replication, interpolation and
* Zoidlevel/authority migration are honestly stubbed -- see
* docs/porting/zoidcom-compat.md for exactly what is real vs stub and what
* Phase B still has to build.
*****************************************/

#ifndef _ZOIDCOM_H_
#define _ZOIDCOM_H_

#include "zoidcom_prereq.h"

class ZoidCom_Private;

/** @brief Available socket types. */
enum eZCom_SocketType
{
  eZCom_SocketLocal,
  eZCom_SocketTCP,
  eZCom_SocketUDP,
  eZCom_SocketCount
};

/** @brief Blocking mode for ZCom_Control::ZCom_processInput() */
enum eZCom_BlockMode
{
  eZCom_Block,
  eZCom_NoBlock
};

/** @brief Set or remove dependency option for ZCom_Node::dependsOn(). */
enum eZCom_DependencyOpt
{
  eZCom_AddDependency,
  eZCom_RemoveDependency
};

/** @brief Discover options. */
enum eZCom_DiscoverOpt
{
  eZCom_DiscoverEnable,
  eZCom_DiscoverDisableAndKeep,
  eZCom_DiscoverDisable
};

/** @brief Address type. */
enum eZCom_AddressType
{
  eZCom_AddressLocal,
  eZCom_AddressTCP,
  eZCom_AddressUDP,
  eZCom_AddressBroadcast
};

enum eZCom_GetIPAddressOption
{
  eZCom_AddressWithPort,
  eZCom_AddressWithoutPort
};

/** @brief Result values for ZCom_Control::ZCom_cbConnectResult() */
enum eZCom_ConnectResult
{
  eZCom_ConnAccepted,
  eZCom_ConnDenied,
  eZCom_ConnTimeout,
  eZCom_ConnHostnameFailed,
  eZCom_ConnWrongVersion
};

/** @brief Reason value for ZCom_Control::ZCom_cbConnectionClosed */
enum eZCom_CloseReason
{
  eZCom_ClosedDisconnect,
  eZCom_ClosedTimeout,
  eZCom_ClosedReconnect
};

/** @brief Result values for ZCom_Control::ZCom_cbZoidResult() */
enum eZCom_ZoidResult
{
  eZCom_ZoidEnabled,
  eZCom_ZoidDenied,
  eZCom_ZoidFailed_System,
  eZCom_ZoidFailed_Node,
  eZCom_ZoidDisabled
};

/** @brief Result values for ZCom_Address::checkHostname() */
enum eZCom_HostnameResult
{
  eZCom_HostnameIdle,
  eZCom_HostnameFailed,
  eZCom_HostnameSuccess,
  eZCom_HostnameInProgress
};

/** @brief Node Roles. See ZCom_Node::setOwner() and ZCom_Node::getRole() */
enum eZCom_NodeRole
{
  eZCom_RoleUndefined,
  eZCom_RoleProxy,
  eZCom_RoleOwner,
  eZCom_RoleAuthority
};

/** @brief Send modes. */
enum eZCom_SendMode
{
  eZCom_ReliableUnordered,
  eZCom_ReliableOrdered,
  eZCom_Unreliable,
  eZCom_UnreliableNotify
};

/** @brief ZCom_Node event types. */
enum eZCom_Event
{
  eZCom_EventNoEvent,
  eZCom_EventInit,
  eZCom_EventSyncRequest,
  eZCom_EventRemoved,
  eZCom_EventFile_Incoming,
  eZCom_EventFile_Data,
  eZCom_EventFile_Aborted,
  eZCom_EventFile_Complete,
  eZCom_EventReplicator,
  eZCom_EventUser
};

/// connection ID
typedef zU32 ZCom_ConnID;
/// class ID
typedef zU32 ZCom_ClassID;
/// node ID
typedef zU32 ZCom_NodeID;
/// file transfer ID
typedef zU32 ZCom_FileTransID;
/// sendgroup ID
typedef zU32 ZCom_GroupID;
/// replication item ID
typedef zU8  ZCom_InterceptID;

const ZCom_ClassID ZCom_Invalid_ID = 0;

/// Connection statistics.
struct ZCOM_API ZCom_ConnStats
{
  zU32  last_in;
  zU32  last_out;
  zU32  last_sec_out, last_sec_in;
  zU32  last_sec_outp, last_sec_inp;
  zU32  current_out, current_in;
  zU32  current_outp, current_inp;
  zU32  total_out, total_in;
  zU32 last_interval_update;
  zU16  ping;
  zU16 avg_ping;
  zU8 last_sec_loss_percent;
  zU8 last_sec_loss_count;
  zU8 current_loss_count;
};

/// File transfer information.
struct ZCOM_API ZCom_FileTransInfo
{
  ZCom_FileTransID id;
  zU32 size;
  zU32 transferred;
  const char  *path;
  zU32 bps;
};

/* ===================================== */

/**
 * @brief Main control class. May be instantiated only once per process.
 *
 * Implementation notes (shim): Init()/Clear() own the process-wide ENet
 * library lifetime (enet_initialize()/enet_deinitialize()) so that any
 * number of ZCom_Control instances can share it. This is real, working
 * behavior, not a stub.
 */
class ZCOM_API ZoidCom
{
private:
  ZoidCom_Private *m_priv;
public:
  ZoidCom();
  ZoidCom( const char *_logfile );
  ZoidCom( void ( *_logfunc ) ( const char* ) );
  ~ZoidCom();

  bool Init();
  void Clear( void );

  void setConnectionTimeout(zU32 _timeout);
  void setResendTimeout(zU32 _timeout);

  static void overrideMemoryHandlers(void*(*_allocator)(size_t _size), void(*_deallocator)(void *_ptr));

  void setLogLevel(zU8 _level);

  static zU32 getTime();
  static void Sleep(zU32 _msecs);
};

#include "zoidcom_bitstream.h"
#include "zoidcom_address.h"
#include "zoidcom_conngroup.h"
#include "zoidcom_control.h"
#include "zoidcom_replicator.h"
#include "zoidcom_replicator_basic.h"
#include "zoidcom_replicator_advanced.h"
#include "zoidcom_node_interceptors.h"
#include "zoidcom_node.h"

#endif
