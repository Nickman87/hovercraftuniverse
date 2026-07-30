# Lua 5.1 + LuaBind for the modern build

## Why not vcpkg

- vcpkg's `lua` port tracks modern Lua (5.5 at the time of writing). This
  game's `Scripting` project and `data/scripts/**` were written for Lua
  5.1's API (`lua_open()`, no integer subtype, `luaL_reg` era headers) and
  LuaBind 0.9's introspection code depends on 5.1-era internals. Bridging
  that gap is out of scope for a build-system port.
- There is no vcpkg port for LuaBind at all (searched the registry and
  GitHub for `vcpkg lua bind port` — nothing maintained exists).

So both are vendored via CMake `FetchContent`, pinned to exact versions,
instead of committing thousands of vendored source files to this repo.

## What's provided

`third_party/CMakeLists.txt` defines two static-library targets:

- **`hu_lua`** — Lua **5.1.5**, from the official tarball
  (`https://www.lua.org/ftp/lua-5.1.5.tar.gz`,
  `SHA256 2640fc56a795f29d28ef15e13c34a47e223960b0240e8cb0a82d9b0738695333`).
  Lua ships no build system of its own; `third_party/lua/CMakeLists.txt.in`
  is our own CMakeLists, copied into the fetched source tree via
  `FetchContent`'s `PATCH_COMMAND`. It compiles all of `src/*.c` except
  `lua.c`/`luac.c` (the interpreter/compiler command-line mains) as C, static
  (`LUA_BUILD_AS_DLL` left undefined), and exposes `src/` as a `PUBLIC`
  include dir, i.e. `#include "lua.h"` / `"lualib.h"` / `"lauxlib.h"` work
  as-is, matching what `ScriptWrapper.h` and `OgreLuaBindings.h` expect.

- **`hu_luabind`** — LuaBind, from the **ryzom/luabind** fork, pinned to
  commit `0ae9bd6e40fe6c70e9d032ff096370929f58c143`
  (`https://github.com/ryzom/luabind.git`).

### Fork selection

Candidates considered:

| Fork | Verdict |
|---|---|
| `luabind/luabind` (official-ish continuation) | Last pushed 2020; no evidence of a validated MSVC+CMake build; passed over in favor of a fork with a proven Windows build. |
| `rpavlik/luabind` | The CMake-ified, MSVC-aware continuation of classic LuaBind 0.9. Actively maintained (pushed 2023). Requires Lua 5.1 explicitly (`find_package(Lua51 REQUIRED)`) and Boost. Good candidate, but **`ryzom/luabind` is a fork of exactly this project** with one relevant advantage below, so it was chosen instead of the rpavlik base. |
| **`ryzom/luabind`** (chosen) | Fork of `rpavlik/luabind` maintained by the Ryzom (MMORPG) project to add C++11 and newer-Lua support, while explicitly keeping Lua 5.1 compatibility behind a `#if LUA_VERSION_NUM < 502` shim (`src/lua51compat.cpp`, providing `luaL_tolstring` for pre-5.2 Lua). Its `CMakeLists.txt` is the same `rpavlik`-authored file (same author header), so behaviour/API is effectively identical to the parent — this fork was picked because this **exact commit is already proven to build cleanly under MSVC with the vcpkg CMake toolchain**: the `nimetu/vcpkg_luabind_example` overlay port builds this same SHA via `vcpkg_cmake_configure`/`vcpkg_cmake_install`. That's independent evidence the CMake build works on Windows before we ever tried it ourselves. |
| `DennisOSRM/luabind-deboostified` | Does not exist on GitHub under that name (404) — not a real candidate; likely conflated with another "deboostified" effort. |
| `Hedede/luabind` ("Luabind with boost excised"), `ForserX/luabind-latest` ("Boost-free luabind") | Boost-free forks exist, but since Boost 1.91 is already installed in this project's vcpkg (`x86-windows` triplet) specifically to support LuaBind, there's no need to take on a de-boostified fork's API/behavioral differences. Not used. |

