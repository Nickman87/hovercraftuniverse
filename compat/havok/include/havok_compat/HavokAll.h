// HavokAll.h -- Havok 6.6 -> Bullet 3.25 compatibility shim, Phase A.
//
// This single header holds the entire API surface the game needs. Every
// "real" Havok header path the game #includes (e.g. <Physics/Dynamics/
// World/hkpWorld.h>) is a one-line forwarding stub under compat/havok/
// include/ that just does `#include "havok_compat/HavokAll.h"`. That keeps
// the game's #include list unchanged (it still says <Physics/Dynamics/
// World/hkpWorld.h>) while keeping the actual implementation in one place
// instead of ~45 tiny files that would have to be kept in perfect sync.
//
// See docs/porting/havok-compat.md for the full symbol table, the
// Havok->Bullet mapping rationale, and every place this shim's behavior
// diverges from real Havok 6.6.
//
// Ownership: compat/havok/** (this porting effort). Not a port of anything
// under HovercraftUniverse/ -- written from scratch against the real Havok
// 6.6 headers (HovercraftUniverse/dependencies/Havoc/Source/) used strictly
// as an API reference, and against the actual game usage (grepped across
// HovercraftUniverse/HovercraftUniverse/**).
#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <functional>

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <BulletCollision/NarrowPhaseCollision/btGjkPairDetector.h>
#include <BulletCollision/NarrowPhaseCollision/btPointCollector.h>
#include <BulletCollision/CollisionShapes/btConvexShape.h>
#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <BulletCollision/CollisionShapes/btSphereShape.h>
#include <BulletCollision/CollisionShapes/btCapsuleShape.h>
#include <BulletCollision/CollisionShapes/btConvexHullShape.h>
#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>
#include <BulletCollision/CollisionShapes/btTriangleMesh.h>

#include "havok_compat/CollisionProvider.h"

namespace havok_compat {
    // Logs `msg` (prefixed "TODO(phaseB): ") to stderr exactly once per
    // distinct call site, keyed by `site` (pass __FUNCTION__ or a literal).
    // Mirrors compat/zoidcom's zshim::todoPhaseBOnce() -- same convention.
    void todoPhaseBOnce(const char* site, const std::string& msg);
}

// ===========================================================================
// Basic typedefs / macros (Common/Base/hkBase.h)
// ===========================================================================

typedef float hkReal;
typedef int32_t hkInt32;
typedef uint32_t hkUint32;
typedef int64_t hkInt64;
typedef uint64_t hkUint64;
typedef bool hkBool;
#if defined(_WIN64)
typedef uint64_t hkUlong;
typedef int64_t hkLong;
#else
typedef unsigned long hkUlong;
typedef long hkLong;
#endif

#define HK_NULL 0
#define HK_SUCCESS 0
#define HK_FAILURE -1
#define HK_CALL
#define HK_REAL_DEG_TO_RAD (0.017453292f)
#define HK_REAL_RAD_TO_DEG (57.295779513f)

// HK_ASSERT / timers / display -- Phase A no-ops (real Havok versions are
// debug/dev-time aids only; they never change game behavior).
#define HK_ASSERT(id, expr) do { } while(0)
#define HK_ASSERT2(id, expr, msg) do { } while(0)
#define HK_TIMER_BEGIN(name, split) do { } while(0)
#define HK_TIMER_END() do { } while(0)

// hkReferencedObject: simple intrusive refcount base, root of most Havok
// heap classes (hkpShape, hkpWorldObject i.e. hkpEntity/hkpPhantom, hkpAction).
class hkReferencedObject {
protected:
    mutable int m_referenceCount;
public:
    hkReferencedObject() : m_referenceCount(1) {}
    virtual ~hkReferencedObject() {}
    void addReference() const { ++m_referenceCount; }
    void removeReference() const {
        if (--m_referenceCount <= 0) delete this;
    }
    int getReferenceCount() const { return m_referenceCount; }
};

// ---------------------------------------------------------------------------
// Math: hkVector4 / hkQuaternion / hkRotation / hkTransform / hkAabb
// ---------------------------------------------------------------------------
// These are implemented for real (not stubbed) -- everything else in the
// shim, and the game's own math, depends on them behaving correctly.
// Internally backed by Bullet's btVector3/btQuaternion/btMatrix3x3/btTransform.

class hkVector4 {
public:
    btVector3 v;

    hkVector4() : v(0, 0, 0) {}
    hkVector4(hkReal x, hkReal y, hkReal z, hkReal w = 0) : v(x, y, z) { m_w = w; }
    hkVector4(const btVector3& bv) : v(bv), m_w(0) {}

    hkReal m_w = 0;

    void set(hkReal x, hkReal y, hkReal z, hkReal w = 0) { v.setValue(x, y, z); m_w = w; }
    void setZero4() { v.setValue(0, 0, 0); m_w = 0; }
    void zeroElement(int i) { (*this)(i) = 0; }

    hkReal operator()(int i) const { return i == 3 ? m_w : v[i]; }
    hkReal& operator()(int i) {
        static hkReal wtmp;
        if (i == 3) { wtmp = m_w; return wtmp; }
        return *(&v.m_floats[i]);
    }
    hkReal getSimdAt(int i) const { return (*this)(i); }

    void add4(const hkVector4& o) { v += o.v; }
    void sub4(const hkVector4& o) { v -= o.v; }
    void mul4(hkReal s) { v *= s; }
    void setAdd4(const hkVector4& a, const hkVector4& b) { v = a.v + b.v; }
    void setSub4(const hkVector4& a, const hkVector4& b) { v = a.v - b.v; }
    void setMul4(hkReal s, const hkVector4& a) { v = a.v * s; }
    void setNeg4(const hkVector4& a) { v = -a.v; }
    void setCross(const hkVector4& a, const hkVector4& b) { v = a.v.cross(b.v); }

    hkReal length3() const { return v.length(); }
    hkReal lengthSquared3() const { return v.length2(); }
    void normalize3() {
        hkReal l = v.length();
        if (l > 1e-8f) v /= l;
    }
    hkReal dot3(const hkVector4& o) const { return v.dot(o.v); }

    bool equals3(const hkVector4& o, hkReal eps) const {
        return (v - o.v).length() <= eps;
    }

    // Applies transform tr to point (this=pos1 semantics from the game code):
    // out = tr.rotation * pos + tr.translation
    void setTransformedPos(const class hkTransform& tr, const hkVector4& pos);
    void setRotatedDir(const class hkQuaternion& q, const hkVector4& dir);

    operator btVector3() const { return v; }
};

inline hkVector4 operator+(const hkVector4& a, const hkVector4& b) { hkVector4 r; r.v = a.v + b.v; return r; }
inline hkVector4 operator-(const hkVector4& a, const hkVector4& b) { hkVector4 r; r.v = a.v - b.v; return r; }

class hkQuaternion {
public:
    btQuaternion q;

    hkQuaternion() : q(0, 0, 0, 1) {}
    hkQuaternion(hkReal x, hkReal y, hkReal z, hkReal w) : q(x, y, z, w) {}
    hkQuaternion(const hkVector4& axis, hkReal angleRad) {
        q.setRotation(axis.v.normalized(), angleRad);
    }
    hkQuaternion(const class hkRotation& r);
    hkQuaternion(const btQuaternion& bq) : q(bq) {}

    void set(hkReal x, hkReal y, hkReal z, hkReal w) { q.setValue(x, y, z, w); }
    void set(const class hkRotation& r);
    void setIdentity() { q = btQuaternion::getIdentity(); }

    hkReal operator()(int i) const {
        switch (i) { case 0: return q.x(); case 1: return q.y(); case 2: return q.z(); default: return q.w(); }
    }

    hkReal getAngle() const { return q.getAngle(); }

