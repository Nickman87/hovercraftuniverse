// ZoidCom-compatible shim: the process-wide ZoidCom class.
//
// Real behavior: owns the process-wide ENet library lifetime
// (enet_initialize()/enet_deinitialize()), reference counted so that
// creating/destroying multiple ZoidCom objects (shouldn't normally happen,
// but is cheap to guard) or having none at all doesn't crash.
#include "zoidcom_shim_internal.h"
#include <enet/enet.h>
#include <chrono>
#include <cstdlib>
#include <mutex>
#include <thread>

namespace zshim {

namespace {
std::mutex gLogMutex;
void (*gLogFunc)(const char*) = NULL;
FILE* gLogFile = NULL;
bool gOwnsLogFile = false;

void* (*gAllocator)(size_t) = NULL;
void (*gDeallocator)(void*) = NULL;

std::mutex gTodoMutex;
std::set<std::string>* gTodoSeen = NULL;
}

void logf(const char* fmt, ...) {
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    buf[sizeof(buf) - 1] = '\0';

    std::lock_guard<std::mutex> lock(gLogMutex);
    if (gLogFunc) {
        gLogFunc(buf);
    } else if (gLogFile) {
        fprintf(gLogFile, "%s\n", buf);
        fflush(gLogFile);
    } else {
        fprintf(stderr, "[ZoidComShim] %s\n", buf);
    }
}

void todoPhaseBOnce(const char* key, const char* detail) {
    std::lock_guard<std::mutex> lock(gTodoMutex);
    if (!gTodoSeen) {
        gTodoSeen = new std::set<std::string>();
    }
    if (gTodoSeen->find(key) != gTodoSeen->end()) {
        return;
    }
    gTodoSeen->insert(key);
    logf("TODO(phaseB): %s -- %s (this message is logged only once)", key, detail);
}

void* zalloc(size_t size) {
    if (gAllocator) return gAllocator(size);
    return malloc(size);
}

void zfree(void* ptr) {
    if (!ptr) return;
    if (gDeallocator) { gDeallocator(ptr); return; }
    free(ptr);
}

zU32 currentTimeMillis() {
    using namespace std::chrono;
    return (zU32) duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

// Exposed for ZoidCom's ctors/overrideMemoryHandlers, defined here to keep
// the small set of file-scope globals above private to this translation unit.
void setLogFunc(void (*f)(const char*)) {
    std::lock_guard<std::mutex> lock(gLogMutex);
    gLogFunc = f;
    if (gLogFile && gOwnsLogFile) { fclose(gLogFile); }
    gLogFile = NULL;
    gOwnsLogFile = false;
}

void setLogFile(const char* path) {
    std::lock_guard<std::mutex> lock(gLogMutex);
    gLogFunc = NULL;
    if (gLogFile && gOwnsLogFile) { fclose(gLogFile); }
    gLogFile = NULL;
    gOwnsLogFile = false;
    if (path) {
        gLogFile = fopen(path, "a");
        gOwnsLogFile = (gLogFile != NULL);
    }
}

void setMemoryHandlers(void*(*alloc)(size_t), void(*dealloc)(void*)) {
    gAllocator = alloc;
    gDeallocator = dealloc;
}

} // namespace zshim

class ZoidCom_Private {
public:
    int refcount_placeholder; // kept for ABI parity / future use
};

namespace {
    // enet_initialize()/enet_deinitialize() must be balanced process-wide;
    // several ZCom_Control objects (and potentially several ZoidCom
    // instances, though the real API says "only once") may coexist.
    int gEnetRefCount = 0;
}

namespace zshim {
bool enetAcquire() {
    if (gEnetRefCount == 0) {
        if (enet_initialize() != 0) {
            logf("ENet enet_initialize() failed");
            return false;
        }
    }
    gEnetRefCount++;
    return true;
}

void enetRelease() {
    if (gEnetRefCount <= 0) return;
    gEnetRefCount--;
    if (gEnetRefCount == 0) {
        enet_deinitialize();
    }
}
}

ZoidCom::ZoidCom() : m_priv(new ZoidCom_Private()) {
    zshim::setLogFunc(NULL);
}

ZoidCom::ZoidCom(const char* _logfile) : m_priv(new ZoidCom_Private()) {
    zshim::setLogFile(_logfile);
}

ZoidCom::ZoidCom(void (*_logfunc)(const char*)) : m_priv(new ZoidCom_Private()) {
    zshim::setLogFunc(_logfunc);
}

ZoidCom::~ZoidCom() {
    Clear();
    delete m_priv;
}

bool ZoidCom::Init() {
    return zshim::enetAcquire();
}

void ZoidCom::Clear(void) {
    zshim::enetRelease();
}

void ZoidCom::setConnectionTimeout(zU32 _timeout) {
    // TODO(phaseB): ENet's own peer timeout (enet_peer_timeout) is set per
    // peer at connect time; this shim does not yet plumb a configurable
    // default through to Control.cpp's ZCom_initSockets()/ZCom_Connect().
    zshim::todoPhaseBOnce("ZoidCom::setConnectionTimeout",
        "connection timeout is not yet plumbed through to ENet's peer timeout settings");
}

void ZoidCom::setResendTimeout(zU32 _timeout) {
    zshim::todoPhaseBOnce("ZoidCom::setResendTimeout",
        "resend timeout has no ENet equivalent wired up yet");
}

void ZoidCom::overrideMemoryHandlers(void*(*_allocator)(size_t), void(*_deallocator)(void*)) {
    zshim::setMemoryHandlers(_allocator, _deallocator);
}

void ZoidCom::setLogLevel(zU8 _level) {
    // Real but trivial: the shim doesn't have multiple verbosity tiers, so
    // this is accepted and ignored rather than stubbed-with-warning (no
    // game logic depends on its effect, only that it compiles and doesn't
    // crash).
    (void) _level;
}

zU32 ZoidCom::getTime() {
    return zshim::currentTimeMillis();
}

void ZoidCom::Sleep(zU32 _msecs) {
    // Portable sleep via <thread>; avoids pulling <windows.h> into this
    // header-light translation unit (and the name clash that would cause
    // with this very method, since unqualified lookup inside a member
    // function definition also searches the class scope).
    std::this_thread::sleep_for(std::chrono::milliseconds(_msecs));
}
