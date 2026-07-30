// Out-of-line implementation for compat/skyx. See
// include/skyx_compat/SkyXAll.h and docs/porting/skyx-compat.md.

#include "skyx_compat/SkyXAll.h"

#include <iostream>
#include <mutex>
#include <string>
#include <unordered_set>

namespace skyx_compat {

void todoPhaseBOnce(const char* site, const std::string& msg) {
    static std::mutex mtx;
    static std::unordered_set<std::string> seen;
    std::lock_guard<std::mutex> lock(mtx);
    std::string key = site ? site : "";
    if (seen.count(key)) return;
    seen.insert(key);
    std::cerr << "[hu_skyx_compat] TODO(phaseB): " << msg << std::endl;
}

} // namespace skyx_compat