    // Rough angular delta estimate between this and other, written into
    // outAxisAngle as an axis*angle vector (small-angle approximation is
    // fine here -- it only feeds a proportional controller in game code).
    void estimateAngleTo(const hkQuaternion& other, hkVector4& outAxisAngle) const {
        btQuaternion diff = other.q * q.inverse();
        if (diff.w() < 0) diff = -diff;
        btVector3 axis = diff.getAxis();
        hkReal angle = diff.getAngle();
        if (std::isnan(angle) || axis.length2() < 1e-12f) { outAxisAngle.set(0, 0, 0); return; }
        outAxisAngle.v = axis * angle;
    }

    operator btQuaternion() const { return q; }
};

class hkRotation {
public:
    // Real Havok's hkRotation (like hkMatrix3) stores its 3x3 matrix as 3
    // column hkVector4s, and game code commonly builds a rotation
    // column-by-column via the idiom `rot.getColumn(i) = someVector;` --
    // i.e. getColumn() returns a genuinely mutating reference straight into
    // the matrix's own storage. HavokHovercraft.cpp's update() (game code,
    // unmodified) relies on exactly that idiom to build its desired
    // orientation out of mSide/mUp/cross(mSide,mUp):
    //   newOrientation.getColumn(0) = mSide;
    //   newOrientation.getColumn(1) = mUp;
    //   newOrientation.getColumn(2).setCross(mSide, mUp);
    //   newOrientation.renormalize();
    //
    // BUG (found while investigating "steering does nothing" / "AI bot
    // drives off in a straight line" / "forward and backward are swapped"):
    // this class used to back the matrix with a single btMatrix3x3 and fake
    // getColumn()'s reference-into-storage semantics with a per-call cache
    // array (`hkVector4 cached[3]`) refreshed from the matrix on every call.
    // A cache refreshed from the matrix is not the same as a reference INTO
    // the matrix: assigning through the returned reference
    // (`getColumn(0) = mSide;`) only ever wrote `cached[0]`, while every
    // subsequent read of the rotation (getColumnRaw(), renormalize(),
    // hkQuaternion's matrix<->quaternion conversions) went straight through
    // the untouched btMatrix3x3. There WAS a commitColumn(i) method to push
    // a cached column back into the matrix, but nothing anywhere in the
    // codebase ever called it (confirmed by grepping the game and this shim)
    // -- so it was dead code, and every write via getColumn() was silently
    // discarded.
    //
    // Net effect: `newOrientation` in HavokHovercraft.cpp::update() was
    // always identity, no matter what mSide/mUp/steering input said.
    // `input.m_forward = newOrientation.getColumn(2)` was therefore always
    // a fixed WORLD-space axis (identity's column 2), never the hovercraft's
    // actual heading -- and the angular-velocity controller a few lines
    // later (`currentOrient.estimateAngleTo(desiredOrient, ...)`, desiredOrient
    // built from this same always-identity newOrientation) always steered
    // toward world-identity orientation, never toward whatever steering
    // input asked for. That one bug explains all three reported symptoms:
    // steering appearing completely dead (the desired orientation never
    // reflected steering input), the AI bot flying off in a straight line
    // (constant input.m_forward every step, so it could accelerate but
    // never actually turn), and forward/backward looking inverted
    // (accelerating always pushed along that fixed identity axis, which
    // happens to be antiparallel to the entity's actual forward convention
    // -- see Entity::getOrientation(), `mOrientation * NEGATIVE_UNIT_Z`).
    //
    // Fixed by making the 3 columns the actual ground truth storage (a
    // real hkVector4[3]), so getColumn() returns a genuine, persistent
    // reference that every later read observes immediately. btMatrix3x3 is
    // now only ever constructed on demand, as a scratch conversion helper
    // for quaternion<->matrix conversion -- never as the storage itself.
    hkVector4 col[3];

    hkRotation() { setIdentity(); }

    void set(const hkQuaternion& q) {
        btMatrix3x3 bm(q.q);
        for (int i = 0; i < 3; i++) col[i] = hkVector4(bm.getColumn(i));
    }
    void setIdentity() {
        col[0].set(1, 0, 0);
        col[1].set(0, 1, 0);
        col[2].set(0, 0, 1);
    }
    void renormalize() {
        // Gram-Schmidt re-orthonormalize the 3 columns (game builds a
        // rotation out of side/up/forward vectors that may not be
        // perfectly orthonormal after cross products).
        btVector3 c0 = col[0].v;
        btVector3 c2raw = col[2].v;
        c0.normalize();
        btVector3 c1 = c2raw.cross(c0); // recompute middle to be orthogonal-ish; keeps handedness stable
        if (c1.length2() > 1e-12f) c1.normalize(); else c1 = col[1].v.normalized();
        btVector3 c2 = c0.cross(c1);
        col[0].v = c0; col[1].v = c1; col[2].v = c2;
    }

    // Genuine reference into this object's own storage -- see the class
    // comment above for why that matters.
    hkVector4& getColumn(int i) { return col[i]; }
    const hkVector4& getColumn(int i) const { return col[i]; }

    // Scratch conversion helper, not storage -- used only by the
    // hkQuaternion<->hkRotation conversions below.
    btMatrix3x3 toBullet() const {
        return btMatrix3x3(
            col[0].v.x(), col[1].v.x(), col[2].v.x(),
            col[0].v.y(), col[1].v.y(), col[2].v.y(),
            col[0].v.z(), col[1].v.z(), col[2].v.z());
    }
};

inline hkQuaternion::hkQuaternion(const hkRotation& r) { r.toBullet().getRotation(q); }
inline void hkQuaternion::set(const hkRotation& r) { r.toBullet().getRotation(q); }

class hkTransform {
public:
    btTransform t;
    hkTransform() { t.setIdentity(); }
    hkTransform(const hkQuaternion& q, const hkVector4& pos) {
        t.setRotation(q.q);
        t.setOrigin(pos.v);
    }
    void setIdentity() { t.setIdentity(); }
    hkVector4 getTranslation() const { return hkVector4(t.getOrigin()); }
    hkQuaternion getRotation() const { return hkQuaternion(t.getRotation()); }
};

inline void hkVector4::setTransformedPos(const hkTransform& tr, const hkVector4& pos) {
    v = tr.t(pos.v);
}
inline void hkVector4::setRotatedDir(const hkQuaternion& q, const hkVector4& dir) {
    v = btMatrix3x3(q.q) * dir.v;
}

// NOTE: `class`, not `struct` -- HavokEntity.h (HovercraftUniverse/**, not
// ours to edit) forward-declares `class hkAabb;`; a struct/class mismatch
// on a forward declaration is only a (harmless) MSVC C4099 warning, but
// there's no reason to leave it triggering.
class hkAabb {
public:
    hkVector4 m_min;
    hkVector4 m_max;
    hkAabb() {}
    hkAabb(const hkVector4& mn, const hkVector4& mx) : m_min(mn), m_max(mx) {}
};

// hkString -- thin wrapper the game uses like a std::string (cString(),
// c_str()-style ctor from const char*).
class hkString {
    std::string s;
public:
    hkString() {}
    hkString(const char* c) : s(c ? c : "") {}
    hkString(const std::string& o) : s(o) {}
    const char* cString() const { return s.c_str(); }
    const char* c_str() const { return s.c_str(); }
    operator std::string() const { return s; }
};

// hkArray<T> -- minimal Havok-style dynamic array over std::vector.
template<typename T>
class hkArray {
    std::vector<T> data;
public:
    hkArray() {}
    int getSize() const { return (int)data.size(); }
    void pushBack(const T& v) { data.push_back(v); }
    int indexOf(const T& v) const {
        auto it = std::find(data.begin(), data.end(), v);
        return it == data.end() ? -1 : (int)(it - data.begin());
    }
    void removeAt(int i) { if (i >= 0 && i < (int)data.size()) data.erase(data.begin() + i); }
    T& operator[](int i) { return data[i]; }
    const T& operator[](int i) const { return data[i]; }
    typename std::vector<T>::iterator begin() { return data.begin(); }
    typename std::vector<T>::iterator end() { return data.end(); }
};

