// PhysicsDiag.h -- reusable, off-by-default diagnostic logging for the Havok
// shim's physics queries (GenericGetClosestPoints and friends).
//
// This is permanent, intentionally-shipped diagnostic support, not
// scaffolding to be deleted later: physics bugs in this shim (bad contact
// normals, wrong triangle picked, degenerate results) are exactly the kind
// that only show up in-engine, under real gameplay, and are expensive to
// re-instrument from scratch each time. Keeping a cheap, always-compiled,
// env-var-gated logging hook means the next investigation just sets
// HU_PHYSICS_DIAG and re-runs, instead of re-adding prints and rebuilding.
//
// Disabled by default and effectively free when disabled: every call site
// checks a function-local static bool (computed once from the environment)
// before doing any work, so a disabled diagLog() call is a single branch.
#pragma once

#include <string>
#include <mutex>
#include <fstream>
#include <cstdlib>
#include <atomic>

#if defined(_WIN32)
#include <process.h> // _getpid()
#else
#include <unistd.h> // getpid()
#endif

namespace havok_compat {

// True iff the HU_PHYSICS_DIAG environment variable is set to a non-empty
// value. Computed once (the getenv result cannot change under us mid-run).
inline bool physicsDiagEnabled() {
    static const bool enabled = [] {
        const char* v = std::getenv("HU_PHYSICS_DIAG");
        return v != nullptr && v[0] != '\0';
    }();
    return enabled;
}

// Appends one line to physics-diag-<pid>.log in the process's current
// working directory, prefixed with an incrementing counter. Mutex-guarded
// and flushed per line so a crash (this is physics-crash-adjacent
// diagnostics, after all) doesn't lose the last few lines in a buffer.
//
// The PID is baked into the filename because single-player hosts client and
// server ZCom_Control instances in one process (so a "the" process is
// already ambiguous) while two-process test mode has two OS processes
// writing logs side by side -- see CLAUDE.md. A single hardcoded filename
// would silently interleave or clobber between the two.
inline void diagLog(const std::string& message) {
    if (!physicsDiagEnabled()) {
        return;
    }

    static std::mutex mtx;
    static std::ofstream* stream = nullptr;
    static long long counter = 0;

    std::lock_guard<std::mutex> lock(mtx);
    if (!stream) {
#if defined(_WIN32)
        const unsigned long pid = static_cast<unsigned long>(_getpid());
#else
        const unsigned long pid = static_cast<unsigned long>(getpid());
#endif
        std::string filename = "physics-diag-" + std::to_string(pid) + ".log";
        stream = new std::ofstream(filename, std::ios::out | std::ios::app);
    }

    ++counter;
    (*stream) << counter << ": " << message << std::endl;
    stream->flush();
}

} // namespace havok_compat
