/****************************************
* zoidcom_shim_internal.h
* Internal-only helpers shared across the ZoidCom compat shim's .cpp files.
* NOT part of the public API surface (game code never includes this).
*****************************************/

#ifndef _ZOIDCOM_SHIM_INTERNAL_H_
#define _ZOIDCOM_SHIM_INTERNAL_H_

#include <zoidcom/zoidcom.h>
#include <cstdio>
#include <cstdarg>
#include <string>
#include <set>

namespace zshim {

/// Routes to the ZoidCom log callback/file set up via the ZoidCom ctor, or
/// stderr if none was configured. Used for both ordinary log lines and
/// Phase B TODO warnings.
void logf(const char* fmt, ...);

/// Logs a "not implemented yet, this is a Phase B item" warning exactly
/// once per distinct `key` for the lifetime of the process, so hot paths
/// (e.g. per-tick replication stubs) don't spam the log.
void todoPhaseBOnce(const char* key, const char* detail);

/// Malloc/free that honor ZoidCom::overrideMemoryHandlers() if set, else
/// fall back to the CRT malloc/free. Used by all ZCOM_API classes' custom
/// operator new/delete.
void* zalloc(size_t size);
void zfree(void* ptr);

/// Shared millisecond clock (used by ZoidCom::getTime() and stats).
zU32 currentTimeMillis();

} // namespace zshim

#endif