// hkStorageStringMap<T> -- minimal Havok-style string-keyed map with the
// get/insert/iterator surface the game actually uses.
template<typename T>
class hkStorageStringMap {
public:
    typedef std::map<std::string, T> MapType;
    typedef typename MapType::iterator Iterator;

    MapType m;

    Iterator getIterator() { return m.begin(); }
    bool isValid(const Iterator& it) { return it != m.end(); }
    Iterator getNext(const Iterator& it) { Iterator n = it; ++n; return n; }
    T getValue(const Iterator& it) { return it->second; }
    void setValue(const Iterator& it, T v) { it->second = v; }
    void insert(const char* key, T v) { m[key] = v; }
    int get(const char* key, T* out) const {
        auto it = m.find(key);
        if (it == m.end()) return HK_FAILURE;
        *out = it->second;
        return HK_SUCCESS;
    }
    void clear() { m.clear(); }
};

// ===========================================================================
// Common/Base/System (init/quit, hardware info, memory, thread memory)
// ===========================================================================

typedef void (HK_CALL *hkErrorReportFunction)(const char* msg, void* userContext);

class hkPoolMemory : public hkReferencedObject {
public:
    hkPoolMemory() {}
};

class hkThreadMemory : public hkReferencedObject {
    char* m_stackArea = nullptr;
    int m_stackSize = 0;
public:
    hkThreadMemory(hkPoolMemory*) {}
    static hkThreadMemory& getInstance();
    void setStackArea(char* area, int size) { m_stackArea = area; m_stackSize = size; }
};

struct hkHardwareInfo {
    int m_numThreads = 1;
};
inline void hkGetHardwareInfo(hkHardwareInfo& info) { info.m_numThreads = 1; }

class hkBaseSystem {
public:
    static void init(hkPoolMemory* mem, hkThreadMemory* threadMem, hkErrorReportFunction report);
    static void quit();
};

template<typename T>
inline T* hkAllocate(int count, int /*memClass*/) { return new T[count]; }
template<typename T>
inline void hkDeallocate(T* p) { delete[] p; }
#define HK_MEMORY_CLASS_BASE 0

// hkMonitorStream: dev-time profiling ring buffer. Phase A no-op.
class hkMonitorStream {
public:
    static hkMonitorStream& getInstance();
    void resize(int) {}
    void reset() {}
};

// hkDefaultError: dev-time error/warning sink. Not used directly by game code
// beyond the #include; nothing to implement.
class hkDefaultError {};

// hkIstream: thin wrapper over an ifstream used only to gate
// hkpHavokSnapshot::load() on file existence/openability.
class hkStreamReader {
public:
    std::string path;
};
class hkIstream {
    std::ifstream f;
public:
    hkStreamReader reader;
    hkIstream(const char* filename) : f(filename, std::ios::binary) { reader.path = filename ? filename : ""; }
    bool isOk() const { return f.good(); }
    hkStreamReader* getStreamReader() { return &reader; }
};

// ===========================================================================
// Common/Base/Thread/Job -- multithreaded job queue / thread pool.
// TODO(phaseB): real Havok fans this out across a job-based thread pool
// (hkCpuJobThreadPool/hkSpuJobThreadPool + hkJobQueue). The game only ever
// calls hkpWorld::stepMultithreaded(jobQueue, threadPool, dt); this shim
// steps Bullet's btDiscreteDynamicsWorld single-threaded on the calling
// thread and treats the queue/pool as inert handles. Functionally
// equivalent for a 30 Hz hovercraft game on modern hardware; NOT a faithful
// reproduction of Havok's SPU/job-based scheduling model.
// ===========================================================================

struct hkJobQueueCinfo {
    struct { int m_numCpuThreads = 1; } m_jobQueueHwSetup;
};
class hkJobQueue {
public:
    hkJobQueue(const hkJobQueueCinfo&) {}
};

struct hkCpuJobThreadPoolCinfo {
    int m_numThreads = 0;
    int m_timerBufferPerThreadAllocation = 0;
};
class hkJobThreadPool : public hkReferencedObject {
public:
    void clearTimerData() {}
};
class hkCpuJobThreadPool : public hkJobThreadPool {
public:
    hkCpuJobThreadPool(const hkCpuJobThreadPoolCinfo&) {}
};
// hkSpuJobThreadPool.h is included by the game but never instantiated
// (PC build); provide the symbol for link-completeness only.
class hkSpuJobThreadPool : public hkJobThreadPool {};

// ===========================================================================
// Common/Visualize -- Visual Debugger. Dev-time-only tool in real Havok
// (a network server you connect the standalone hkVisualDebugger.exe viewer
// to). TODO(phaseB): not implemented -- no viewer exists to connect to
// after this port, and the game never queries it for gameplay state, only
// drives its lifecycle (serve()/step()). Safe, honest no-op.
// ===========================================================================

class hkProcessContext {
public:
    virtual ~hkProcessContext() {}
};
class hkVisualDebugger : public hkReferencedObject {
public:
    hkVisualDebugger(hkArray<hkProcessContext*>&) {}
    void serve() {}
    void step(hkReal = 0) {}
};

namespace hkColor { enum Color { AZURE = 0xFF007FFF }; }
// HK_DISPLAY_LINE: dev-time debug-draw-to-VDB macro. No-op (no VDB viewer).
#define HK_DISPLAY_LINE(a, b, color) do { } while(0)

// ===========================================================================
// Forward declarations for the physics object graph
// ===========================================================================

class hkpWorld;
class hkpEntity;
class hkpRigidBody;
class hkpPhantom;
class hkpAction;
class hkpShape;
class hkpCollidable;

// hkpPropertyValue -- used by HavokEntityType to stash an int tag on
// hkpWorldObject instances.
class hkpPropertyValue {
    int m_int = 0;
public:
    void setInt(int v) { m_int = v; }
    int getInt() const { return m_int; }
};

// hkpWorldObject: root of hkpEntity/hkpPhantom. Owns the small property bag
// HavokEntityType uses, user-data, and the collidable.
class hkpWorldObject : public hkReferencedObject {
public:
    enum BroadPhaseType { BROAD_PHASE_ENTITY, BROAD_PHASE_PHANTOM };

    hkUlong m_userData = 0;
    std::unordered_map<int, hkpPropertyValue> m_properties;
    hkpCollidable* m_collidable = nullptr;

    virtual ~hkpWorldObject() {}

    void setUserData(hkUlong d) { m_userData = d; }
    hkUlong getUserData() const { return m_userData; }

    bool hasProperty(int key) const { return m_properties.count(key) != 0; }
    hkpPropertyValue getProperty(int key) const {
        auto it = m_properties.find(key);
        return it == m_properties.end() ? hkpPropertyValue() : it->second;
    }
    void editProperty(int key, const hkpPropertyValue& v) { m_properties[key] = v; }
    void addProperty(int key, const hkpPropertyValue& v) { m_properties[key] = v; }
    void unlockPropertiesFromLoadedObject() { /* no-op: shim never locks these */ }

    hkpCollidable* getCollidable() const { return m_collidable; }
    virtual BroadPhaseType getBroadPhaseType() const = 0;
};

// hkpCollidable: minimal wrapper the game dereferences to get back to the
// owning hkpEntity/hkpPhantom (getOwner()) and its shape (getShape()).
class hkpCollidable {
public:
    hkpWorldObject* m_owner = nullptr;
    hkpShape* m_shape = nullptr;

    void* getOwner() const { return m_owner; }
    hkpShape* getShape() const { return m_shape; }
    hkpWorldObject::BroadPhaseType getType() const { return m_owner->getBroadPhaseType(); }
};

// hkGetRigidBody(handle): pervasive game helper, real Havok provides the
// equivalent free function. Returns the owning rigid body if the collidable
// belongs to one, else HK_NULL.
inline hkpRigidBody* hkGetRigidBody(const hkpCollidable* handle);

