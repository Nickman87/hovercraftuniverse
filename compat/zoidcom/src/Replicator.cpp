// ZoidCom-compatible shim: ZCom_ReplicatorSetup / ZCom_Replicator /
// ZCom_ReplicatorBasic / ZCom_ReplicatorAdvanced base-class bookkeeping.
//
// Real: setup storage, replicator flags/id bookkeeping, the peek-buffer
// plumbing (peekDataStore/peekDataRetrieve/getPeekStream), operator new/
// delete honoring ZoidCom::overrideMemoryHandlers().
//
// TODO(phaseB): nothing currently *drives* checkState()/packData()/
// unpackData()/Process() on a live replication tick (see
// ZCom_Node::addReplicator() in Node.cpp and
// ZCom_Control::ZCom_processReplicators() in Control.cpp), and
// ZCom_ReplicatorAdvanced::sendData()/sendDataDirect() have no node-level
// routing to deliver to yet, so they log-and-drop.
#include "zoidcom_shim_internal.h"

// --- ZCom_ReplicatorSetup -------------------------------------------------

ZCom_ReplicatorSetup::ZCom_ReplicatorSetup(zU8 _flags, zU8 _rules)
    : m_flags(_flags), m_rules(_rules), m_intercept_id(0), m_mindelay(-1), m_maxdelay(-1) {}

ZCom_ReplicatorSetup::ZCom_ReplicatorSetup(zU8 _flags, zU8 _rules, zU8 _intercept_id,
                                            zS16 _min_delay, zS16 _max_delay)
    : m_flags(_flags), m_rules(_rules), m_intercept_id(_intercept_id),
      m_mindelay(_min_delay), m_maxdelay(_max_delay) {}

ZCom_ReplicatorSetup::~ZCom_ReplicatorSetup() {}

ZCom_ReplicatorSetup* ZCom_ReplicatorSetup::Duplicate() {
    if (m_flags & ZCOM_REPFLAG_SETUPPERSISTS) return this;
    return new ZCom_ReplicatorSetup(m_flags, m_rules, m_intercept_id, m_mindelay, m_maxdelay);
}

void* ZCom_ReplicatorSetup::operator new(size_t _size) { return zshim::zalloc(_size); }
void ZCom_ReplicatorSetup::operator delete(void* _p) { zshim::zfree(_p); }

// --- ZCom_Replicator -------------------------------------------------------

namespace {
thread_local void* tls_peek_ptr = NULL;
thread_local ZCom_BitStream* tls_peek_stream = NULL;
zU16 gNextReplicatorId = 1;
}

ZCom_Replicator::ZCom_Replicator(ZCom_ReplicatorSetup* _setup)
    : m_id(gNextReplicatorId++), m_flags(0), m_setup(_setup) {}

ZCom_Replicator::~ZCom_Replicator() {
    // Real ZoidCom deletes m_setup only if ZCOM_REPFLAG_SETUPAUTODELETE is
    // set on it (that flag lives on the setup object, not the replicator).
    if (m_setup && (m_setup->getFlags() & ZCOM_REPFLAG_SETUPAUTODELETE)) {
        delete m_setup;
    }
}

void* ZCom_Replicator::operator new(size_t _size) { return zshim::zalloc(_size); }
void ZCom_Replicator::operator delete(void* _p) { zshim::zfree(_p); }

ZCom_BitStream* ZCom_Replicator::getPeekStream() const {
    return tls_peek_stream;
}

void ZCom_Replicator::peekDataStore(void* _ptr) {
    if (tls_peek_ptr) {
        // Real ZoidCom calls clearPeekData() automatically in this case;
        // we can't call the pure virtual from here portably without a
        // vtable-safe cast, so just log -- this path requires an
        // interceptor to actually be driving peek calls, which Phase A
        // doesn't do yet anyway.
        zshim::todoPhaseBOnce("ZCom_Replicator::peekDataStore(double-store)",
            "peekDataStore() called twice without an intervening clearPeekData()");
    }
    tls_peek_ptr = _ptr;
}

void* ZCom_Replicator::peekDataRetrieve() {
    void* p = tls_peek_ptr;
    tls_peek_ptr = NULL;
    return p;
}

// Internal helper used by Node.cpp's (future, Phase B) interceptor-driven
// peek path to set which stream getPeekStream() should return.
namespace zshim {
void setThreadLocalPeekStream(ZCom_BitStream* s) { tls_peek_stream = s; }
}

// --- ZCom_ReplicatorBasic ---------------------------------------------------

ZCom_ReplicatorBasic::ZCom_ReplicatorBasic(ZCom_ReplicatorSetup* _setup) : ZCom_Replicator(_setup) {
    m_flags |= ZCOM_REPLICATOR_BASIC;
}

// --- ZCom_ReplicatorAdvanced -------------------------------------------------

ZCom_ReplicatorAdvanced::ZCom_ReplicatorAdvanced(ZCom_ReplicatorSetup* _setup)
    : ZCom_Replicator(_setup), m_node(NULL) {
    m_flags |= ZCOM_REPLICATOR_ADVANCED;
}

ZCom_Node* ZCom_ReplicatorAdvanced::getNode() const { return m_node; }

void ZCom_ReplicatorAdvanced::setNode(ZCom_Node* _node) { m_node = _node; }

zU32* ZCom_ReplicatorAdvanced::getLastUpdateTime(ZCom_ConnID _cid) {
    (void) _cid;
    zshim::todoPhaseBOnce("ZCom_ReplicatorAdvanced::getLastUpdateTime",
        "per-connection last-update timestamps are not tracked yet (advanced replicators must self-time mindelay/maxdelay)");
    return NULL;
}

void ZCom_ReplicatorAdvanced::sendData(eZCom_SendMode _mode, ZCom_BitStream* _stream, zU32 _reference_id) {
    (void) _mode; (void) _reference_id;
    zshim::todoPhaseBOnce("ZCom_ReplicatorAdvanced::sendData",
        "advanced-replicator event routing to peer replicators is not implemented; data is dropped");
    delete _stream; // matches real contract: ownership is taken and eventually freed
}

void ZCom_ReplicatorAdvanced::sendDataDirect(eZCom_SendMode _mode, ZCom_ConnID _dest, ZCom_BitStream* _stream, zU32 _reference_id) {
    (void) _mode; (void) _dest; (void) _reference_id;
    zshim::todoPhaseBOnce("ZCom_ReplicatorAdvanced::sendDataDirect",
        "advanced-replicator direct event routing is not implemented; data is dropped");
    delete _stream;
}