LuaBind version pinned: **0.9.1** (per `CPACK_PACKAGE_VERSION_*` in
`ryzom/luabind`'s `CMakeLists.txt`), commit
`0ae9bd6e40fe6c70e9d032ff096370929f58c143` (merge of
"Fixes alignment assumption in `object_rep` placement new", 2022-02-19).

## How it's wired

`third_party/CMakeLists.txt`:

1. Fetches and builds `hu_lua` as above.
2. Before fetching LuaBind, sets `LUA_FOUND=TRUE`, `LUA_INCLUDE_DIRS` (from
   `hu_lua`'s `INTERFACE_INCLUDE_DIRECTORIES`), and `LUA_LIBRARIES=hu_lua`.
   LuaBind's own `CMakeLists.txt` skips its `find_package(Lua51)` call when
   `LUA_FOUND` is already set and links its `luabind` target against
   whatever `LUA_LIBRARIES` names — so LuaBind ends up built against and
   linked to *our* Lua 5.1.5, not a system/vcpkg one.
3. Calls `find_package(Boost REQUIRED)` itself (LuaBind's own
   `CMakeLists.txt` does the same internally, redundant but harmless) so
   `Boost_INCLUDE_DIRS` is available in our scope too.
4. Fetches `ryzom/luabind` via `FetchContent` at the pinned SHA, with
   `CMAKE_POLICY_VERSION_MINIMUM` temporarily set to 3.5, because LuaBind's
   vendored `CMakeLists.txt` declares `cmake_minimum_required(VERSION
   3.0.0)`, which CMake >= 4.0 refuses to evaluate at all
   ("Compatibility with CMake < 3.5 has been removed"). We don't patch
   LuaBind's CMakeLists.txt itself, just tell CMake how to interpret it.
5. **Important gap this file fixes up**: LuaBind's own `CMakeLists.txt`
   only adds its own headers and Boost's include dir via old-style
   directory-scoped `include_directories(BEFORE/AFTER ...)`, not
   `target_include_directories()`. That means none of it propagates to
   consumers via `target_link_libraries(... luabind)` alone — confirmed by
   trial build (see "Compile verification" below). We add it back with:
   ```cmake
   target_include_directories(luabind INTERFACE
       "${hu_luabind_src_SOURCE_DIR}"
       "${hu_luabind_src_BINARY_DIR}"   # for generated luabind/build_information.hpp
       ${Boost_INCLUDE_DIRS})
   ```
6. Sets sets `LUABIND_INSTALL=OFF`, `BUILD_TESTING=OFF` (we only need the
   library, not LuaBind's own install rules or test suite).
7. Exposes the fetched `luabind` target under our own name:
   `add_library(hu_luabind ALIAS luabind)`.
8. LuaBind's `src/CMakeLists.txt` already links `luabind` against
   `${LUA_LIBRARIES}` (i.e. `hu_lua`) using the old plain-signature form of
   `target_link_libraries()`; we deliberately do **not** add a second,
   keyword-form `target_link_libraries(luabind PUBLIC ...)` call, because
   CMake forbids mixing plain and keyword forms on the same target. Nothing
   further needed there.

### One extra vcpkg install required

Building `luabind` failed initially with two missing headers:
`boost/dynamic_bitset.hpp` and `boost/foreach.hpp` — Boost split these into
separate vcpkg ports (`boost-dynamic-bitset`, `boost-foreach`) not pulled in
by whatever installed the base `boost` set. Installed with:

```
vcpkg install boost-dynamic-bitset:x86-windows boost-foreach:x86-windows
```

If setting up a fresh vcpkg instance for this project, install both
alongside the rest of Boost.

## Compile verification

Verified with a scratch top-level CMake project at `C:\hu-tp-build`
(`add_subdirectory` on `third_party`, generator `Visual Studio 17 2022 -A
Win32`, `CMAKE_TOOLCHAIN_FILE` = vcpkg's, `VCPKG_TARGET_TRIPLET=x86-windows`):

- `hu_lua` — builds clean, Release and Debug, Win32/v143. No warnings beyond
  a `_CRT_SECURE_NO_WARNINGS`-suppressed set.
- `hu_luabind` (target `luabind`) — builds clean, Release and Debug,
  Win32/v143, after the two extra Boost vcpkg installs above. Only warning
  output is Boost's own deprecation notice about global bind placeholders
  (`boost/bind.hpp`), harmless.
- A small smoke-test executable (`smoketest.cpp`, linked against
  `hu_luabind` + `hu_lua` only, mirroring exactly how the game's Scripting
  project would consume these targets) was written, built, and **run**. It
  exercises the same LuaBind API surface the game code uses (see below) —
  `lua_open`/`luaL_openlibs`/`luabind::open`, `module`/`def`/`class_`,
  `constructor<>`, `def_readwrite`, `tostring(self)`, `self + other<T>()`,
  `globals(L)` + `object` table indexing/assignment (the `LUA_CONST_*` macro
  pattern from `OgreLuaBindings.cpp`), and `call_function<int>`. It compiled
  and ran correctly (exit code 0, correct computed values printed).

## API compatibility findings

Reviewed (read-only) `HovercraftUniverse/Scripting/ScriptWrapper.cpp/.h` and
`OgreLuaBindings.cpp/.h`. LuaBind APIs used by the game:

| API used by the game | Where | Present in `ryzom/luabind`? |
|---|---|---|
| `lua_open()` | `ScriptWrapper.cpp` | Yes — Lua 5.1.5's `lua.h` still defines `#define lua_open() luaL_newstate()` (line 287), so this compiles unchanged against `hu_lua`. |
| `luaL_openlibs`, `luaL_dofile`, `luaL_dostring`, `lua_close` | `ScriptWrapper.cpp` | Plain Lua 5.1 C API, unaffected by LuaBind fork choice. |
| `luabind::open(lua_State*)` | `ScriptWrapper.cpp` | Present, unchanged signature. |
| `luabind::module(L) [ ... ]` | Both files | Present, unchanged. |
| `luabind::def(name, fn)` | Both files | Present, unchanged. |
| `luabind::class_<T>(name)`, `.def(...)`, `.def_readwrite(...)`, `.def(constructor<...>())` | `OgreLuaBindings.cpp` | Present, unchanged. |
| `luabind::class_<Derived, Base>(name)` (inheritance) | `OgreLuaBindings.cpp` (`Entity`/`MovableObject`) | Present, unchanged. |
| `luabind::tostring(self)` operator, `self + other<T>()`, `self - other<T>()`, `self * other<T>()`, `self * Real()`, `self / Real()` | `OgreLuaBindings.cpp` (`luabind/operator.hpp`) | Present, unchanged — verified in the smoke test. |
| `luabind::globals(L)` + `luabind::object` (table `operator[]` get/set) | `OgreLuaBindings.cpp` (`LUA_CONST_START/LUA_CONST/LUA_CONST_END` macros) | Present, unchanged — verified in the smoke test. |
| `luabind::call_function<int>(L, name)` | `ScriptWrapper.cpp` | Present, unchanged. |
| Overload resolution via C-style function-pointer casts, e.g. `(void(Camera::*)(const Vector3&))&Camera::setPosition` | `OgreLuaBindings.cpp` (`bindCamera`) | Ordinary C++, unaffected by LuaBind. |

**No gaps found.** Every LuaBind API the game actually uses is present and
unchanged in `ryzom/luabind` at the pinned commit — expected, since this
fork only adds newer-Lua/C++11 support behind compatibility shims rather
than reworking the public API (unlike, say, a "luabind 2.x"-style rewrite,
which does not appear to exist as a maintained project).

## Integration snippet for `modern/CMakeLists.txt`

Not applied here (out of scope for this task — owned by another
in-progress edit). The main build needs exactly one `add_subdirectory` plus
linking the two targets into the `Scripting` target (and anything else that
`#include`s `lua.h`/`luabind/luabind.hpp`):

```cmake
add_subdirectory(${CMAKE_SOURCE_DIR}/../third_party third_party)
# ... on the Scripting library/target:
target_link_libraries(Scripting PUBLIC hu_lua hu_luabind)
```

(Adjust the relative path to `third_party` — it is a sibling of `modern/`,
not nested under it — and PUBLIC vs PRIVATE per whatever else links against
`Scripting` and also needs the Lua/LuaBind headers.)
