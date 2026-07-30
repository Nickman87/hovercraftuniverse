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
#     vendored), and Hikari (built from archived source). Havok remains out
#     of scope for this modern tree for now.
#   - ZoidCom: the real vendor library is dead (vendor gone, only VC9
#     binaries survive) so it is replaced by an in-repo, API-compatible
#     shim, hu_zoidcom_compat (compat/zoidcom/**), built from source and
#     linked against ENet (vcpkg's `enet` port, exposed as the
#     unofficial-enet CMake config package). See
#     docs/porting/zoidcom-compat.md for what's real vs. stubbed.
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

# --- hu_hikari ---------------------------------------------------------------
# Hikari, the Flash-in-Ogre GUI library the game's GUI project is built on.
# Vendored from the upstream Google Code SVN source archive
# (archive-mirror/hikari-source-archive.zip, LGPL 2.1) into
# third_party/hikari/{include,source} -- NOT a port of anything under
# HovercraftUniverse/ (this is a third-party library, owned by this porting
# effort like third_party/lua, third_party/luabind).
#
# The game shipped against a team-patched Hikari (three "Win-Hikari-Update"
# revisions, archive-mirror/Win-Hikari-Update{,_2,_3}.rar), of which only
# prebuilt VC9 binaries + patched PUBLIC headers survive
# (HovercraftUniverse/dependencies/Hikari/{includes,lib} -- read-only,
# reference only, not linked against here). Diffing those patched headers
# against upstream's and cross-checking against actual GUI/** call sites
# (grep for handleInputs/toggleBringToTop/createFlashOverlay's signature)
# fully identified the patch and it has been reproduced byte-identically at
# the header level in third_party/hikari/include (see
# docs/porting/hikari-gui.md for the diff and the matching source-level
# reconstruction, since no patched .cpp survives to diff against):
#   - Multi-tier z-ordering removed: FlashControl/ViewportOverlay/
#     HikariManager::createFlashOverlay's `Ogre::uchar zOrder, Ogre::uchar
#     tier` collapsed to a single `Ogre::ushort zOrder`.
#   - FlashControl::setPosition/resetPosition/getCoordinates and
#     HikariManager::getFocusedControl removed from the public API (dead
#     weight -- confirmed unused by GUI/**).
#   - FlashControl::handleInputs(bool)/isHandlingInputs() added (GUI/**
#     calls handleInputs from BasicOverlay::show/hide).
#   - HikariManager::toggleBringToTop(bool) added (GUI/GUIManager.cpp calls
#     it) to let a HUD overlay stay visually on top without the focus code
#     reshuffling its z-order every frame.
#
# Additional MODERN-BUILD-ONLY fixes (not part of the team's patch, needed
# only to get 2008-era Hikari through v143 + Ogre 14, see
# docs/porting/hikari-gui.md):
#   - impl/FlashSite.h, impl/flashhandler.h: `#import
#     "PROGID:ShockwaveFlash.ShockwaveFlash"` (requires Flash.ocx to be
#     registered via regsvr32 on the build machine) replaced with a
#     computed `#import HIKARI_FLASH_OCX_PATH` that loads the typelib
#     directly out of the Flash.ocx file below -- no system-wide ActiveX
#     registration performed or required.
#   - HikariPlatform.h: `_HikariExport` gains a HIKARI_STATIC branch so it's
#     a no-op when Hikari is built as a static lib (this target), instead
#     of always dllexport/dllimport (which only made sense for the
#     original Hikari.dll).
#   - FlashControl.cpp: two `texture.setNull()` calls -> `texture.reset()`
#     (Ogre 14's SharedPtr dropped isNull()/setNull()/getPointer(), same
#     class of break as docs/porting/ogre-api-gap.md's SharedPtr-idiom row).
#   - FlashControl.{h,cpp}: the `lastDirtyWidth`/`lastDirtyHeight` int
#     members the team's patch already removed from the public header are
#     reconstructed as a GetObject()-based bitmap-size query instead (see
#     source comment), preserving the original DC-rebuild-on-resize
#     behavior without the removed members.
#
# Hikari renders Flash movies into an Ogre::Texture and wraps them in an
# Ogre::Overlay (Impl::ViewportOverlay, third_party/hikari/include/impl) --
# needs the Overlay component (OgreOverlay, already required by hu_ogremax/
# hu_coreengine above) exactly as docs/porting/ogre-api-gap.md's
# Hikari/Flash-GUI row anticipated.
set(HU_HIKARI_ROOT ${CMAKE_SOURCE_DIR}/../third_party/hikari)

# Flash.ocx to import the ActiveX typelib from at compile time (see the
# FlashSite.h/flashhandler.h note above) AND to load at runtime
# (HikariManager loads "Flash.ocx" next to the executable, falling back to
# CoCreateInstance-by-registry if that fails -- see Hikari.cpp). Only a
# real, installed copy of Adobe Flash Player's ActiveX control (32-bit
# only -- this is the reason this whole modern tree is pinned to Win32/x86,
# not just Havok/ZoidCom-shim convenience) satisfies either use. The only
# copy in this repository is local-game/data/Flash.ocx (a real Flash.ocx
# binary, not vendored/redistributed by this porting effort -- see
# docs/porting/hikari-gui.md for the redistribution/licensing note).
set(HU_FLASH_OCX_PATH "${CMAKE_SOURCE_DIR}/../local-game/data/Flash.ocx" CACHE FILEPATH
    "Path to Flash.ocx, used both to #import its ActiveX typelib at hu_hikari compile time and (at runtime) loaded by HikariManager")

if(NOT EXISTS "${HU_FLASH_OCX_PATH}")
    message(WARNING "HU_FLASH_OCX_PATH (${HU_FLASH_OCX_PATH}) does not exist -- hu_hikari's #import of the Flash ActiveX typelib will fail to compile. See docs/porting/hikari-gui.md.")
endif()

find_package(OGRE CONFIG REQUIRED)

add_library(hu_hikari STATIC
    ${HU_HIKARI_ROOT}/source/FlashControl.cpp
    ${HU_HIKARI_ROOT}/source/FlashValue.cpp
    ${HU_HIKARI_ROOT}/source/Hikari.cpp
    ${HU_HIKARI_ROOT}/source/KeyboardHook.cpp
    ${HU_HIKARI_ROOT}/source/Position.cpp
    ${HU_HIKARI_ROOT}/source/RenderBuffer.cpp
    ${HU_HIKARI_ROOT}/source/ViewportOverlay.cpp

    ${HU_HIKARI_ROOT}/include/Delegate.h
    ${HU_HIKARI_ROOT}/include/FlashControl.h
    ${HU_HIKARI_ROOT}/include/FlashValue.h
    ${HU_HIKARI_ROOT}/include/Hikari.h
    ${HU_HIKARI_ROOT}/include/HikariPlatform.h
    ${HU_HIKARI_ROOT}/include/Position.h
    ${HU_HIKARI_ROOT}/include/impl/FlashSite.h
    ${HU_HIKARI_ROOT}/include/impl/flashhandler.h
    ${HU_HIKARI_ROOT}/include/impl/KeyboardHook.h
    ${HU_HIKARI_ROOT}/include/impl/RenderBuffer.h
    ${HU_HIKARI_ROOT}/include/impl/ViewportOverlay.h
)

target_include_directories(hu_hikari PUBLIC
    ${HU_HIKARI_ROOT}/include
)

target_compile_definitions(hu_hikari
    PUBLIC
        HIKARI_STATIC
    PRIVATE
        HIKARI_NONCLIENT_BUILD
        HIKARI_FLASH_OCX_PATH="${HU_FLASH_OCX_PATH}"
        WIN32
        _LIB
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
)

target_link_libraries(hu_hikari PUBLIC
    OgreMain
    OgreOverlay
)

