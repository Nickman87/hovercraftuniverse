/****************************************
* zoidcom_replicator_basic.h
* ZoidCom-compatible shim
*****************************************/

#ifndef _ZOIDREPLICATORBASIC_H_
#define _ZOIDREPLICATORBASIC_H_

#include "zoidcom.h"

/** @brief Interface for standard data replicators. */
class ZCOM_API ZCom_ReplicatorBasic : public ZCom_Replicator
{
public:
  ZCom_ReplicatorBasic(ZCom_ReplicatorSetup *_setup);

  virtual bool checkState() = 0;
  virtual void packData(ZCom_BitStream *_stream) = 0;
  virtual void unpackData(ZCom_BitStream *_stream, bool _store, zU32 _estimated_time_sent) = 0;

  void setModified() { m_flags |= ZCOM_REPLICATOR_MODIFIED; }
};

#endif