// ===========================================================================
// Physics/Collide/Shape -- hkpShape family. Real, Bullet-backed: each
// concrete shape just owns/wraps a matching btCollisionShape.
// ===========================================================================

typedef int hkpShapeType;
enum { HK_SHAPE_BOX, HK_SHAPE_SPHERE, HK_SHAPE_CAPSULE, HK_SHAPE_CONVEX_VERTICES, HK_SHAPE_MESH, HK_SHAPE_BV_TREE, HK_SHAPE_OTHER };

class hkpShape : public hkReferencedObject {
public:
    btCollisionShape* m_bulletShape = nullptr;
    hkpShapeType m_type = HK_SHAPE_OTHER;

    virtual ~hkpShape() { delete m_bulletShape; }
    hkpShapeType getType() const { return m_type; }
    btCollisionShape* bullet() const { return m_bulletShape; }
};

class hkpBoxShape : public hkpShape {
public:
    hkVector4 m_halfExtents;
    hkpBoxShape(const hkVector4& halfExtents) : m_halfExtents(halfExtents) {
        m_type = HK_SHAPE_BOX;
        m_bulletShape = new btBoxShape(halfExtents.v);
    }
};

class hkpSphereShape : public hkpShape {
public:
    hkReal m_radius;
    hkpSphereShape(hkReal radius) : m_radius(radius) {
        m_type = HK_SHAPE_SPHERE;
        m_bulletShape = new btSphereShape(radius);
    }
};

class hkpCapsuleShape : public hkpShape {
public:
    hkVector4 m_vertexA, m_vertexB;
    hkReal m_radius;
    hkpCapsuleShape(const hkVector4& a, const hkVector4& b, hkReal radius)
        : m_vertexA(a), m_vertexB(b), m_radius(radius) {
        m_type = HK_SHAPE_CAPSULE;
        // Bullet's btCapsuleShape is always axis-aligned along Y through the
        // origin with a given half-height; approximate Havok's arbitrary
        // A/B segment capsule with that (good enough for the probe-shaped
        // capsules this game actually uses -- see docs/porting/havok-compat.md).
        hkReal halfHeight = (b - a).v.length() * 0.5f;
        m_bulletShape = new btCapsuleShape(radius, halfHeight * 2.0f);
    }
};

// hkpConvexVerticesShape / hkpMeshShape / hkpBvTreeShape -- declared for API
// completeness (the game's own code never constructs these directly; they'd
// come from .hkx level geometry, which is Phase B -- see
// docs/porting/havok-compat.md's ".hkx collision data" section). Backed for
// real by btConvexHullShape / btBvhTriangleMeshShape so that whoever
// implements the Phase B mesh-from-OgreMax-geometry loader has a working
// target to construct.
class hkpConvexVerticesShape : public hkpShape {
public:
    hkpConvexVerticesShape(const std::vector<hkVector4>& verts) {
        m_type = HK_SHAPE_CONVEX_VERTICES;
        auto* hull = new btConvexHullShape();
        for (auto& v : verts) hull->addPoint(v.v);
        m_bulletShape = hull;
    }
};
class hkpMeshShape : public hkpShape {
public:
    btTriangleMesh* m_mesh = nullptr;
    hkpMeshShape(btTriangleMesh* mesh) : m_mesh(mesh) {
        m_type = HK_SHAPE_MESH;
        m_bulletShape = new btBvhTriangleMeshShape(mesh, true);
    }
    ~hkpMeshShape() override { delete m_mesh; }
};
typedef hkpMeshShape hkpBvTreeShape; // Havok wraps mesh shapes in a bounding-volume tree; Bullet's btBvhTriangleMeshShape already is one.

// ===========================================================================
// Physics/Dynamics/Entity -- hkpEntity / hkpRigidBody. Real, Bullet-backed.
// ===========================================================================

class hkpMotion {
public:
    enum MotionType { MOTION_DYNAMIC, MOTION_FIXED, MOTION_KEYFRAMED };
    btRigidBody* m_body;
    hkpMotion(btRigidBody* b) : m_body(b) {}
    hkVector4 getLinearVelocity() const { return hkVector4(m_body->getLinearVelocity()); }
};

class hkpEntityActivationListener {
public:
    virtual ~hkpEntityActivationListener() {}
    virtual void entityDeactivatedCallback(hkpEntity* entity) {}
    virtual void entityActivatedCallback(hkpEntity* entity) {}
};

class hkpCollisionListener; // fwd, defined in Physics/Dynamics/Collide section below

class hkpEntity : public hkpWorldObject {
public:
    hkpWorld* m_world = nullptr;
    std::vector<hkpAction*> m_actions;
    std::vector<hkpEntityActivationListener*> m_activationListeners;
    std::vector<hkpCollisionListener*> m_collisionListeners;

    BroadPhaseType getBroadPhaseType() const override { return BROAD_PHASE_ENTITY; }

    hkpWorld* getWorld() const { return m_world; }

    void addEntityActivationListener(hkpEntityActivationListener* l) { m_activationListeners.push_back(l); }
    void removeEntityActivationListener(hkpEntityActivationListener* l) {
        m_activationListeners.erase(std::remove(m_activationListeners.begin(), m_activationListeners.end(), l), m_activationListeners.end());
    }
    void fireDeactivated();
    void fireActivated();

    void addCollisionListener(hkpCollisionListener* l) { m_collisionListeners.push_back(l); }
    void removeCollisionListener(hkpCollisionListener* l) {
        m_collisionListeners.erase(std::remove(m_collisionListeners.begin(), m_collisionListeners.end(), l), m_collisionListeners.end());
    }

    int getNumActions() const { return (int)m_actions.size(); }
    hkpAction* getAction(int i) const { return m_actions[i]; }
    void _internalAddAction(hkpAction* a) { m_actions.push_back(a); }
    void _internalRemoveAction(hkpAction* a) { m_actions.erase(std::remove(m_actions.begin(), m_actions.end(), a), m_actions.end()); }
};

class hkpRigidBody : public hkpEntity {
public:
    btRigidBody* m_bulletBody = nullptr;
    hkpShape* m_shape = nullptr;
    hkpMotion* m_motion = nullptr;
    hkpMotion::MotionType m_motionType = hkpMotion::MOTION_DYNAMIC;
    std::string m_name;
    // Real Havok's getPosition()/getRotation()/getLinearVelocity() return
    // const references into internal motion-state storage; several game
    // call sites bind those to `const hkVector4&`/`const hkQuaternion&`
    // locals, so these shim equivalents cache into a member and return a
    // reference to *that*, rather than to a temporary.
    mutable hkVector4 m_posCache;
    mutable hkVector4 m_velCache;
    mutable hkQuaternion m_rotCache;

    hkpRigidBody() { m_collidable = new hkpCollidable(); m_collidable->m_owner = this; }
    ~hkpRigidBody() override;

    void attachBullet(btRigidBody* body, hkpShape* shape) {
        m_bulletBody = body; m_shape = shape;
        m_collidable->m_shape = shape;
        m_motion = new hkpMotion(body);
        body->setUserPointer(this);
    }

    hkpMotion* getMotion() const { return m_motion; }
    hkpMotion::MotionType getMotionType() const { return m_motionType; }

