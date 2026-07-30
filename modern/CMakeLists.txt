cmake_minimum_required(VERSION 3.20)
project(HovercraftUniverseModern CXX)

# This is a SEPARATE, modern CMake build tree, independent of the faithful
# VC9 conversion under HovercraftUniverse/CMakeLists.txt (which must not be
# touched). Its purpose is to prove that HovercraftUniverse/Utils and
# HovercraftUniverse/Exceptions compile clean under the v143 toolchain, with
# sources referenced in place (not copied).
#
# --- Dependency strategy -----------------------------------------------
# Dependencies for this tree come from vcpkg (triplet x86-windows), installed
# via scripts/bootstrap-modern-deps.ps1:
#   - Ogre 14.5.2 as ogre[core,d3d9,overlay,zip] -- NOT the default feature
#     set. The default 'freeimage' feature drags in openjph, which hits an
#     MSVC C1001 internal compiler error in its AVX2 code on x86; dropping
#     freeimage avoids it and Ogre falls back to stb-based image codecs.
#     'd3d9' is required (the game's render system / fixed-function pipeline
#     for 13 of 16 materials), 'overlay' is required (HUD/GUI), 'zip' is
#     required (resource archives).
#   - Building RenderSystem_Direct3D9 additionally requires d3dx9.lib/d3dx9.h
#     from the legacy DirectX SDK (June 2010), which are not part of the
#     modern Windows SDK. The bootstrap script extracts (never installs) the
#     needed Include/ and Lib/x86/ subtrees into toolchain/dxsdk/DXSDK, and
#     HU_DXSDK_DIR below points at that location.
#   - Boost (algorithm, date_time, thread, bind, function, smart_ptr) backs
#     HU_WITH_BOOST_UTILS, see below.
#   - Still vendored separately, NOT from vcpkg (pending): Lua 5.1 (vcpkg's
#     `lua` port is 5.5, too new for the game's Lua 5.1-era scripts and
#     LuaBind), LuaBind (no vcpkg port; a maintained deboostified fork will be
#     vendored), and Hikari (built from archived source). Havok and ZoidCom
#     remain out of scope for this modern tree for now.
# -------------------------------------------------------------------------

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W3 /permissive-)
endif()

set(HU_ROOT ${CMAKE_SOURCE_DIR}/../HovercraftUniverse)

# --- DirectX SDK (d3dx9) location --------------------------------------
# Only needed once Ogre-dependent / D3D9-related targets are wired up (not
# yet, in this CMakeLists -- see the QUIET find_package(OGRE) probe below).
# Defaults to the location scripts/bootstrap-modern-deps.ps1 extracts the
# legacy June-2010 DirectX SDK's Include/ and Lib/x86/ subtrees into.
set(HU_DXSDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../toolchain/dxsdk/DXSDK" CACHE PATH
    "Root of the extracted legacy DirectX SDK (Include/, Lib/x86/), needed only for D3D9 linkage (d3dx9.lib)")
set(HU_DXSDK_INCLUDE_DIR "${HU_DXSDK_DIR}/Include")
set(HU_DXSDK_LIB_DIR "${HU_DXSDK_DIR}/Lib/x86")

if(EXISTS "${HU_DXSDK_INCLUDE_DIR}/d3dx9.h")
    message(STATUS "HU_DXSDK_DIR looks valid (found d3dx9.h under ${HU_DXSDK_INCLUDE_DIR})")
else()
    message(STATUS "HU_DXSDK_DIR does not yet contain d3dx9.h -- run scripts/bootstrap-modern-deps.ps1 before wiring D3D9 targets")
endif()

# --- vcpkg-provided Ogre probe (informational only) ---------------------
# Ogre-dependent targets are not added yet (Ogre may still be building via
# the bootstrap script). This QUIET probe just reports whether vcpkg's Ogre
# config package is currently visible to CMake, so the next step can wire up
# real targets against it.
find_package(OGRE CONFIG QUIET)
if(OGRE_FOUND)
    message(STATUS "find_package(OGRE CONFIG) succeeded -- Ogre is available for a future target wiring step")
else()
    message(STATUS "find_package(OGRE CONFIG) did not find Ogre yet -- install it via scripts/bootstrap-modern-deps.ps1 and ensure CMAKE_TOOLCHAIN_FILE points at vcpkg.cmake")
endif()

# --- hu_exceptions -----------------------------------------------------
add_library(hu_exceptions STATIC
    ${HU_ROOT}/Exceptions/Exception.cpp
    ${HU_ROOT}/Exceptions/Exception.h
)

target_include_directories(hu_exceptions PUBLIC
    ${HU_ROOT}/Exceptions
)

target_compile_definitions(hu_exceptions PRIVATE
    _CRT_SECURE_NO_WARNINGS
)

# --- hu_utils ------------------------------------------------------------
# NOTE: Config.cpp and Timing.cpp have a genuine (non-portable-fix-able)
# dependency on Boost (boost::algorithm::string::trim and
# boost::posix_time respectively). They are added conditionally, only when
# HU_WITH_BOOST_UTILS is enabled (see below, defaults ON once vcpkg's Boost
# is detected) -- excluded from the base hu_utils sources here.
add_library(hu_utils STATIC
    ${HU_ROOT}/Utils/inifile.cpp
    ${HU_ROOT}/Utils/StringUtils.cpp
    ${HU_ROOT}/Utils/inifile.h
    ${HU_ROOT}/Utils/StringUtils.h
    ${HU_ROOT}/Utils/Listenable.h
    ${HU_ROOT}/Utils/ListenableImpl.h
    ${HU_ROOT}/Utils/PlayerMap.h
    ${HU_ROOT}/Utils/PlayerMapImpl.h
    ${HU_ROOT}/Utils/tstdlibs.h
)

