/****************************************
* zoidcom_address.h
* network address -- ZoidCom-compatible shim
*
* Real implementation backed by plain string/IP parsing (no ENet types
* leak into this header, so game code including it doesn't need ENet on
* its include path). Hostname resolution is synchronous only (see .cpp);
* async resolveHostname(true, ...) is a TODO(phaseB) stub.
*****************************************/

#ifndef _ZOIDCOMADDRESS_H_
#define _ZOIDCOMADDRESS_H_

#include "zoidcom.h"

class ZCom_Address_Private;

class ZCOM_API ZCom_Address
{
private:
  ZCom_Address_Private *m_priv;
public:
  ZCom_Address(void);
  ZCom_Address(const ZCom_Address &);
  ~ZCom_Address(void);

  bool setAddress(eZCom_AddressType _type, zU8 _control_id, const char *_addr);

  const char* getAddressIP(eZCom_GetIPAddressOption _with_port = eZCom_AddressWithPort) const;
  const char* getAddressHostname() const;
  const char* toString() const;

  void setIP(zU8 _a, zU8 _b, zU8 _c, zU8 _d);
  void setIP(zU32 _ip);
  void setPort(zU16 _port);
  void setType(eZCom_AddressType _type);
  void setControlID(zU8 _id);

  zU16 getPort(void) const;
  zU32 getIP(void) const;
  zU8 getIP(zU8 _pos) const;
  eZCom_AddressType getType(void) const;
  zU8 getControlID(void) const;

  bool operator==(const ZCom_Address &_c) const;
  ZCom_Address& operator=(const ZCom_Address &_s);

  bool resolveHostname(bool _async, zU32 _timeout);
  eZCom_HostnameResult checkHostname();

  zU32 computeHashKey(zU32 _max) const;
};

#endif