    const hkVector4& getPosition() const { m_posCache = hkVector4(m_bulletBody->getWorldTransform().getOrigin()); return m_posCache; }
    void setPosition(const hkVector4& p) {
        btTransform t = m_bulletBody->getWorldTransform();
        t.setOrigin(p.v);
        m_bulletBody->setWorldTransform(t);
        m_bulletBody->activate(true);
    }
    const hkQuaternion& getRotation() const { m_rotCache = hkQuaternion(m_bulletBody->getWorldTransform().getRotation()); return m_rotCache; }
    void setRotation(const hkQuaternion& q) {
        btTransform t = m_bulletBody->getWorldTransform();
        t.setRotation(q.q);
        m_bulletBody->setWorldTransform(t);
    }
    const hkVector4& getLinearVelocity() const { m_velCache = hkVector4(m_bulletBody->getLinearVelocity()); return m_velCache; }
    void setLinearVelocity(const hkVector4& v) { m_bulletBody->setLinearVelocity(v.v); m_bulletBody->activate(true); }
    // Overload used by character rigid body: setLinearVelocity(vel, deltaTime)
    void setLinearVelocity(const hkVector4& v, hkReal /*deltaTime*/) { setLinearVelocity(v); }
    void setAngularVelocity(const hkVector4& v) { m_bulletBody->setAngularVelocity(v.v); m_bulletBody->activate(true); }
    hkReal getMass() const { return m_bulletBody->getInvMass() > 0 ? 1.0f / m_bulletBody->getInvMass() : 0.0f; }

    void applyForce(hkReal deltaTime, const hkVector4& force) {
        m_bulletBody->activate(true);
        m_bulletBody->applyCentralImpulse(force.v * deltaTime);
    }
    void applyLinearImpulse(const hkVector4& impulse) {
        m_bulletBody->activate(true);
        m_bulletBody->applyCentralImpulse(impulse.v);
    }

    void setShape(const hkpShape* shape) {
        m_shape = const_cast<hkpShape*>(shape);
        m_collidable->m_shape = m_shape;
        m_bulletBody->setCollisionShape(shape->bullet());
    }

    void setUserData_unused() {}
};

inline hkpRigidBody* hkGetRigidBody(const hkpCollidable* handle) {
    if (!handle || !handle->m_owner) return nullptr;
    if (handle->m_owner->getBroadPhaseType() != hkpWorldObject::BROAD_PHASE_ENTITY) return nullptr;
    return static_cast<hkpRigidBody*>(handle->m_owner);
}

struct hkpRigidBodyCinfo {
    hkReal m_mass = 1.0f;
    hkpShape* m_shape = nullptr;
    hkVector4 m_position;
    hkQuaternion m_rotation;
    hkpMotion::MotionType m_motionType = hkpMotion::MOTION_DYNAMIC;
    hkReal m_friction = 0.5f;
    hkReal m_restitution = 0.0f;
    hkReal m_linearDamping = 0.0f;
    hkReal m_angularDamping = 0.05f;
};

// ===========================================================================
// Physics/Dynamics/Collide -- hkpCollisionListener + event structs.
// Real: driven off Bullet's persistent manifolds each step (see
// hkpWorld::_dispatchCollisionEvents in HavokWorld.cpp).
// ===========================================================================

class hkContactPoint {
public:
    hkVector4 m_position;
    hkVector4 m_normal;
    hkReal m_distance = 0;
    const hkVector4& getPosition() const { return m_position; }
    const hkVector4& getNormal() const { return m_normal; }
    hkReal getDistance() const { return m_distance; }
};

struct hkpProcessCdPoint {
    hkContactPoint m_contact;
};

class hkpProcessCollisionData {
public:
    std::vector<hkpProcessCdPoint> m_points;
    int getNumContactPoints() const { return (int)m_points.size(); }
    hkpProcessCdPoint& getContactPoint(int i) { return m_points[i]; }
};

struct hkpContactPointAddedEvent { hkpCollidable* m_collidableA = nullptr; hkpCollidable* m_collidableB = nullptr; };
struct hkpContactPointConfirmedEvent { hkpCollidable* m_collidableA = nullptr; hkpCollidable* m_collidableB = nullptr; };
struct hkpContactPointRemovedEvent { hkpCollidable* m_collidableA = nullptr; hkpCollidable* m_collidableB = nullptr; };
struct hkpContactProcessEvent {
    hkpCollidable* m_collidableA = nullptr;
    hkpCollidable* m_collidableB = nullptr;
    hkpProcessCollisionData* m_collisionData = nullptr;
};

class hkpCollisionListener {
public:
    virtual ~hkpCollisionListener() {}
    virtual void contactPointAddedCallback(hkpContactPointAddedEvent& e) {}
    virtual void contactPointConfirmedCallback(hkpContactPointConfirmedEvent& e) {}
    virtual void contactPointRemovedCallback(hkpContactPointRemovedEvent& e) {}
    virtual void contactProcessCallback(hkpContactProcessEvent& e) {}
};

// ===========================================================================
// Physics/Dynamics/Action -- hkpAction / hkpUnaryAction.
// Real: hkpWorld drives applyAction() on every registered action each step,
// after Bullet's own stepSimulation -- see docs/porting/havok-compat.md for
// why this shim does not wire actions through btActionInterface directly.
// ===========================================================================

struct hkStepInfo {
    hkReal m_deltaTime = 0;
    hkReal m_invDeltaTime = 0;
};

class hkpPhantom; // fwd

class hkpAction : public hkReferencedObject {
public:
    hkpWorld* m_world = nullptr;
    hkUlong m_userData = 0;

    virtual ~hkpAction() {}
    hkpWorld* getWorld() const { return m_world; }
    void setUserData(hkUlong d) { m_userData = d; }
    hkUlong getUserData() const { return m_userData; }

    virtual void applyAction(const hkStepInfo& stepInfo) = 0;
    virtual hkpAction* clone(const hkArray<hkpEntity*>& newEntities, const hkArray<hkpPhantom*>& newPhantoms) const = 0;
};

class hkpUnaryAction : public hkpAction {
public:
    hkpEntity* m_entity;
    explicit hkpUnaryAction(hkpEntity* entity) : m_entity(entity) {
        if (m_entity) m_entity->_internalAddAction(this);
    }
    hkpRigidBody* getRigidBody() const { return static_cast<hkpRigidBody*>(m_entity); }
    hkpEntity* getEntity() const { return m_entity; }
};

inline void hkpEntity::fireDeactivated() { for (auto* l : m_activationListeners) l->entityDeactivatedCallback(this); }
inline void hkpEntity::fireActivated() { for (auto* l : m_activationListeners) l->entityActivatedCallback(this); }

// ===========================================================================
// Physics/Dynamics/Phantom -- hkpPhantom family. Real: each phantom owns a
// btPairCachingGhostObject added to the dynamics world; hkpWorld diffs the
// ghost's overlapping-pairs list each step and calls
// add/removeOverlappingCollidable() -- this is the hkpPhantom<->btGhostObject
// mapping called out in the task brief.
// ===========================================================================

class hkpPhantom : public hkpWorldObject {
public:
    btPairCachingGhostObject* m_ghost = nullptr;
    hkpWorld* m_world = nullptr;
    std::vector<hkpCollidable*> m_currentOverlaps;

    hkpPhantom() { m_collidable = new hkpCollidable(); m_collidable->m_owner = this; }
    ~hkpPhantom() override { delete m_ghost; }

    BroadPhaseType getBroadPhaseType() const override { return BROAD_PHASE_PHANTOM; }

    virtual void addOverlappingCollidable(hkpCollidable* handle) { m_currentOverlaps.push_back(handle); }
    virtual void removeOverlappingCollidable(hkpCollidable* handle) {
        m_currentOverlaps.erase(std::remove(m_currentOverlaps.begin(), m_currentOverlaps.end(), handle), m_currentOverlaps.end());
    }
};

class hkpAabbPhantom : public hkpPhantom {
public:
    hkAabb m_aabb;
    hkpAabbPhantom(const hkAabb& aabb, int /*group*/) : m_aabb(aabb) {}
    const hkAabb& getAabb() const { return m_aabb; }
    void setAabb(const hkAabb& aabb);
};