target_include_directories(hu_utils PUBLIC
    ${HU_ROOT}/Utils
    ${HU_ROOT}/Exceptions
)

target_compile_definitions(hu_utils PRIVATE
    _CRT_SECURE_NO_WARNINGS
)

target_link_libraries(hu_utils PUBLIC hu_exceptions)

# Optional: build Config.cpp / Timing.cpp, which have a genuine (non-portable
# -fix-able) dependency on Boost (boost::algorithm::string::trim and
# boost::posix_time respectively). Now that dependencies come from vcpkg
# (see scripts/bootstrap-modern-deps.ps1, which installs boost-algorithm,
# boost-date-time, boost-thread, boost-bind, boost-function, boost-smart-ptr
# for x86-windows), Boost is expected to be available, so this defaults ON
# -- probed live below via a QUIET find_package(Boost) so the default only
# turns on when vcpkg's Boost is actually present. Pass
# -DHU_WITH_BOOST_UTILS=OFF to force it off regardless.
find_package(Boost QUIET COMPONENTS date_time)
if(Boost_FOUND)
    message(STATUS "find_package(Boost) succeeded -- defaulting HU_WITH_BOOST_UTILS to ON")
else()
    message(STATUS "find_package(Boost) did not find Boost -- defaulting HU_WITH_BOOST_UTILS to OFF (install via scripts/bootstrap-modern-deps.ps1)")
endif()
option(HU_WITH_BOOST_UTILS "Also build Config.cpp/Timing.cpp (requires Boost)" ${Boost_FOUND})

if(HU_WITH_BOOST_UTILS)
    if(NOT Boost_FOUND)
        find_package(Boost REQUIRED COMPONENTS date_time)
    endif()
    target_sources(hu_utils PRIVATE
        ${HU_ROOT}/Utils/Config.cpp
        ${HU_ROOT}/Utils/Config.h
        ${HU_ROOT}/Utils/Timing.cpp
        ${HU_ROOT}/Utils/Timing.h
    )
    target_link_libraries(hu_utils PUBLIC Boost::boost Boost::date_time)
endif()

# --- hu_ogremax ----------------------------------------------------------
# Ported (in place, not copied) from HovercraftUniverse/OgreMax/*.cpp/.hpp --
# the faithful VC9 conversion tracked by HovercraftUniverse/OgreMax/CMakeLists.txt
# (source list mirrored from there; that file is NOT modified). This target
# builds those same sources against Ogre 14.5.2 (vcpkg), with in-place API
# fixes applied directly to the OgreMax sources (see docs/porting/ogre-api-gap.md).
set(HU_OGREMAX_ROOT ${HU_ROOT}/OgreMax)

find_package(OGRE CONFIG REQUIRED)

add_library(hu_ogremax STATIC
    ${HU_OGREMAX_ROOT}/OgreMaxModel.cpp
    ${HU_OGREMAX_ROOT}/OgreMaxScene.cpp
    ${HU_OGREMAX_ROOT}/OgreMaxUtilities.cpp
    ${HU_OGREMAX_ROOT}/ProgressCalculator.cpp
    ${HU_OGREMAX_ROOT}/tinyxml/tinyxml.cpp
    ${HU_OGREMAX_ROOT}/tinyxml/tinyxmlerror.cpp
    ${HU_OGREMAX_ROOT}/tinyxml/tinyxmlparser.cpp
    ${HU_OGREMAX_ROOT}/Version.cpp
    ${HU_OGREMAX_ROOT}/OgreMaxModel.hpp
    ${HU_OGREMAX_ROOT}/OgreMaxPlatform.hpp
    ${HU_OGREMAX_ROOT}/OgreMaxRenderWindowIterator.hpp
    ${HU_OGREMAX_ROOT}/OgreMaxScene.hpp
    ${HU_OGREMAX_ROOT}/OgreMaxTypes.hpp
    ${HU_OGREMAX_ROOT}/OgreMaxUtilities.hpp
    ${HU_OGREMAX_ROOT}/ProgressCalculator.hpp
    ${HU_OGREMAX_ROOT}/tinyxml/tinyxml.h
    ${HU_OGREMAX_ROOT}/Version.hpp
)

target_include_directories(hu_ogremax PRIVATE
    ${HU_OGREMAX_ROOT}
    ${HU_OGREMAX_ROOT}/tinyxml
)

target_compile_definitions(hu_ogremax PRIVATE
    WIN32
    _LIB
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)
# NOTE: deliberately NOT defining UNICODE/_UNICODE here (unlike the VC9
# OgreMax.vcproj / HovercraftUniverse/OgreMax/CMakeLists.txt, which do). The
# OgreMax sources call several Win32 APIs (GetModuleFileName, GetFullPathName,
# GetFileAttributes in OgreMaxUtilities.cpp) with narrow `char` buffers and
# Ogre::String (std::string) throughout -- they were never actually Unicode
# clean, and defining UNICODE/_UNICODE here resolves those calls to their *W
# (wchar_t) forms, which fails to compile against narrow buffers. This is a
# build-config difference from the VC9 project; it does not affect the VC9
# build (HovercraftUniverse/OgreMax/CMakeLists.txt is untouched).

target_link_libraries(hu_ogremax PUBLIC
    OgreMain
    OgreOverlay
)
