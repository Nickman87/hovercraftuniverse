/****************************************
* zoidcom_replicator.h
* replicator base definition -- ZoidCom-compatible shim
*
* ZCom_ReplicatorSetup and the ZCom_Replicator base bookkeeping (flags,
* setup pointer, peek buffer plumbing) are implemented for real -- they're
* just plain data holders.
*
* Real (Phase B, steps 3-4): checkState()/packData()/unpackData()/Process()
* are now actually driven as part of a live replication tick -- see
* ZCom_Node::ZCom_shimTickReplication() in Node.cpp and
* ZCom_Control::ZCom_processReplicators() in Control.cpp, plus
* docs/porting/phase-b-replication.md.
*****************************************/

#ifndef _ZOIDREPLICATOR_H_
#define _ZOIDREPLICATOR_H_

#include "zoidcom.h"

class ZCom_Node;
class ZCom_Node_Private;

/** \name Replicator flags. Stored in m_flags in ZCom_Replicator. */
#define ZCOM_REPLICATOR_CALLPROCESS     (1L << 0)
#define ZCOM_REPLICATOR_ZCOMOWNAGE      (1L << 1)
#define ZCOM_REPLICATOR_INITIALIZED     (1L << 2)
#define ZCOM_REPLICATOR_BASIC           (1L << 3)
#define ZCOM_REPLICATOR_ADVANCED        (1L << 4)
#define ZCOM_REPLICATOR_MODIFIED        (1L << 5)
#define ZCOM_REPLICATOR_USER1           (1L << 6)
#define ZCOM_REPLICATOR_USER2           (1L << 7)

class ZCOM_API ZCom_ReplicatorSetup
{
friend class ZCom_Node_Private;
protected:
  zU8            m_flags;
  zU8            m_rules;
  zU8            m_intercept_id;
  zS16           m_mindelay;
  zS16           m_maxdelay;

public:
  ZCom_ReplicatorSetup(zU8 _flags, zU8 _rules);
  ZCom_ReplicatorSetup(zU8 _flags, zU8 _rules, zU8 _intercept_id,
                                zS16 _min_delay, zS16 _max_delay);
  virtual ~ZCom_ReplicatorSetup();

  virtual ZCom_ReplicatorSetup* Duplicate();

  inline zU8 getInterceptID() const { return m_intercept_id; }
  inline void setInterceptID(zU8 _id) { m_intercept_id = _id; }
  inline zS16 getMinDelay() const { return m_mindelay; }
  inline void setMinDelay(zS16 _dly) { m_mindelay = _dly; }
  inline zS16 getMaxDelay() const { return m_maxdelay; }
  inline void setMaxDelay(zS16 _dly) { m_maxdelay = _dly; }
  inline zU8 getRules() const { return m_rules; }
  inline zU8 getFlags() const { return m_flags; }

  void* operator new(size_t _size);
  void  operator delete(void *_p);
};

class ZCOM_API ZCom_Replicator
{
friend class ZCom_Node_Private;
private:
  zU16                  m_id;
protected:
  zU8                   m_flags;
  ZCom_ReplicatorSetup* m_setup;
public:
  ZCom_Replicator(ZCom_ReplicatorSetup *_setup);
  virtual ~ZCom_Replicator();

  void* operator new(size_t _size);
  void  operator delete(void *_p);

public:
  virtual void* peekData() = 0;
  virtual void clearPeekData() = 0;

protected:
  ZCom_BitStream* getPeekStream() const;
  void peekDataStore(void *_ptr);
  void* peekDataRetrieve();

public:
  virtual void Process(eZCom_NodeRole _localrole, zU32 _simulation_time_passed) = 0;

  inline bool callProcess() { return (m_flags & ZCOM_REPLICATOR_CALLPROCESS) ? true : false ; }

  ZCom_ReplicatorSetup *getSetup() const { return m_setup; }
  zU8 getFlags() const { return m_flags; }
  zU16 getId() const { return m_id; }
};

#endif