class hkpShapePhantom : public hkpPhantom {
public:
    hkpShape* m_shape;
    hkTransform m_transform;
    hkpShapePhantom(const hkpShape* shape, const hkTransform& transform)
        : m_shape(const_cast<hkpShape*>(shape)), m_transform(transform) {
        m_shape->addReference();
        m_collidable->m_shape = m_shape;
    }
    ~hkpShapePhantom() override { m_shape->removeReference(); }
    void setTransform(const hkTransform& t);
    const hkTransform& getTransform() const { return m_transform; }
};

class hkpSimpleShapePhantom : public hkpShapePhantom {
public:
    hkpSimpleShapePhantom(const hkpShape* shape, const hkTransform& transform, int /*group*/ = 0)
        : hkpShapePhantom(shape, transform) {}
};

// ===========================================================================
// Physics/Collide -- ray casting + closest-points query surface used by
// HoverAction (hover raycast) and PlanetGravityAction (closest-points
// gravity direction). Both are real, Bullet-backed.
// ===========================================================================

struct hkpWorldRayCastInput {
    hkVector4 m_from;
    hkVector4 m_to;
};
struct hkpWorldRayCastOutput {
    hkReal m_hitFraction = 1.0f;
    bool m_hit = false;
    hkVector4 m_normal;
    bool hasHit() const { return m_hit; }
};
struct hkpShapeRayCastInput {};
class hkpRayHitCollector {};

class hkpCollisionFilter { public: virtual ~hkpCollisionFilter() {} };

struct hkpCollisionInput {
    hkReal m_tolerance = 0.1f;
};
typedef hkpCollisionInput hkpProcessCollisionInput;

class hkpClosestCdPointCollector {
public:
    bool m_hasHit = false;
    hkContactPoint m_contact;
    bool hasHit() const { return m_hasHit; }
    const hkContactPoint& getHitContact() const { return m_contact; }
};

class hkpCollisionDispatcher {
public:
    typedef void (*GetClosestPointsFunc)(const hkpCollidable&, const hkpCollidable&, const hkpCollisionInput&, hkpClosestCdPointCollector&);
    static void GenericGetClosestPoints(const hkpCollidable& a, const hkpCollidable& b, const hkpCollisionInput& input, hkpClosestCdPointCollector& collector);
    // Real Havok dispatches by (shapeTypeA, shapeTypeB) into a big function
    // table. This shim ignores the shape-type arguments and always returns
    // one generic GJK-based closest-points implementation that works for
    // any convex pair -- see docs/porting/havok-compat.md.
    GetClosestPointsFunc getGetClosestPointsFunc(hkpShapeType, hkpShapeType) const { return &GenericGetClosestPoints; }
};

// hkpBroadPhase: exposes the set of collidables hkpWorld currently knows
// about, so hkpSimpleWorldRayCaster can walk it.
class hkpBroadPhase {
public:
    std::vector<hkpCollidable*>* m_all = nullptr;
};

struct hkpTypedBroadPhaseHandle {
    hkpCollidable* m_collidable;
    void* getOwner() const { return m_collidable; }
};
typedef hkpTypedBroadPhaseHandle hkpBroadPhaseHandle;

class hkpBroadPhaseCastCollector {
public:
    virtual ~hkpBroadPhaseCastCollector() {}
    virtual hkReal addBroadPhaseHandle(const hkpBroadPhaseHandle* handle, int castIndex) = 0;
};

// hkpSimpleWorldRayCaster: base ray caster the game subclasses
// (PlanetRayCastCallback) to filter by entity type before doing the real
// intersection test. Real: addBroadPhaseHandle() does a genuine Bullet
// convex-sweep/ray test against that one collidable's shape.
class hkpSimpleWorldRayCaster : public hkpBroadPhaseCastCollector {
public:
    hkpWorldRayCastInput m_input;
    hkpWorldRayCastOutput* m_output = nullptr;

    virtual hkReal addBroadPhaseHandle(const hkpBroadPhaseHandle* handle, int castIndex) override;

    void castRay(hkpBroadPhase& broadphase, const hkpWorldRayCastInput& input, hkpCollisionFilter* /*filter*/, hkpWorldRayCastOutput& output) {
        m_input = input;
        m_output = &output;
        if (!broadphase.m_all) return;
        for (size_t i = 0; i < broadphase.m_all->size(); ++i) {
            hkpTypedBroadPhaseHandle h; h.m_collidable = (*broadphase.m_all)[i];
            if (addBroadPhaseHandle(&h, (int)i) <= 0.0f) break;
        }
    }
};

// ===========================================================================
// Physics/Collide/Filter -- hkpGroupFilterSetup. Real Havok's collision
// filter tables aren't exercised meaningfully by this game (no group-based
// exclusion rules found in the grep of game code); no-op setup function is
// honest.
// ===========================================================================
namespace hkpGroupFilterSetup {
    inline void setupCollisionFilter(hkpCollisionFilter*) { /* no-op: no group rules used by this game */ }
}

// ===========================================================================
// Physics/Utilities/Serialize -- hkpPhysicsData / hkpHavokSnapshot.
// TODO(phaseB): .hkx loading. See docs/porting/havok-compat.md. This is the
// single biggest Phase A stub: real levels + hovercraft collision geometry
// live in proprietary .hkx files that this shim does not parse. `load()`
// logs once and returns an *empty* hkpPhysicsData/hkpWorld pair so the game
// keeps running (with no static collision geometry) instead of crashing.
// ===========================================================================

namespace hkPackfileReader {
    class AllocatedData : public hkReferencedObject {
    public:
        void disableDestructors() {}
        void callDestructors() {}
    };
}

struct hkpWorldCinfo {
    enum SimulationType { SIMULATION_TYPE_SINGLE_THREADED, SIMULATION_TYPE_MULTITHREADED, SIMULATION_TYPE_CONTINUOUS };
    enum BroadPhaseBorderBehaviour { BROADPHASE_BORDER_ASSERT, BROADPHASE_BORDER_REMOVE_ENTITY, BROADPHASE_BORDER_FIX_ENTITY };

    SimulationType m_simulationType = SIMULATION_TYPE_SINGLE_THREADED;
    bool m_processActionsInSingleThread = true;
    hkAabb m_broadPhaseWorldAabb;
    BroadPhaseBorderBehaviour m_broadPhaseBorderBehaviour = BROADPHASE_BORDER_ASSERT;
    hkVector4 m_gravity = hkVector4(0, -9.8f, 0);
};

class hkpPhysicsData : public hkReferencedObject {
public:
    hkpWorldCinfo m_cinfo;
    // Populated either by Phase B's CollisionProvider (see below), or left
    // empty exactly as Phase A if no provider is registered -- see
    // hkpHavokSnapshot::load().
    std::unordered_map<std::string, hkpRigidBody*> m_namedBodies;

    // Releases this object's ownership share of every named body (see
    // hkpHavokSnapshot::load(): each is constructed with the implicit
    // hkReferencedObject refcount of 1, held by this map). Bodies also
    // added to a world by createWorld() picked up their own extra reference
    // there via hkpWorld::addEntity(), so this does not free those -- it
    // only frees bodies nobody else ever referenced (e.g. a hovercraft
    // hull snapshot's body, whose *shape* gets addReference()'d and reused
    // by HavokHovercraft::load() but whose body itself is never added to
    // any hkpWorld).
    ~hkpPhysicsData() override {
        for (auto& kv : m_namedBodies) {
            if (kv.second) kv.second->removeReference();
        }
    }

    void setWorldCinfo(const hkpWorldCinfo* cinfo) { m_cinfo = *cinfo; }
    hkpWorld* createWorld();
    hkpRigidBody* findRigidBodyByName(const char* name) {
        auto it = m_namedBodies.find(name ? name : "");
        if (it == m_namedBodies.end()) {
            havok_compat::todoPhaseBOnce("hkpPhysicsData::findRigidBodyByName",
                std::string("no rigid body named '") + (name ? name : "") +
                "' -- .hkx collision data is not loaded (no CollisionProvider registered, or the name does not appear in the matching .scene; see docs/porting/phase-b-collision.md)");
            return nullptr;
        }
        return it->second;
    }
};

