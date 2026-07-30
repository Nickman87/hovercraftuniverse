# third_party/CMakeLists.txt
#
# Provides Lua 5.1 and LuaBind (built against that Lua 5.1) as static-library
# CMake targets for the modern Hovercraft Universe build, since neither is
# available in a usable form from vcpkg:
#   - vcpkg's "lua" port tracks modern Lua (5.5 at time of writing), which is
#     source-incompatible with this 2010 game's Lua-5.1-era scripts and with
#     LuaBind (which needs the 5.1 C API, e.g. lua_open()/luaL_newstate()
#     compat, lua_Number as double by default, etc.)
#   - there is no vcpkg port for LuaBind at all.
#
# Targets exposed to the parent build:
#   hu_lua      - Lua 5.1.5, built from the official upstream source tarball.
#   hu_luabind  - LuaBind (ryzom/luabind fork), linked against hu_lua + Boost.
#
# See docs/porting/lua-luabind.md for the full rationale, pinned versions,
# and API-compatibility notes.

cmake_minimum_required(VERSION 3.15)
project(hu_third_party LANGUAGES C CXX)

include(FetchContent)

# ---------------------------------------------------------------------------
# Lua 5.1.5
# ---------------------------------------------------------------------------
# Lua ships no CMakeLists.txt of its own, so we patch our own in
# (third_party/lua/CMakeLists.txt.in) after extracting the tarball.
FetchContent_Declare(
    hu_lua_src
    URL https://www.lua.org/ftp/lua-5.1.5.tar.gz
    URL_HASH SHA256=2640fc56a795f29d28ef15e13c34a47e223960b0240e8cb0a82d9b0738695333
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    PATCH_COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_SOURCE_DIR}/lua/CMakeLists.txt.in"
        "<SOURCE_DIR>/CMakeLists.txt"
    UPDATE_DISCONNECTED TRUE
)
FetchContent_MakeAvailable(hu_lua_src)
# hu_lua target now exists, defined by the patched-in CMakeLists.txt above.

# ---------------------------------------------------------------------------
# LuaBind (ryzom/luabind fork), pinned to a specific commit.
# ---------------------------------------------------------------------------
# Chosen over the plain upstream luabind/luabind and other forks because:
#   - it is a maintained fork of rpavlik/luabind (itself the CMake-ified,
#     MSVC-friendly continuation of the original 0.9 luabind) that keeps full
#     Lua 5.1 support behind a `#if LUA_VERSION_NUM < 502` compat shim
#     (src/lua51compat.cpp), while adding newer-Lua and C++11 support;
#   - this exact commit is already proven to configure/build cleanly under
#     MSVC + vcpkg's CMake toolchain (see the nimetu/vcpkg_luabind_example
#     overlay port, which builds this same SHA via
#     vcpkg_cmake_configure/vcpkg_cmake_install);
#   - it keeps the classic luabind:: API surface (module/class_/def/
#     constructor/def_readwrite/operator overloads/globals/object/
#     call_function<T>) that this game's ScriptWrapper/OgreLuaBindings code
#     uses, unlike unrelated "luabind"-named projects that are different
#     libraries entirely.
#
# LuaBind's own CMakeLists.txt does `find_package(Lua)` / `find_package(Boost
# REQUIRED)` unless LUA_FOUND is already set. We point it at hu_lua instead of
# a system/vcpkg Lua so it builds against the same Lua 5.1 as the rest of the
# game, and let it find Boost normally via the vcpkg toolchain.
set(LUA_FOUND TRUE)
get_target_property(HU_LUA_INCLUDE_DIRS hu_lua INTERFACE_INCLUDE_DIRECTORIES)
set(LUA_INCLUDE_DIRS ${HU_LUA_INCLUDE_DIRS})
set(LUA_LIBRARIES hu_lua)

set(LUABIND_INSTALL OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)

# luabind's CMakeLists.txt only adds Boost's include dir via a
# directory-scoped include_directories(AFTER SYSTEM ...) inside its own
# add_subdirectory() scope, so it never reaches consumers outside that
# directory (e.g. our own smoke test, and eventually the game's Scripting
# project) even though luabind's public headers (e.g.
# luabind/detail/has_get_pointer.hpp) themselves include Boost headers.
# Resolve Boost here too so we can propagate Boost_INCLUDE_DIRS to the
# hu_luabind INTERFACE below.
find_package(Boost REQUIRED)

FetchContent_Declare(
    hu_luabind_src
    GIT_REPOSITORY https://github.com/ryzom/luabind.git
    GIT_TAG 0ae9bd6e40fe6c70e9d032ff096370929f58c143
)
# luabind's own CMakeLists.txt declares
# `cmake_minimum_required(VERSION 3.0.0)`, which recent CMake (>= 4.0)
# refuses to evaluate at all ("Compatibility with CMake < 3.5 has been
# removed"). We are not modifying luabind's vendored CMakeLists.txt (it is
# fetched source, not ours to patch beyond what's needed), so tell CMake to
# evaluate it as if it had requested 3.5 policies instead.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
FetchContent_MakeAvailable(hu_luabind_src)
unset(CMAKE_POLICY_VERSION_MINIMUM)

# luabind's own CMakeLists.txt only adds its headers via the old
# directory-level include_directories(BEFORE ...), not
# target_include_directories(), so nothing propagates to consumers via
# target_link_libraries() alone. Fix that up here: consumers need both the
# source root (for "luabind/luabind.hpp" etc.) and the binary dir (for the
# generated "luabind/build_information.hpp").
target_include_directories(luabind INTERFACE
    "${hu_luabind_src_SOURCE_DIR}"
    "${hu_luabind_src_BINARY_DIR}"
    ${Boost_INCLUDE_DIRS})

# This defines a target literally named "luabind" (see
# hu_luabind_src-src/src/CMakeLists.txt). Expose it under our own name so the
# parent build only ever needs to know about hu_lua / hu_luabind.
add_library(hu_luabind ALIAS luabind)


# luabind's own src/CMakeLists.txt already links `luabind` against
# ${LUA_LIBRARIES} (== hu_lua, set above) using the old plain-signature form
# of target_link_libraries(); mixing plain and keyword (PUBLIC/PRIVATE) forms
# on the same target is an error, so nothing further to link here.