# ole32/oleaut32: COM/OLE (CoCreateInstance, IOleObject, VARIANT/BSTR
# handling) used throughout FlashControl.cpp/impl/FlashSite.h/
# impl/flashhandler.h to host the Flash ActiveX control. user32/gdi32: the
# GetDC/CreateCompatibleDC/CreateDIBSection double-buffering + WM_* message
# injection in FlashControl.cpp/impl/KeyboardHook.cpp.
target_link_libraries(hu_hikari PUBLIC
    ole32
    oleaut32
    user32
    gdi32
)

# --- hu_gui ------------------------------------------------------------------
# Ported (in place, not copied) from HovercraftUniverse/GUI/*.cpp/.h -- the
# faithful VC9 conversion tracked by HovercraftUniverse/GUI/CMakeLists.txt
# (source list mirrored from there; that file is NOT modified, and per its
# own comment Test.cpp is deliberately excluded there too). Builds against
# hu_hikari (the rebuilt Hikari, see above) instead of the original prebuilt
# dependencies/Hikari, and Ogre 14.5.2's Overlay component.
set(HU_GUI_ROOT ${HU_ROOT}/GUI)

add_library(hu_gui STATIC
    ${HU_GUI_ROOT}/GUIManager.cpp
    ${HU_GUI_ROOT}/MouseVisualisation.cpp
    ${HU_GUI_ROOT}/OverlayContainer.cpp
    ${HU_GUI_ROOT}/Resolution.cpp
    ${HU_GUI_ROOT}/ScalingResolution.cpp
    ${HU_GUI_ROOT}/BasicOverlay.cpp
    ${HU_GUI_ROOT}/OverlayManager.cpp
    ${HU_GUI_ROOT}/Chat.cpp
    ${HU_GUI_ROOT}/ConnectWindow.cpp
    ${HU_GUI_ROOT}/Countdown.cpp
    ${HU_GUI_ROOT}/Direction.cpp
    ${HU_GUI_ROOT}/LoadingOverlay.cpp
    ${HU_GUI_ROOT}/LobbyOverlay.cpp
    ${HU_GUI_ROOT}/MenuButton.cpp
    ${HU_GUI_ROOT}/NMessageBox.cpp
    ${HU_GUI_ROOT}/Position.cpp
    ${HU_GUI_ROOT}/Results.cpp
    ${HU_GUI_ROOT}/ServerMenu.cpp
    ${HU_GUI_ROOT}/Speedometer.cpp
    ${HU_GUI_ROOT}/TimeBehind.cpp
    ${HU_GUI_ROOT}/Timer.cpp
    # Test.cpp intentionally excluded -- not part of GUI.vcproj's file list
    # (see HovercraftUniverse/GUI/CMakeLists.txt's own comment on this)

    ${HU_GUI_ROOT}/GUIManager.h
    ${HU_GUI_ROOT}/MouseVisualisation.h
    ${HU_GUI_ROOT}/OverlayContainer.h
    ${HU_GUI_ROOT}/IResolution.h
    ${HU_GUI_ROOT}/Resolution.h
    ${HU_GUI_ROOT}/ScalingResolution.h
    ${HU_GUI_ROOT}/BasicOverlay.h
    ${HU_GUI_ROOT}/OverlayManager.h
    ${HU_GUI_ROOT}/OverlayParameters.h
    ${HU_GUI_ROOT}/Chat.h
    ${HU_GUI_ROOT}/ConnectListener.h
    ${HU_GUI_ROOT}/ConnectWindow.h
    ${HU_GUI_ROOT}/Countdown.h
    ${HU_GUI_ROOT}/Direction.h
    ${HU_GUI_ROOT}/LoadingOverlay.h
    ${HU_GUI_ROOT}/LobbyOverlay.h
    ${HU_GUI_ROOT}/MenuButton.h
    ${HU_GUI_ROOT}/NMessagebox.h
    ${HU_GUI_ROOT}/Position.h
    ${HU_GUI_ROOT}/Results.h
    ${HU_GUI_ROOT}/ServerMenu.h
    ${HU_GUI_ROOT}/ServerMenuListener.h
    ${HU_GUI_ROOT}/Speedometer.h
    ${HU_GUI_ROOT}/TimeBehind.h
    ${HU_GUI_ROOT}/Timer.h
)

# OIS 1.6 (vcpkg): see the identical hu_coreengine note below for why the
# ois/ subdirectory itself (not just its parent) must be on the include path.
# (hu_gui is wired up before hu_coreengine in this file, so this target
# resolves HU_OIS_INCLUDE_DIR/HU_OIS_LIBRARY itself rather than relying on
# hu_coreengine's later find_path/find_library calls -- both are cached
# find results, so doing this twice is harmless.)
find_path(HU_OIS_INCLUDE_DIR NAMES OIS.h PATH_SUFFIXES ois)
# OIS's Release and Debug import libs (OIS.lib / OIS_d.lib) are NOT ABI
# compatible with each other (different CRT/_ITERATOR_DEBUG_LEVEL) -- a
# single `find_library(NAMES OIS)` always resolves the Release one
# (vcpkg's non-debug lib/ dir), regardless of which config this multi-config
# generator is currently building. Linking that into a Debug-config exe
# compiles/links fine but crashes at runtime (STATUS_ACCESS_VIOLATION inside
# OIS.dll itself, confirmed via Windows Event Log's Application error entry
# while first running the client -- see docs/porting/first-run.md) the
# instant OIS is asked to do anything involving an STL container crossing
# the DLL boundary (device enumeration). Fixed below by resolving BOTH
# import libs and selecting per-config via the debug/optimized
# target_link_libraries keywords (the standard CMake idiom for this,
# works with any generator).
find_library(HU_OIS_LIBRARY_RELEASE NAMES OIS)
find_library(HU_OIS_LIBRARY_DEBUG NAMES OIS_d)

# GUIManager.h uses boost::signals2::mutex (<boost/signals2/mutex.hpp>) and
# IResolution.h uses boost::shared_ptr (<boost/shared_ptr.hpp>) -- both
# genuine, narrow dependency gaps like hu_coreengine's boost-thread/
# boost-random additions above (boost-signals2 wasn't in the original
# bootstrap script's vcpkg install list; installed here via
# `vcpkg install boost-signals2:x86-windows`, boost-smart-ptr for
# shared_ptr was already installed). scripts/bootstrap-modern-deps.ps1
# should gain boost-signals2 too (tracked separately, flagged here as with
# the other ad hoc Boost additions in this file).
find_package(Boost REQUIRED COMPONENTS signals2)

target_include_directories(hu_gui PUBLIC
    ${HU_GUI_ROOT}
    ${HU_OIS_INCLUDE_DIR}
)