namespace hkpHavokSnapshot {
    // Wraps a provider-owned btCollisionShape in an hkpShape and constructs
    // a mass-0 ("fixed") hkpRigidBody at the given world transform, exactly
    // like a body real Havok's snapshot loader would have deserialized out
    // of the .hkx. Named helper (not a lambda) so it's visible from the
    // single call site below and easy to find when reading this file.
    inline hkpRigidBody* buildNamedBody(const havok_compat::ReconstructedBody& rb) {
        hkpShape* shapeWrapper = new hkpShape();
        shapeWrapper->m_bulletShape = rb.shape;
        shapeWrapper->m_type = rb.isStatic ? HK_SHAPE_MESH : HK_SHAPE_CONVEX_VERTICES;

        btVector3 localInertia(0, 0, 0); // mass 0 => static, no inertia to compute
        auto* motionState = new btDefaultMotionState(rb.worldTransform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(0.0f, motionState, rb.shape, localInertia);
        auto* bulletBody = new btRigidBody(rbInfo);

        hkpRigidBody* body = new hkpRigidBody();
        body->attachBullet(bulletBody, shapeWrapper);
        body->m_motionType = hkpMotion::MOTION_FIXED;
        body->m_name = rb.name;
        return body;
    }

    inline hkpPhysicsData* load(hkStreamReader* reader, hkPackfileReader::AllocatedData** loadedData) {
        *loadedData = new hkPackfileReader::AllocatedData();
        hkpPhysicsData* data = new hkpPhysicsData();

        havok_compat::CollisionProvider* provider = havok_compat::collisionProvider();
        std::string path = reader ? reader->path : "";
        // Unconditional (not todoPhaseBOnce-deduplicated) entry trace: this
        // whole function runs on HavokThread.cpp's dedicated physics thread,
        // where an uncaught exception is reported via a modal MessageBox
        // nobody in an automated test run can click, permanently deadlocking
        // the process with no other log output -- see
        // docs/porting/phase-b-collision.md's verification section. This
        // line existing at all is the difference between "silent deadlock"
        // and "we know exactly which .hkx it was loading when it happened".
        std::cerr << "[hu_havok_compat] hkpHavokSnapshot::load(" << path << ") provider="
                  << (provider ? "registered" : "none") << std::endl;
        if (provider) {
            std::vector<havok_compat::ReconstructedBody> bodies;
            if (provider->build(path, bodies)) {
                for (auto& rb : bodies) {
                    data->m_namedBodies[rb.name] = buildNamedBody(rb);
                }
                return data;
            }
            havok_compat::todoPhaseBOnce(("hkpHavokSnapshot::load:" + path).c_str(),
                std::string("CollisionProvider::build() failed for '") + path +
                "' -- returning an empty hkpPhysicsData; see docs/porting/phase-b-collision.md");
            return data;
        }

        havok_compat::todoPhaseBOnce("hkpHavokSnapshot::load",
            std::string(".hkx snapshot loading is not implemented (path: ") + path +
            "). Returning an empty hkpPhysicsData; see docs/porting/havok-compat.md and "
            "docs/porting/phase-b-collision.md -- no CollisionProvider is registered "
            "(hu_collision not linked in, or setCollisionProvider() never called).");
        return data;
    }
}

// ===========================================================================
// Physics/Utilities/VisualDebugger -- hkpPhysicsContext. Dev-time-only;
// same rationale as hkVisualDebugger above.
// ===========================================================================

class hkpPhysicsContext : public hkProcessContext, public hkReferencedObject {
public:
    static void registerAllPhysicsProcesses() {}
    void addWorld(hkpWorld*) {}
    void syncTimers(hkJobThreadPool*) {}
};

// ===========================================================================
// Physics/Dynamics/World -- hkpWorld. Real: wraps a btDiscreteDynamicsWorld.
// ===========================================================================

class hkpWorld : public hkReferencedObject {
public:
    btDefaultCollisionConfiguration* m_collisionConfig;
    btCollisionDispatcher* m_dispatcher;
    btBroadphaseInterface* m_broadphasePair;
    btSequentialImpulseConstraintSolver* m_solver;
    btDiscreteDynamicsWorld* m_bullet;

    std::vector<hkpEntity*> m_entities;
    std::vector<hkpPhantom*> m_phantoms;
    std::vector<hkpAction*> m_actions;
    std::vector<hkpCollidable*> m_allCollidables; // for hkpBroadPhase::m_all
    hkpBroadPhase m_broadPhase;
    hkpCollisionDispatcher m_collisionDispatcher;
    hkpCollisionFilter m_collisionFilterObj;
    hkpCollisionInput m_collisionInput;

    explicit hkpWorld(const hkpWorldCinfo& cinfo) {
        m_collisionConfig = new btDefaultCollisionConfiguration();
        m_dispatcher = new btCollisionDispatcher(m_collisionConfig);
        m_broadphasePair = new btDbvtBroadphase();
        m_solver = new btSequentialImpulseConstraintSolver();
        m_bullet = new btDiscreteDynamicsWorld(m_dispatcher, m_broadphasePair, m_solver, m_collisionConfig);
        m_bullet->setGravity(cinfo.m_gravity.v);
        m_bullet->getPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
        m_broadPhase.m_all = &m_allCollidables;
    }
    ~hkpWorld() override {
        delete m_bullet; delete m_solver; delete m_broadphasePair; delete m_dispatcher; delete m_collisionConfig;
    }

    // markForWrite/unmarkForWrite: real Havok uses these to guard
    // multithreaded access from the simulation thread pool. This shim runs
    // single-threaded (see the job-queue note above), so they are harmless
    // no-ops -- kept only so the game's exact call pattern compiles.
    void markForWrite() {}
    void unmarkForWrite() {}
    void registerWithJobQueue(hkJobQueue*) {}

    void addEntity(hkpRigidBody* body);
    void removeEntity(hkpRigidBody* body);
    void addPhantom(hkpPhantom* phantom);
    void removePhantom(hkpPhantom* phantom);
    void addAction(hkpAction* action) { action->m_world = this; action->addReference(); m_actions.push_back(action); }
    void removeAction(hkpAction* action) {
        m_actions.erase(std::remove(m_actions.begin(), m_actions.end(), action), m_actions.end());
        action->removeReference();
    }

    hkpBroadPhase* getBroadPhase() { return &m_broadPhase; }
    hkpCollisionFilter* getCollisionFilter() { return &m_collisionFilterObj; }
    hkpCollisionDispatcher* getCollisionDispatcher() { return &m_collisionDispatcher; }
    const hkpCollisionInput* getCollisionInput() const { return &m_collisionInput; }

