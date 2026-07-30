// ZoidCom-compatible shim: ZCom_ReplicatorSetup / ZCom_Replicator /
// ZCom_ReplicatorBasic / ZCom_ReplicatorAdvanced base-class bookkeeping.
//
// Real: setup storage, replicator flags/id bookkeeping, the peek-buffer
// plumbing (peekDataStore/peekDataRetrieve/getPeekStream), operator new/
// delete honoring ZoidCom::overrideMemoryHandlers().
//
// Real (Phase B, steps 3-4 -- see docs/porting/phase-b-replication.md):
// checkState()/packData()/unpackData() (ZCom_ReplicatorBasic) and Process()
// (both) are now actually driven by ZCom_Node::ZCom_shimTickReplication(),
// called once per node from ZCom_Control::ZCom_processReplicators() --
// see Node.cpp's file header for the full design.
// ZCom_ReplicatorAdvanced::sendData()/sendDataDirect() below now route
// through ZCom_Node::ZCom_shimSendAdvancedData(), which resolves the
// destination connection(s) from this replicator's node and setup rules
// and hands off to ZCom_Control's wire send. onDataReceived() is called
// for real from ZCom_Node::ZCom_shimDeliverReplAdvanced().
//
// TODO(phaseB): getLastUpdateTime() stays a stub -- the one concrete
// ZCom_ReplicatorAdvanced in this codebase (EntityPropertyMapReplicator)
// never calls it (mindelay/maxdelay are explicitly not enforced for
// Advanced replicators per zoidcom-original-semantics.md §5, so it has no
// self-timing need).
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
    if (!m_node) {
        zshim::todoPhaseBOnce("ZCom_ReplicatorAdvanced::sendData(no-node)",
            "sendData() called before this replicator was attached to a node (addReplicator() calls setNode())");
        delete _stream;
        return;
    }
    // ZCom_Invalid_ID as the destination means "broadcast per the setup's
    // replication rule direction" -- see ZCom_Node::ZCom_shimSendAdvancedData().
    m_node->ZCom_shimSendAdvancedData(this, _mode, _stream, _reference_id, ZCom_Invalid_ID);
}

void ZCom_ReplicatorAdvanced::sendDataDirect(eZCom_SendMode _mode, ZCom_ConnID _dest, ZCom_BitStream* _stream, zU32 _reference_id) {
    if (!m_node) {
        zshim::todoPhaseBOnce("ZCom_ReplicatorAdvanced::sendDataDirect(no-node)",
            "sendDataDirect() called before this replicator was attached to a node (addReplicator() calls setNode())");
        delete _stream;
        return;
    }
    m_node->ZCom_shimSendAdvancedData(this, _mode, _stream, _reference_id, _dest);
}