target_compile_definitions(hu_gui PRIVATE
    WIN32
    _CONSOLE
    _MBCS
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

target_link_libraries(hu_gui PUBLIC
    hu_hikari
    hu_exceptions
    OgreMain
    OgreOverlay
    debug ${HU_OIS_LIBRARY_DEBUG}
    optimized ${HU_OIS_LIBRARY_RELEASE}
    Boost::boost
    Boost::signals2
)

# --- hu_zoidcom_compat -----------------------------------------------------
# API-compatible, from-scratch replacement for the dead closed-source
# ZoidCom networking library (compat/zoidcom/**, owned by this porting
# effort -- not a port of anything under HovercraftUniverse/). See
# docs/porting/zoidcom-compat.md.
add_subdirectory(${CMAKE_SOURCE_DIR}/../compat/zoidcom ${CMAKE_BINARY_DIR}/compat_zoidcom)

# --- hu_networking -----------------------------------------------------
# Ported (in place, not copied) from HovercraftUniverse/Networking/*.cpp/.h --
# the faithful VC9 conversion tracked by HovercraftUniverse/Networking/CMakeLists.txt
# (source list mirrored from there; that file is NOT modified). Builds
# against hu_zoidcom_compat instead of the real (dead) ZoidCom prebuilt lib.
set(HU_NETWORKING_ROOT ${HU_ROOT}/Networking)

add_library(hu_networking STATIC
    ${HU_NETWORKING_ROOT}/OgreQuaternion_Replicator.cpp
    ${HU_NETWORKING_ROOT}/OgreVector3_Replicator.cpp
    ${HU_NETWORKING_ROOT}/String_Replicator.cpp
    ${HU_NETWORKING_ROOT}/ChatClient.cpp
    ${HU_NETWORKING_ROOT}/ChatEntity.cpp
    ${HU_NETWORKING_ROOT}/ChatEvent.cpp
    ${HU_NETWORKING_ROOT}/ChatEventParser.cpp
    ${HU_NETWORKING_ROOT}/ChatServer.cpp
    ${HU_NETWORKING_ROOT}/NotifyEvent.cpp
    ${HU_NETWORKING_ROOT}/TextEvent.cpp
    ${HU_NETWORKING_ROOT}/NetworkClient.cpp
    ${HU_NETWORKING_ROOT}/NetworkEntity.cpp
    ${HU_NETWORKING_ROOT}/NetworkIDManager.cpp
    ${HU_NETWORKING_ROOT}/NetworkServer.cpp

    ${HU_NETWORKING_ROOT}/OgreQuaternion_Replicator.h
    ${HU_NETWORKING_ROOT}/OgreVector3_Replicator.h
    ${HU_NETWORKING_ROOT}/String_Replicator.h
    ${HU_NETWORKING_ROOT}/ChatClient.h
    ${HU_NETWORKING_ROOT}/ChatEntity.h
    ${HU_NETWORKING_ROOT}/ChatEvent.h
    ${HU_NETWORKING_ROOT}/ChatEventParser.h
    ${HU_NETWORKING_ROOT}/ChatEventType.h
    ${HU_NETWORKING_ROOT}/ChatListener.h
    ${HU_NETWORKING_ROOT}/ChatServer.h
    ${HU_NETWORKING_ROOT}/NotifyEvent.h
    ${HU_NETWORKING_ROOT}/TextEvent.h
    ${HU_NETWORKING_ROOT}/NetworkClient.h
    ${HU_NETWORKING_ROOT}/NetworkEntity.h
    ${HU_NETWORKING_ROOT}/NetworkEvent.h
    ${HU_NETWORKING_ROOT}/NetworkEventImpl.h
    ${HU_NETWORKING_ROOT}/NetworkEventParser.h
    ${HU_NETWORKING_ROOT}/NetworkIDManager.h
    ${HU_NETWORKING_ROOT}/NetworkServer.h
)

target_include_directories(hu_networking PUBLIC
    ${HU_NETWORKING_ROOT}
    ${HU_ROOT}/Utils
)

find_package(OGRE CONFIG REQUIRED)

target_compile_definitions(hu_networking PRIVATE
    WIN32
    _LIB
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

target_link_libraries(hu_networking PUBLIC
    hu_zoidcom_compat
    hu_exceptions
    hu_utils
    OgreMain
)

# --- hu_coreengine ---------------------------------------------------------
# Ported (in place, not copied) from HovercraftUniverse/CoreEngine/*.cpp/.h --
# the faithful VC9 conversion tracked by HovercraftUniverse/CoreEngine/CMakeLists.txt
# (source list mirrored from there; that file is NOT modified). This target builds
# a SUBSET of those sources against Ogre 14.5.2 (vcpkg) + OIS 1.6, with in-place
# API fixes applied directly to the CoreEngine sources.
#
# Per the VC9 CMakeLists.txt, MovableTextOverlay.cpp/.h and RectLayoutManager.h
# are orphaned (never referenced by CoreEngine.vcproj) and stay excluded here too.
#
# EXCLUSIONS (real, beyond the above): CoreEngine's source tree turns out to be
# far more entangled with the not-yet-ported GUI/Networking/Sound/ZoidCom
# projects than the Ogre-only gap analysis (docs/porting/ogre-api-gap.md)
# anticipated -- that doc only measured Ogre API breaks, not cross-project
# #include dependencies. Tracing #includes (not just the obvious ones):
#   - Entity.h includes "NetworkEntity.h" (HovercraftUniverse/Networking) and
#     the Entity class itself derives from NetworkEntity and takes
#     ZCom_BitStream*/eZCom_Event params (ZoidCom, a prebuilt proprietary lib).
#   - Controller.h includes "ControllerEvent.h", which includes "NetworkEvent.h"
#     (Networking) -- so Controller.h, FreeroamCameraController.h (includes
#     Controller.h) and RaceCamera.h (includes FreeroamCameraController.h) are
#     all blocked transitively, even though none of them look
#     networking-related at a glance.
#   - ControllerEventParser.h includes "NetworkEventParser.h" (Networking).
#   - EntityPropertySystem.h includes <zoidcom/zoidcom.h> directly.
#   - Application.h and BasicGameState.h include <GUIManager.h> and
#     <SoundManager.h> (GUI, Sound projects) directly.
#   - RepresentationManager.h/.cpp pull in EntityManager.h/EntityRepresentation.h
#     /GameView.h (all Entity.h-tainted) and GUIManager.h/SoundManager.h directly.
#   - GameStateManager.cpp (though not its .h) #includes "BasicGameState.h" in
#     full, so it is blocked too despite GameStateManager.h itself only
#     forward-declaring BasicGameState.
#
# Net effect (UPDATED once hu_zoidcom_compat + hu_networking exist, see
# above): of the 22 .cpp files in CoreEngine.vcproj, the ones tainted only
# by Networking/ZoidCom (not by GUI/Sound) are now re-added below. Only
# files that still transitively pull in <GUIManager.h>/<SoundManager.h>
# (GUI/Sound projects, not ported yet) remain excluded. See the porting
# report (docs/porting/zoidcom-compat.md) for the full rationale; nothing
# here silently drops functionality -- every excluded file still exists on
# disk, untouched, ready to be re-added once its blocking project is ported.
set(HU_COREENGINE_ROOT ${HU_ROOT}/CoreEngine)

add_library(hu_coreengine STATIC
    ${HU_COREENGINE_ROOT}/CameraSpring.cpp
    ${HU_COREENGINE_ROOT}/InputManager.cpp
    ${HU_COREENGINE_ROOT}/KeyManager.cpp
    ${HU_COREENGINE_ROOT}/PathReader.cpp
    ${HU_COREENGINE_ROOT}/OgreWindowListener.cpp
    ${HU_COREENGINE_ROOT}/ObjectTextDisplay.cpp
    ${HU_COREENGINE_ROOT}/BasicView.cpp
    ${HU_COREENGINE_ROOT}/DedicatedServer.cpp
    ${HU_COREENGINE_ROOT}/EntityMapping.cpp

    # Re-enabled now that hu_zoidcom_compat/hu_networking exist (previously
    # blocked purely by Networking/ZoidCom #include taint; verified to
    # compile clean against the shim -- see docs/porting/zoidcom-compat.md
    # for the up to date accounting):
    ${HU_COREENGINE_ROOT}/FreeroamCameraController.cpp
    ${HU_COREENGINE_ROOT}/BasicEntityEvent.cpp
    ${HU_COREENGINE_ROOT}/ControllerEvent.cpp
    ${HU_COREENGINE_ROOT}/ControllerEventParser.cpp
    ${HU_COREENGINE_ROOT}/Entity.cpp
    ${HU_COREENGINE_ROOT}/EntityManager.cpp

    # Re-enabled now that hu_gui exists (previously blocked by GUI/Sound
    # #include taint via <GUIManager.h>/<SoundManager.h> -- hu_sound already
    # unblocked the Sound half; hu_gui unblocks the rest). See
    # docs/porting/hikari-gui.md for what did/didn't compile clean.
    ${HU_COREENGINE_ROOT}/EntityRepresentation.cpp
    ${HU_COREENGINE_ROOT}/BasicGameState.cpp
    ${HU_COREENGINE_ROOT}/GameStateManager.cpp
    ${HU_COREENGINE_ROOT}/RepresentationManager.cpp
    ${HU_COREENGINE_ROOT}/Application.cpp

    # Also re-enabled: RaceCamera.cpp/.h and GameView.cpp/.h were previously
    # excluded for a SEPARATE reason from the GUI/Sound taint above -- real
    # Ogre 1.7->14 Camera API breakage (Ogre::Camera's nodeless positioning
    # methods only exist when Ogre is built with OGRE_NODELESS_POSITIONING,
    # which this tree's vcpkg `ogre` port is not). Migrated in place to the
    # SceneNode-based idiom (operate on the camera's attached parent
    # SceneNode instead of the camera itself) -- see the detailed comments
    # in RaceCamera.cpp/.h and docs/porting/hikari-gui.md.
    ${HU_COREENGINE_ROOT}/RaceCamera.cpp
    ${HU_COREENGINE_ROOT}/GameView.cpp

    # Also re-enabled: EntityPropertySystem.cpp/.h -- was excluded for a
    # DIFFERENT, genuine reason (missing #include <iostream>, see the fix
    # in that file and the comment further down in this list), not GUI/Sound.
    ${HU_COREENGINE_ROOT}/EntityPropertySystem.cpp

    ${HU_COREENGINE_ROOT}/CameraSpring.h
    ${HU_COREENGINE_ROOT}/InputManager.h
    ${HU_COREENGINE_ROOT}/KeyManager.h
    ${HU_COREENGINE_ROOT}/CameraControllerActionType.h
    ${HU_COREENGINE_ROOT}/PathReader.h
    ${HU_COREENGINE_ROOT}/OgreWindowListener.h
    ${HU_COREENGINE_ROOT}/ObjectTextDisplay.h
    ${HU_COREENGINE_ROOT}/BasicView.h
    ${HU_COREENGINE_ROOT}/DedicatedServer.h
    ${HU_COREENGINE_ROOT}/EntityMapping.h
    ${HU_COREENGINE_ROOT}/EntityResource.h
    ${HU_COREENGINE_ROOT}/Controller.h
    ${HU_COREENGINE_ROOT}/ControllerActionType.h
    ${HU_COREENGINE_ROOT}/ControllerEvent.h
    ${HU_COREENGINE_ROOT}/ControllerEventParser.h
    ${HU_COREENGINE_ROOT}/ControllerEventType.h
    ${HU_COREENGINE_ROOT}/Entity.h
    ${HU_COREENGINE_ROOT}/EntityManager.h
    ${HU_COREENGINE_ROOT}/FreeroamCameraController.h
    ${HU_COREENGINE_ROOT}/BasicEntityEvent.h
    ${HU_COREENGINE_ROOT}/GameView.h
    ${HU_COREENGINE_ROOT}/RaceCamera.h
    ${HU_COREENGINE_ROOT}/EntityPropertySystem.h
    ${HU_COREENGINE_ROOT}/EntityRepresentation.h
    ${HU_COREENGINE_ROOT}/BasicGameState.h
    ${HU_COREENGINE_ROOT}/GameStateManager.h
    ${HU_COREENGINE_ROOT}/RepresentationManager.h
    ${HU_COREENGINE_ROOT}/Application.h

    # TODO(modernize): still excluded, but NOT for ZoidCom/Networking reasons
    # anymore -- the shim resolves that taint. Two independent kinds of
    # blocker remain, both out of scope for this task (compat/zoidcom/**
    # and modern/CMakeLists.txt are owned here; HovercraftUniverse/** is
    # read-only). Confirmed by actually attempting to compile each file
    # against hu_zoidcom_compat/hu_networking:
    #
    #   (A) Pre-existing Ogre API-gap breakage (see docs/porting/ogre-api-gap.md
    #       for the same class of issue elsewhere), independent of this task:
    #       RaceCamera.cpp/.h and GameView.cpp/.h have SINCE been fixed and
    #       re-enabled above (see the comments there) -- both
    #       Ogre::Camera's and Ogre::Light's nodeless positioning methods
    #       (setPosition/setDirection/lookAt/setFixedYawAxis/pitch/yaw/roll/
    #       getOrientation/getPosition) only exist when Ogre itself is built
    #       with OGRE_NODELESS_POSITIONING, which this tree's vcpkg `ogre`
    #       port is not; both were migrated to operate on/via a SceneNode
    #       instead. What remains here:
    #     - EntityPropertySystem.cpp/.h have SINCE been fixed and re-enabled
    #       above too (missing `#include <iostream>` for its std::cout
    #       calls -- see the comment there; its <zoidcom/zoidcom.h> #include
    #       already resolved fine, that was never the actual blocker).
    #       Discovered because linking the HovercraftUniverse exe target
    #       (see that target below) surfaced unresolved
    #       EntityPropertyMap/EntityProperty/EntityPropertyFactory/
    #       EntityPropertyMapReplicator symbols referenced from
    #       hu_coreengine's Entity.cpp and hu_game_physics's
    #       BoostAction/BoostProperty/SpeedBoost/HovercraftAIController.
    #
    #   (B) Formerly GUI/Sound-tainted -- ALL fixed and re-enabled above now
    #       that hu_gui/hu_sound exist: EntityRepresentation.cpp/.h,
    #       BasicGameState.cpp/.h, GameStateManager.cpp/.h,
    #       RepresentationManager.cpp/.h, Application.cpp/.h. See the
    #       comments up in the main source list above for what each needed
    #       (mostly just the new include dirs/link libs added to this
    #       target below; Application.cpp additionally needed the
    #       showConfigDialog/ST_GENERIC Ogre-API-gap fixes noted there).
    #
    # Inert (header-only, no matching .cpp in CoreEngine.vcproj, kept for
    # source-list parity even though nothing here #includes them):
    # NOTE: MenuView.h is listed for parity but has a real latent bug inherited
    # from the original source: it reuses BasicView.h's include guard
    # (`#define _BASICVIEW_H`), a copy-paste mistake. If anything ever
    # #includes both BasicView.h and MenuView.h in the same translation unit
    # (in that order), MenuView.h's body -- including the MenuView class
    # itself -- is silently skipped by the preprocessor. Nothing in
    # hu_coreengine currently includes MenuView.h, so this is inert for now;
    # flagged here for whoever ports the file(s) that use it.
    ${HU_COREENGINE_ROOT}/MenuView.h
)

# OIS 1.6 (vcpkg) ships no CMake config package (only a plain .lib + a
# pkg-config file at share/ois -- Cflags there list both ${includedir} and
# ${includedir}/ois). CoreEngine sources #include <OIS.h> (not <OIS/OIS.h>),
# so the ois/ subdirectory itself must be on the include path.
find_path(HU_OIS_INCLUDE_DIR NAMES OIS.h PATH_SUFFIXES ois)
# OIS's Release and Debug import libs (OIS.lib / OIS_d.lib) are NOT ABI
# compatible with each other (different CRT/_ITERATOR_DEBUG_LEVEL) -- a
# single `find_library(NAMES OIS)` always resolves the Release one
# (vcpkg's non-debug lib/ dir), regardless of which config this multi-config
# generator is currently building. Linking that into a Debug-config exe
# compiles/links fine but crashes at runtime (STATUS_ACCESS_VIOLATION inside
# OIS.dll itself, confirmed via Windows Event Log's Application error entry
# while first running the client -- see docs/porting/first-run.md) the
# instant OIS is asked to do anything involving an STL container crossing
# the DLL boundary (device enumeration). Fixed below by resolving BOTH
# import libs and selecting per-config via the debug/optimized
# target_link_libraries keywords (the standard CMake idiom for this,
# works with any generator).
find_library(HU_OIS_LIBRARY_RELEASE NAMES OIS)
find_library(HU_OIS_LIBRARY_DEBUG NAMES OIS_d)

# EntityPropertySystem.cpp/.h (re-enabled above) uses boost::mutex
# (<boost/thread/mutex.hpp>), which is a genuine, non-portable-fix-able
# dependency, same as hu_utils's Boost usage above. boost-thread is part of
# the vcpkg package set scripts/bootstrap-modern-deps.ps1 installs.
find_package(Boost REQUIRED COMPONENTS thread)

target_include_directories(hu_coreengine PUBLIC
    ${HU_COREENGINE_ROOT}
    ${HU_OIS_INCLUDE_DIR}
    ${HU_ROOT}/GUI
    ${HU_ROOT}/Sound
    ${HU_OGREMAX_ROOT}
    ${HU_OGREMAX_ROOT}/tinyxml
)

target_compile_definitions(hu_coreengine PRIVATE
    WIN32
    _LIB
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
    # RepresentationManager.h (re-enabled above, now that hu_gui exists) has
    # a pre-existing `using namespace std;` at header/global scope -- a bad
    # habit that was harmless in 2010 but, now that this file's #include
    # chain also reaches GUIManager.h/Hikari.h's <windows.h>/COM headers
    # (objidl.h etc, which declare a global `typedef unsigned char byte;`)
    # in the same translation unit, collides with C++17's `std::byte`
    # (pulled unqualified into scope by the `using namespace std;`) --
    # "'byte': ambiguous symbol". _HAS_STD_BYTE=0 is the standard MS-STL
    # escape hatch for exactly this legacy-code-plus-Windows-SDK collision
    # (undefines std::byte instead of requiring every legacy `using
    # namespace std;` header to be untangled). Not a team patch, not a
    # functionality change -- see docs/porting/hikari-gui.md.
    _HAS_STD_BYTE=0
)
# NOTE: deliberately NOT defining UNICODE/_UNICODE here (unlike the VC9
# CoreEngine.vcproj / HovercraftUniverse/CoreEngine/CMakeLists.txt, which do).
# Same reasoning as hu_ogremax above: DedicatedServer.cpp (one of the buildable
# files here) calls GetFullPathName()/SetCurrentDirectory() with narrow
# std::string::c_str() buffers into TCHAR locals -- never actually Unicode
# clean. Defining UNICODE/_UNICODE would resolve those Win32 calls to their *W
# forms and fail to compile against the narrow buffers. Build-config
# difference only; the VC9 project is untouched.

target_link_libraries(hu_coreengine PUBLIC
    OgreMain
    OgreOverlay
    OgreBites
    debug ${HU_OIS_LIBRARY_DEBUG}
    optimized ${HU_OIS_LIBRARY_RELEASE}
    hu_utils
    hu_exceptions
    hu_networking
    hu_zoidcom_compat
    hu_gui
    hu_sound
    hu_ogremax
    Boost::thread
)

# --- hu_sound --------------------------------------------------------------
# Ported (in place, not copied) from HovercraftUniverse/Sound/*.cpp/.h -- the
# faithful VC9 conversion tracked by HovercraftUniverse/Sound/CMakeLists.txt
# (source list mirrored from there; that file is NOT modified). FMOD Ex is a
# prebuilt VC9-era library (HovercraftUniverse/dependencies/FMOD) exposing a
# C API (the C++ classes in fmod.hpp/fmod_event.hpp are thin inline wrappers
# over extern "C" functions), so its VC9 import libs (fmodex_vc.lib,
# fmod_event.lib) link fine against v143 -- verified empirically with a
# throwaway test executable (FMOD::System_Create + System::getVersion) built
# in the build tree; see docs/porting/ for the writeup. No shim needed.
set(HU_SOUND_ROOT ${HU_ROOT}/Sound)

add_library(hu_sound STATIC
    ${HU_SOUND_ROOT}/Moveable3DEmitter.cpp
    ${HU_SOUND_ROOT}/SoundManager.cpp
    ${HU_SOUND_ROOT}/Moveable3DEmitter.h
    ${HU_SOUND_ROOT}/SoundManager.h
)

target_include_directories(hu_sound PUBLIC
    ${HU_SOUND_ROOT}
    ${HU_ROOT}/dependencies/FMOD/includes
)

target_compile_definitions(hu_sound PRIVATE
    WIN32
    _LIB
    _MBCS
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

target_link_libraries(hu_sound PUBLIC
    OgreMain
    ${HU_ROOT}/dependencies/FMOD/lib/fmodex_vc.lib
    ${HU_ROOT}/dependencies/FMOD/lib/fmod_event.lib
)

# --- Lua 5.1 + LuaBind (third_party) ---------------------------------------
# Vendored (FetchContent-built) dependencies for hu_scripting below. See
# docs/porting/lua-luabind.md and third_party/CMakeLists.txt (owned by this
# porting effort, not a port of anything under HovercraftUniverse/) for the
# full rationale: vcpkg's `lua` port is 5.5 (source-incompatible with this
# game's Lua-5.1-era scripts/LuaBind), and there is no vcpkg port for LuaBind
# at all. Provides hu_lua and hu_luabind targets.
add_subdirectory(${CMAKE_SOURCE_DIR}/../third_party third_party)

# --- hu_scripting ------------------------------------------------------------
# Ported (in place, not copied) from HovercraftUniverse/Scripting/*.cpp/.h --
# the faithful VC9 conversion tracked by HovercraftUniverse/Scripting/CMakeLists.txt
# (source list mirrored from there; that file is NOT modified). Builds
# against hu_lua/hu_luabind above instead of the original vendored
# dependencies/lua, dependencies/luabind directories.
set(HU_SCRIPTING_ROOT ${HU_ROOT}/Scripting)

add_library(hu_scripting STATIC
    ${HU_SCRIPTING_ROOT}/OgreLuaBindings.cpp
    ${HU_SCRIPTING_ROOT}/ScriptWrapper.cpp
    ${HU_SCRIPTING_ROOT}/OgreLuaBindings.h
    ${HU_SCRIPTING_ROOT}/ScriptWrapper.h
)

target_include_directories(hu_scripting PUBLIC
    ${HU_SCRIPTING_ROOT}
)

target_compile_definitions(hu_scripting PRIVATE
    WIN32
    _MBCS
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

target_link_libraries(hu_scripting PUBLIC
    hu_lua
    hu_luabind
    OgreMain
)

# --- hu_havok_compat -----------------------------------------------------
# API-compatible, from-scratch replacement for the dead closed-source Havok
# Physics 6.6 SDK (compat/havok/**, owned by this porting effort -- not a
# port of anything under HovercraftUniverse/), backed by Bullet 3.25
# (vcpkg). See docs/porting/havok-compat.md.
add_subdirectory(${CMAKE_SOURCE_DIR}/../compat/havok ${CMAKE_BINARY_DIR}/compat_havok)

# --- hu_skyx_compat --------------------------------------------------------
# API-compatible, from-scratch replacement for SkyX 0.1 (sky/cloud/star
# rendering, LGPL). Real SkyX 0.1 source could not be recovered (only VC9
# headers+binaries survive, see archive-mirror/SkyX_0_1.rar); a real SkyX
# 0.4 source mirror exists (github.com/TommyTeaVee/ogre_skyx) but its API
# already diverged from the 0.1 headers this game targets. See
# docs/porting/skyx-compat.md for the full route-not-taken analysis.
# Sky rendering is inert as a result (InGameState.cpp's mSkyX exists and is
# updated every frame, but draws nothing) -- honestly logged once, not faked.
add_subdirectory(${CMAKE_SOURCE_DIR}/../compat/skyx ${CMAKE_BINARY_DIR}/compat_skyx)

# --- hu_game_physics -------------------------------------------------------
# Ported (in place, not copied) from HovercraftUniverse/HovercraftUniverse/*
# -- the game-logic project (HovercraftUniverse.vcproj / that directory's
# own, untouched CMakeLists.txt). This target builds the subset of its 91
# .cpp files that do not need the not-yet-ported GUI (Hikari), Sound (FMOD)
# or Scripting (Lua/LuaBind) projects, against hu_havok_compat instead of
# the real (dead, VC9-only) Havok prebuilt libs.
#
# The clean/tainted split below was produced by a transitive #include-taint
# analysis (same method as hu_coreengine's ZoidCom/Networking split above):
# starting from each .cpp (and its matching .h), recursively follow local
# quoted #includes and flag any file whose closure reaches GUIManager.h,
# SoundManager.h, or a Scripting header (lua.h/luabind.hpp/
# OgreLuaBindings.h). 78 of 91 files are clean by that measure; see the
# EXCLUDED list at the bottom of this target for the other 13 and exactly
# what blocks each one. Every excluded file still exists on disk, untouched,
# ready to be re-added once its blocking project is ported -- nothing here
# silently drops functionality.
set(HU_GAMEPHYSICS_ROOT ${HU_ROOT}/HovercraftUniverse)

add_library(hu_game_physics STATIC
    ${HU_GAMEPHYSICS_ROOT}/AbstractHavokWorld.cpp
    ${HU_GAMEPHYSICS_ROOT}/AdvancedCollisionPrevention.cpp
    ${HU_GAMEPHYSICS_ROOT}/Asteroid.cpp
    ${HU_GAMEPHYSICS_ROOT}/AsteroidRepresentation.cpp
    ${HU_GAMEPHYSICS_ROOT}/BasePhantom.cpp
    ${HU_GAMEPHYSICS_ROOT}/BoostAction.cpp
    ${HU_GAMEPHYSICS_ROOT}/BoostProperty.cpp
    ${HU_GAMEPHYSICS_ROOT}/BoostRepresentation.cpp
    ${HU_GAMEPHYSICS_ROOT}/CharacterContextContainer.cpp
    ${HU_GAMEPHYSICS_ROOT}/CheckPoint.cpp
    ${HU_GAMEPHYSICS_ROOT}/CheckPointRepresentation.cpp
    ${HU_GAMEPHYSICS_ROOT}/CheckpointEvent.cpp
    ${HU_GAMEPHYSICS_ROOT}/CheckpointPhantom.cpp
    ${HU_GAMEPHYSICS_ROOT}/CollisionEvent.cpp
    ${HU_GAMEPHYSICS_ROOT}/Console.cpp
    ${HU_GAMEPHYSICS_ROOT}/ControlsReader.cpp
    ${HU_GAMEPHYSICS_ROOT}/CustomOgreMaxScene.cpp
    ${HU_GAMEPHYSICS_ROOT}/EntityCollisionPrevention.cpp
    ${HU_GAMEPHYSICS_ROOT}/EntityRegister.cpp
    ${HU_GAMEPHYSICS_ROOT}/Finish.cpp
    ${HU_GAMEPHYSICS_ROOT}/FinishPhantom.cpp
    ${HU_GAMEPHYSICS_ROOT}/FinishRepresentation.cpp
    ${HU_GAMEPHYSICS_ROOT}/GameEvent.cpp
    ${HU_GAMEPHYSICS_ROOT}/GameEventParser.cpp
    ${HU_GAMEPHYSICS_ROOT}/HUDedicatedServer.cpp
    ${HU_GAMEPHYSICS_ROOT}/HUServer.cpp
    ${HU_GAMEPHYSICS_ROOT}/HUServerCore.cpp
    ${HU_GAMEPHYSICS_ROOT}/Havok.cpp
    ${HU_GAMEPHYSICS_ROOT}/HavokEntity.cpp
    ${HU_GAMEPHYSICS_ROOT}/HavokEntityType.cpp
    ${HU_GAMEPHYSICS_ROOT}/HavokHovercraft.cpp
    ${HU_GAMEPHYSICS_ROOT}/HavokHovercraftCollisionEffect.cpp
    ${HU_GAMEPHYSICS_ROOT}/HavokThread.cpp
    ${HU_GAMEPHYSICS_ROOT}/HoverAction.cpp
    ${HU_GAMEPHYSICS_ROOT}/HoverCraftUniverseWorld.cpp
    ${HU_GAMEPHYSICS_ROOT}/Hovercraft.cpp
    ${HU_GAMEPHYSICS_ROOT}/HovercraftController.cpp
    ${HU_GAMEPHYSICS_ROOT}/HovercraftPlayerController.cpp
    ${HU_GAMEPHYSICS_ROOT}/HovercraftUniverseEntity.cpp
    ${HU_GAMEPHYSICS_ROOT}/InitEvent.cpp
    ${HU_GAMEPHYSICS_ROOT}/Loader.cpp
    ${HU_GAMEPHYSICS_ROOT}/Lobby.cpp
    ${HU_GAMEPHYSICS_ROOT}/PhantomTrackAction.cpp
    ${HU_GAMEPHYSICS_ROOT}/PlanetGravityAction.cpp
    ${HU_GAMEPHYSICS_ROOT}/PlanetGravityPhantom.cpp
    ${HU_GAMEPHYSICS_ROOT}/PlayerSettings.cpp
    ${HU_GAMEPHYSICS_ROOT}/PlayerSettingsInterceptor.cpp
    ${HU_GAMEPHYSICS_ROOT}/Portal.cpp
    ${HU_GAMEPHYSICS_ROOT}/PortalPhantom.cpp
    ${HU_GAMEPHYSICS_ROOT}/PortalRepresentation.cpp
    ${HU_GAMEPHYSICS_ROOT}/PowerupPhantom.cpp
    ${HU_GAMEPHYSICS_ROOT}/PowerupSpawn.cpp
    ${HU_GAMEPHYSICS_ROOT}/ProgressMonitor.cpp
    ${HU_GAMEPHYSICS_ROOT}/RacePlayer.cpp
    ${HU_GAMEPHYSICS_ROOT}/RaceState.cpp
    ${HU_GAMEPHYSICS_ROOT}/ResetSpawn.cpp
    ${HU_GAMEPHYSICS_ROOT}/ServerWorldLoader.cpp
    ${HU_GAMEPHYSICS_ROOT}/SpeedBoost.cpp
    ${HU_GAMEPHYSICS_ROOT}/SpeedBoostPhantom.cpp
    ${HU_GAMEPHYSICS_ROOT}/Start.cpp
    ${HU_GAMEPHYSICS_ROOT}/StartPhantom.cpp
    ${HU_GAMEPHYSICS_ROOT}/StartPosition.cpp
    ${HU_GAMEPHYSICS_ROOT}/StartRepresentation.cpp
    ${HU_GAMEPHYSICS_ROOT}/StateEvent.cpp
    ${HU_GAMEPHYSICS_ROOT}/StaticBody.cpp
    ${HU_GAMEPHYSICS_ROOT}/StaticBodyEntity.cpp
    ${HU_GAMEPHYSICS_ROOT}/StaticBodyRepresentation.cpp
    ${HU_GAMEPHYSICS_ROOT}/Track.cpp
    ${HU_GAMEPHYSICS_ROOT}/TrackInfoLoader.cpp
    ${HU_GAMEPHYSICS_ROOT}/UpdatePositionAction.cpp
    ${HU_GAMEPHYSICS_ROOT}/VisualEvent.cpp
    ${HU_GAMEPHYSICS_ROOT}/VisualEventParser.cpp

    # Re-enabled now that hu_sound/hu_scripting exist (previously blocked
    # purely by Sound/Scripting #include taint; verified to compile clean):
    ${HU_GAMEPHYSICS_ROOT}/HovercraftAIController.cpp
    ${HU_GAMEPHYSICS_ROOT}/ServerLoader.cpp

    # Re-enabled: boost::xtime/boost::thread::sleep(xtime) (removed from
    # modern Boost) replaced in place with
    # boost::this_thread::sleep_for(boost::chrono::milliseconds(1)) --
    # same 1ms sleep, modern chrono-based API. See docs/porting/ for detail.
    ${HU_GAMEPHYSICS_ROOT}/PathRecorder.cpp
    ${HU_GAMEPHYSICS_ROOT}/HUServerThread.cpp

    # Re-enabled now that hu_gui exists (previously EXCLUDED (A): transitively
    # #include GUIManager.h/SoundManager.h/Scripting headers; hu_sound/
    # hu_scripting already unblocked the Sound/Scripting half of the taint,
    # hu_gui unblocks the rest). See docs/porting/hikari-gui.md.
    # main.cpp is deliberately NOT re-added here -- it's the WinMain entry
    # point and belongs to the HovercraftUniverse exe target instead (see
    # that target below), not this static lib.
    ${HU_GAMEPHYSICS_ROOT}/ClientPreparationLoader.cpp
    ${HU_GAMEPHYSICS_ROOT}/HUApplication.cpp
    ${HU_GAMEPHYSICS_ROOT}/HUClient.cpp
    ${HU_GAMEPHYSICS_ROOT}/HUD.cpp
    ${HU_GAMEPHYSICS_ROOT}/HovercraftRepresentation.cpp
    ${HU_GAMEPHYSICS_ROOT}/LobbyGUI.cpp
    ${HU_GAMEPHYSICS_ROOT}/MainMenu.cpp
    ${HU_GAMEPHYSICS_ROOT}/ClientConnectThread.cpp

    # Re-enabled now that hu_skyx_compat exists (see docs/porting/skyx-compat.md).
    # InGameState.cpp/.h #include <SkyX.h> and use the SkyX::SkyX sky
    # rendering plugin class directly; MainMenuState.cpp #includes
    # InGameState.h to transition into it; LobbyState.cpp does the same AND
    # directly instantiates `new InGameState(...)`. All three build clean
    # against compat/skyx's shim (sky rendering itself is inert -- see that
    # doc -- but the game code compiles and links unchanged).
    ${HU_GAMEPHYSICS_ROOT}/InGameState.cpp
    ${HU_GAMEPHYSICS_ROOT}/MainMenuState.cpp
    ${HU_GAMEPHYSICS_ROOT}/LobbyState.cpp

    # Re-enabled: ClientLoader.cpp's Ogre 1.7->14 migration (see
    # docs/porting/ogre-api-gap.md and the EXCLUDED (C) note below, now
    # resolved): setShadowTextureCasterMaterial/ReceiverMaterial now take a
    # MaterialPtr (resolved via MaterialManager::getByName(), same fix as
    # OgreMax/OgreMaxScene.cpp's identical call); Ogre::Light::setPosition()
    # no longer exists (nodeless positioning not built) -- the light is now
    # attached to a SceneNode positioned at its authored location, the same
    # pattern already used by GameView.cpp/RaceCamera.cpp. The
    # shadow-camera-setup classes (DefaultShadowCameraSetup and friends) and
    # the Ogre::Animation/TransformKeyFrame APIs turned out to need NO
    # changes at all -- both are still present, unchanged, in this vcpkg
    # Ogre 14.5.2 build (confirmed against
    # C:\Users\dirk\vcpkg\installed\x86-windows\include\OGRE\
    # OgreShadowCameraSetup*.h), and ClientLoader.cpp's onSkyBox/onSkyDome/
    # onNode NodeAnimation parameters were always unused pass-throughs, not
    # actual Animation/KeyFrame API calls -- the original exclusion comment's
    # assumption that a "nontrivial Ogre migration pass" was needed there was
    # wrong; the real fix was two call sites.
    ${HU_GAMEPHYSICS_ROOT}/ClientLoader.cpp

    # Re-enabled: HovercraftLoader.cpp. Re-enabling InGameState.cpp/
    # MainMenuState.cpp/LobbyState.cpp above changed the exe's link closure
    # enough that the linker now DOES pull in HUClient.obj's
    # HovercraftLoader::HovercraftLoader/~HovercraftLoader call site (it did
    # not before, when this file's exclusion was merely latent -- see the
    # earlier revision of this comment). On actually compiling it: the
    # RaceCamera/GameView Ogre-api-gap breakage this file was assumed to
    # inherit transitively (via #include "GameView.h") turned out to already
    # be fixed (see docs/porting/hikari-gui.md section 7 -- RaceCamera.cpp/
    # GameView.cpp were ported before this task started), and its
    # Ogre::Animation/NodeAnimationTrack/TransformKeyFrame calls in onNode()
    # needed NO changes -- that API is unchanged between Ogre 1.7 and 14.5.2
    # (confirmed against OgreAnimation.h/OgreAnimationTrack.h/OgreKeyFrame.h).
    # This file compiles clean, unmodified, once its two real blockers
    # (RaceCamera/GameView, now already fixed) are out of the way -- the
    # original exclusion comment's assumption of "a real Ogre migration pass
    # needed" was stale.
    ${HU_GAMEPHYSICS_ROOT}/HovercraftLoader.cpp
    #
    # EXCLUDED (B) -- NOT GUI/Sound/Scripting-tainted; each blocked by its
    # own genuine, pre-existing, non-Havok bug in HovercraftUniverse/**
    # (confirmed by actually attempting to compile each file against
    # hu_havok_compat -- same "document rather than hack" rule as (A) and
    # as the RaceCamera/GameView/EntityPropertySystem findings in
    # docs/porting/zoidcom-compat.md). See docs/porting/havok-compat.md for
    # detail on each:
    #   PortalData.cpp  -- #includes "PortalData.h", which does not exist
    #                       anywhere in the repository (an orphaned .cpp
    #                       with no matching header -- the header was
    #                       apparently never committed, or removed without
    #                       removing the .cpp).
    #   Portal.cpp -- FIXED and re-enabled above: called the Ogre 1.x
    #                 `Ogre::vector<Ogre::String>::type` + `Ogre::StringUtil::split()`
    #                 idiom (same class as docs/porting/ogre-api-gap.md's row 1,
    #                 just not caught by that document's original pass
    #                 because this file was Havok-blocked at the time);
    #                 replaced with `std::vector<Ogre::String>` (2 call
    #                 sites). Discovered/confirmed necessary because linking
    #                 the HovercraftUniverse exe target (see that target
    #                 below) surfaced unresolved Portal::Portal/onEnter/
    #                 onLeave/load/getClassName symbols referenced from
    #                 EntityRegister.cpp, PortalPhantom.cpp, ServerLoader.cpp.
    #
    # (PathRecorder.cpp/HUServerThread.cpp were also excluded here for
    # boost::xtime/boost::TIME_UTC breakage -- fixed in place, see above;
    # both now build clean.)
    #
    # EXCLUDED (C) -- were assumed Sound-only-tainted before this task (via
    # <Moveable3DEmitter.h>, see the ClientLoader.cpp/HovercraftLoader.cpp
    # note that used to live under EXCLUDED (A) above), but actually
    # compiling each against hu_sound/hu_scripting shows they are ALSO
    # blocked by real, independent Ogre-api-gap breakage that hu_sound does
    # not touch -- confirmed empirically by attempting the compile:
    #   ClientLoader.cpp    -- Ogre::SceneManager::setShadowTextureCasterMaterial/
    #                          setShadowTextureReceiverMaterial now take a
    #                          MaterialPtr, not a name string;
    #                          Ogre::DefaultShadowCameraSetup/FocusedShadowCameraSetup/
    #                          LiSPSMShadowCameraSetup/PlaneOptimalShadowCameraSetup
    #                          have all been renamed/moved in modern Ogre;
    #                          Ogre::Light::setPosition() no longer exists
    #                          (same SceneNode-based idiom change as
    #                          Ogre::Camera, see OgreLuaBindings.cpp's
    #                          Camera_setPosition/Camera_lookAt fix for the
    #                          general pattern -- not applied here, out of
    #                          scope for this task).
    #   HovercraftLoader.cpp -- includes GameView.h, which includes
    #                          CoreEngine/RaceCamera.h, which calls
    #                          Ogre::Camera::setPosition/setDirection --
    #                          exactly the already-documented RaceCamera.cpp
    #                          Ogre-api-gap breakage above (hu_coreengine's
    #                          EXCLUDED (A) list); additionally calls
    #                          Ogre::Animation/TransformKeyFrame APIs that no
    #                          longer resolve the same way. Both files need a
    #                          real Ogre 1.7->14 migration pass (shadow
    #                          camera setups, Light/Camera node-based
    #                          positioning, Animation/KeyFrame API), not a
    #                          Sound/Scripting fix -- out of scope here.
)

target_include_directories(hu_game_physics PUBLIC
    # NOTE: listed ahead of hu_sound's own PUBLIC include dir (which is
    # pulled in transitively via the target_link_libraries below and also
    # contains ${HU_ROOT}/dependencies/FMOD/includes/HovSound.h -- a STALE
    # copy missing MUSICCUE_HOVSOUND_MENU, present only in this real,
    # up-to-date data/sound/HovSound.h). InGameState.cpp/MainMenuState.cpp's
    # #include <HovSound.h> (angle-bracket, i.e. include-path search order
    # dependent) resolved to the stale FMOD copy before this entry existed,
    # which is why re-enabling those files (see EXCLUDED note above) surfaced
    # "MUSICCUE_HOVSOUND_MENU: undeclared identifier" -- a real,
    # pre-existing include-path ambiguity between two out-of-sync copies of
    # the same generated header, not a new bug in this port. Putting the
    # real copy first (build-config fix, not a game-code edit) makes the
    # angle-bracket include resolve to the complete, current header.
    ${HU_ROOT}/data/sound
    ${HU_GAMEPHYSICS_ROOT}
    ${HU_COREENGINE_ROOT}
    ${HU_NETWORKING_ROOT}
    ${HU_OGREMAX_ROOT}
    ${HU_ROOT}/Utils
    ${HU_OIS_INCLUDE_DIR}
    ${HU_ROOT}/GUI
    ${HU_ROOT}/Sound
    ${HU_ROOT}/Scripting
)

target_compile_definitions(hu_game_physics PRIVATE
    WIN32
    _LIB
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
    # See hu_coreengine's identical note above -- the same
    # using-namespace-std-plus-COM-headers `byte` ambiguity resurfaces here
    # now that GUI-tainted files (HUD.cpp etc) pull in GUIManager.h/Hikari.h.
    _HAS_STD_BYTE=0
)
# Deliberately NOT defining UNICODE/_UNICODE -- same reasoning as
# hu_ogremax/hu_coreengine above (this project's real .vcproj does define
# it, but several files here call narrow-buffer Win32 APIs / std::string
# APIs that are not actually Unicode-clean).

if(MSVC)
    # Deliberately re-enabling permissive (non-conformant) parsing for THIS
    # target only, overriding the top-level `/permissive-` set above.
    # HovercraftUniverse/Utils/PlayerMapImpl.h (a template, `PlayerMap<Key,
    # Player, Unique>::find()`) uses several dependent-type names
    # (`list_type::iterator`) without the `typename` keyword they
    # technically need; MSVC's classic (permissive) two-phase lookup
    # tolerates that, strict `/permissive-` two-phase lookup does not
    # (C7510/C2065/C3878 cascades). Likewise
    # HovercraftUniverse/HovercraftUniverse/CustomOgreMaxScene.h declares
    # `Ogre::String CustomOgreMaxScene::getUniqueName();` *inside* the
    # class body it belongs to (a redundant, invalid qualifier) -- rejected
    # under strict conformance as C4596, tolerated under permissive parsing.
    # Both are genuine, pre-existing bugs in code this porting effort is not
    # allowed to touch (HovercraftUniverse/**); relaxing conformance for
    # just this one target is the least invasive fix available. See
    # docs/porting/havok-compat.md.
    target_compile_options(hu_game_physics PRIVATE /permissive)

    # Havok.h/HavokThread.h (both HovercraftUniverse/**, not ours to edit)
    # declare `DWORD WINAPI runHavok(LPVOID lpParam);` without ever
    # #including <windows.h> themselves -- in the original VC9 .vcproj this
    # worked because some other header earlier in that project's
    # precompiled-header chain already pulled in <windows.h> first. This
    # target has no precompiled header, so the same real, pre-existing gap
    # now surfaces as DWORD/WINAPI/LPVOID being undeclared. Forcing
    # <windows.h> in ahead of every translation unit reproduces the old
    # implicit-PCH behavior without touching game code.
    target_compile_options(hu_game_physics PRIVATE /FI windows.h)
endif()

# RaceState.h needs boost::random (a Mersenne-twister RNG for lobby/track
# selection) and HUClient.h/ClientConnectThread.cpp need
# boost::interprocess (a semaphore) -- both genuine, narrow dependency
# gaps (the packages simply weren't in the original bootstrap script's
# vcpkg install list), fixed the same way boost-thread was added for
# hu_coreengine. Installed via:
#   vcpkg install boost-random:x86-windows boost-interprocess:x86-windows
# scripts/bootstrap-modern-deps.ps1 should gain these too (tracked
# separately; not this task's file to edit blindly, but flagged here).
find_package(Boost REQUIRED COMPONENTS random)
find_package(Boost QUIET COMPONENTS interprocess) # header-only; component mainly used to confirm presence

target_link_libraries(hu_game_physics PUBLIC
    hu_havok_compat
    hu_skyx_compat
    hu_coreengine
    hu_networking
    hu_zoidcom_compat
    hu_ogremax
    hu_utils
    hu_exceptions
    hu_sound
    hu_scripting
    hu_gui
    OgreMain
    OgreOverlay
    Boost::thread
    Boost::random
    # winmm: timeBeginPeriod/timeEndPeriod, used by HavokThread.cpp's physics
    # timing loop (see docs/porting/timing-and-smoothing.md) to raise Windows
    # timer granularity so Sleep() is accurate enough for a 60 Hz+ physics tick.
    winmm
)

# --- HovercraftUniverse (exe) ------------------------------------------------
# WIN32-subsystem executable, WinMain entry point, mirroring the faithful
# VC9 project's structure (HovercraftUniverse/HovercraftUniverse/main.cpp +
# HovercraftUniverse.vcproj's own AdditionalDependencies linking OgreMax,
# GUI, Sound, Exceptions, Scripting, Utils, CoreEngine, Networking, plus the
# prebuilt third-party import libs) -- see that project's own untouched
# CMakeLists.txt for the full faithful accounting. In this modern tree,
# hu_game_physics already PUBLIC-links every one of those (hu_coreengine,
# hu_networking, hu_ogremax, hu_gui, hu_sound, hu_scripting, hu_utils,
# hu_exceptions, hu_zoidcom_compat, hu_havok_compat, OgreMain, OgreOverlay),
# so main.cpp only needs to be compiled and linked against that one target.
# main.cpp itself needed two in-place fixes (see
# docs/porting/hikari-gui.md): the `Ogre::vector<Ogre::String>::type` /
# `Ogre::StringUtil::split` idiom (removed in Ogre 14, same class of issue
# as docs/porting/ogre-api-gap.md's row 1) and an explicit
# `#include <zoidcom/zoidcom.h>` (ZoidCom's declaration was previously only
# ever reached transitively via the VC9 project's precompiled-header
# chain).
add_executable(HovercraftUniverse WIN32
    ${HU_GAMEPHYSICS_ROOT}/main.cpp
)

target_compile_definitions(HovercraftUniverse PRIVATE
    WIN32
    _WINDOWS
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
    # See hu_game_physics's identical note above (using-namespace-std +
    # COM-headers `byte` ambiguity).
    _HAS_STD_BYTE=0
)
# Deliberately NOT defining UNICODE/_UNICODE -- same reasoning as
# hu_coreengine/hu_game_physics above (the VC9 .vcproj does define it, but
# main.cpp's dependency closure includes narrow-buffer Win32 API calls that
# are not actually Unicode-clean).

if(MSVC)
    # See hu_game_physics's identical settings above for why: PlayerMapImpl.h
    # (permissive two-phase-lookup relaxation) and Havok.h/HavokThread.h
    # (missing <windows.h> before DWORD/WINAPI/LPVOID) are both reachable
    # from main.cpp's #include closure.
    target_compile_options(HovercraftUniverse PRIVATE /permissive /FI windows.h)
endif()

target_link_libraries(HovercraftUniverse PRIVATE
    hu_game_physics
    OgreMain
    OgreOverlay
    OgreBites
)