    // Steps Bullet, then drives registered Havok actions/phantoms/collision
    // listeners on top -- see the mapping note near hkpAction above.
    void stepMultithreaded(hkJobQueue*, hkJobThreadPool*, hkReal deltaTime);
    void _dispatchCollisionEvents();
    void _updatePhantomOverlaps();
};

inline hkpRigidBody::~hkpRigidBody() {
    delete m_motion;
    delete m_collidable;
    // m_bulletBody is owned/deleted by hkpWorld::removeEntity's caller contract:
    // real Havok callers remove the entity from the world before releasing
    // their last reference, matching AbstractHavokWorld/HavokHovercraft's
    // actual usage (removeEntity() then removeReference()). We delete the
    // Bullet body here to avoid leaking it.
    delete m_bulletBody;
}

inline void hkpAabbPhantom::setAabb(const hkAabb& aabb) {
    m_aabb = aabb;
    if (m_ghost) {
        hkVector4 center; center.setAdd4(aabb.m_min, aabb.m_max); center.mul4(0.5f);
        btTransform t; t.setIdentity(); t.setOrigin(center.v);
        m_ghost->setWorldTransform(t);
    }
}

inline void hkpShapePhantom::setTransform(const hkTransform& t) {
    m_transform = t;
    if (m_ghost) m_ghost->setWorldTransform(t.t);
}

// ===========================================================================
// Physics/Utilities/CharacterControl -- hkpCharacterRigidBody + state
// machine. See docs/porting/havok-compat.md: Havok's real character state
// machine (spring-based ground/air projection, slope handling, step
// climbing) has no clean Bullet equivalent, so this is a SIMPLIFIED
// reimplementation, not a faithful port: each state stores speed/gain/
// max-linear-acceleration knobs (matching the game's own tuning calls) and
// hkpCharacterContext::update() exponentially drives velocity toward
// forward*inputUD*speed + up*verticalComponent using those knobs. This is
// enough for the hovercraft to actually accelerate/turn/stop under game
// control, but it will *feel* different from real Havok's spring dynamics
// (see the gameplay-feel-divergences section of the doc).
// ===========================================================================

enum {
    HK_CHARACTER_ON_GROUND = 0,
    HK_CHARACTER_IN_AIR = 1,
    HK_CHARACTER_JUMPING = 2,
    HK_CHARACTER_CLIMBING = 3
};

struct hkpSurfaceInfo {
    bool m_supported = false;
    hkVector4 m_surfaceNormal;
    hkVector4 m_surfaceVelocity;
};

struct hkpCharacterInput {
    bool m_atLadder = false;
    hkReal m_inputLR = 0;
    hkReal m_inputUD = 0;
    bool m_wantJump = false;
    hkVector4 m_up;
    hkVector4 m_forward;
    hkStepInfo m_stepInfo;
    hkVector4 m_characterGravity;
    hkVector4 m_velocity;
    hkVector4 m_position;
    hkpSurfaceInfo m_surfaceInfo;
};
struct hkpCharacterOutput {
    hkVector4 m_velocity;
};

class hkpCharacterState : public hkReferencedObject {
public:
    hkReal m_speed = 5.0f;
    hkReal m_gain = 1.0f;
    hkReal m_maxLinearAcceleration = 50.0f;
    bool m_disableHorizontalProjection = false;

    void setSpeed(hkReal s) { m_speed = s; }
    void setGain(hkReal g) { m_gain = g; }
    void setMaxLinearAcceleration(hkReal a) { m_maxLinearAcceleration = a; }
    void setDisableHorizontalProjection(bool v) { m_disableHorizontalProjection = v; }

    virtual void handle(const hkpCharacterInput& in, hkpCharacterOutput& out) {
        // Simplified: drive current velocity toward the desired forward
        // speed + preserve/append gravity along "down", clamped by gain and
        // max acceleration. See class-level doc comment above.
        hkVector4 desired;
        desired.setMul4(in.m_inputUD * m_speed, in.m_forward);
        hkVector4 delta = desired - in.m_velocity;
        hkReal dt = in.m_stepInfo.m_deltaTime;
        hkReal maxDelta = m_maxLinearAcceleration * dt;
        hkReal deltaLen = delta.length3();
        if (deltaLen > maxDelta && deltaLen > 1e-6f) {
            delta.mul4(maxDelta / deltaLen);
        }
        out.m_velocity = in.m_velocity + delta;
        out.m_velocity = out.m_velocity + hkVector4(in.m_characterGravity.v * dt);
    }
};
class hkpCharacterStateOnGround : public hkpCharacterState {};
class hkpCharacterStateInAir : public hkpCharacterState {};
class hkpCharacterStateJumping : public hkpCharacterState {};
class hkpCharacterStateClimbing : public hkpCharacterState {};

class hkpCharacterStateManager : public hkReferencedObject {
public:
    std::unordered_map<int, hkpCharacterState*> m_states;
    void registerState(hkpCharacterState* state, int id) {
        state->addReference();
        m_states[id] = state;
    }
    ~hkpCharacterStateManager() override {
        for (auto& kv : m_states) kv.second->removeReference();
    }
};

class hkpCharacterContext : public hkReferencedObject {
public:
    enum CharacterType { HK_CHARACTER_RIGIDBODY, HK_CHARACTER_PROXY };
    hkpCharacterStateManager* m_manager;
    int m_currentState;
    CharacterType m_type = HK_CHARACTER_RIGIDBODY;
    bool m_filterEnable = true;
    hkReal m_filterA = 0, m_filterB = 0, m_filterC = 0;

    hkpCharacterContext(hkpCharacterStateManager* manager, int initialState)
        : m_manager(manager), m_currentState(initialState) {
        m_manager->addReference();
    }
    ~hkpCharacterContext() override { m_manager->removeReference(); }

    void setCharacterType(CharacterType t) { m_type = t; }
    void setFilterEnable(bool v) { m_filterEnable = v; }
    void setFilterParameters(hkReal a, hkReal b, hkReal c) { m_filterA = a; m_filterB = b; m_filterC = c; }

    void update(const hkpCharacterInput& in, hkpCharacterOutput& out) {
        int state = in.m_surfaceInfo.m_supported ? HK_CHARACTER_ON_GROUND : HK_CHARACTER_IN_AIR;
        auto it = m_manager->m_states.find(state);
        if (it == m_manager->m_states.end()) it = m_manager->m_states.find(m_currentState);
        if (it != m_manager->m_states.end()) it->second->handle(in, out);
        else out.m_velocity = in.m_velocity;
        m_currentState = state;
    }
};

class hkpCharacterRigidBodyListener : public hkReferencedObject {
public:
    virtual ~hkpCharacterRigidBodyListener() {}
};

struct hkpCharacterRigidBodyCinfo {
    hkReal m_mass = 1.0f;
    hkpShape* m_shape = nullptr;
    hkReal m_maxForce = 1000.0f;
    hkVector4 m_position;
    hkQuaternion m_rotation;
    hkReal m_maxLinearVelocity = 100.0f;
    hkReal m_maxSpeedForSimplexSolver = 100.0f;
    hkReal m_maxSlope = 0.785f;
    hkReal m_friction = 0.5f;
};

// hkpCharacterRigidBody: owns a real hkpRigidBody (dynamic, damped) and
// forwards motion queries/commands to it; checkSupport() does a genuine
// downward raycast against the owning hkpWorld to find ground distance.
class hkpCharacterRigidBody : public hkReferencedObject {
public:
    hkpRigidBody* m_rigidBody = nullptr;
    hkVector4 m_up = hkVector4(0, 1, 0);
    hkpCharacterRigidBodyListener* m_listener = nullptr;
    hkpWorld* m_world = nullptr; // set post-hoc by whoever adds m_rigidBody to a world; see HavokHovercraft::load

    explicit hkpCharacterRigidBody(const hkpCharacterRigidBodyCinfo& info);
    ~hkpCharacterRigidBody() override { if (m_listener) m_listener->removeReference(); }

    hkpRigidBody* getRigidBody() const { return m_rigidBody; }
    void setListener(hkpCharacterRigidBodyListener* l) { l->addReference(); m_listener = l; }

    // Real hkpCharacterRigidBody forwards these directly (in addition to
    // exposing getRigidBody() for callers that want the full rigid body).
    const hkVector4& getPosition() const { return m_rigidBody->getPosition(); }
    const hkVector4& getLinearVelocity() const { return m_rigidBody->getLinearVelocity(); }

    void setLinearVelocity(const hkVector4& v, hkReal deltaTime) { m_rigidBody->setLinearVelocity(v, deltaTime); }
    void setAngularVelocity(const hkVector4& v) { m_rigidBody->setAngularVelocity(v); }

    void checkSupport(const hkStepInfo& stepInfo, hkpSurfaceInfo& surfaceInfo);
};

// ===========================================================================
// Compat-only helper the game does not call directly, but which
// hkpWorld::_updatePhantomOverlaps()/stepMultithreaded() need: exposed here
// so HavokAll.cpp's out-of-line definitions can see the full class bodies
// above. (Everything below this point is implementation, not API surface.)
// ===========================================================================
