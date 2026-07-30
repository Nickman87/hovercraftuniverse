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

    # TODO(modernize): still excluded, but NOT for ZoidCom/Networking reasons
    # anymore -- the shim resolves that taint. Two independent kinds of
    # blocker remain, both out of scope for this task (compat/zoidcom/**
    # and modern/CMakeLists.txt are owned here; HovercraftUniverse/** is
    # read-only). Confirmed by actually attempting to compile each file
    # against hu_zoidcom_compat/hu_networking:
    #
    #   (A) Pre-existing Ogre API-gap breakage (see docs/porting/ogre-api-gap.md
    #       for the same class of issue elsewhere), independent of this task:
    #     - RaceCamera.cpp/.h -- calls Ogre::Camera::setPosition/setDirection/
    #       lookAt/setFixedYawAxis/pitch/yaw/roll/getOrientation/getPosition,
    #       none of which exist anymore on Ogre::Camera in Ogre 14.5.2 (Camera
    #       no longer derives from Frustum/MovableObject the same way; those
    #       calls now go through Camera::getParentSceneNode() or similar).
    #     - GameView.cpp/.h -- includes RaceCamera.h (same breakage,
    #       transitively) and additionally calls Ogre::Light::setPosition(),
    #       also removed/moved in modern Ogre.
    #     - EntityPropertySystem.cpp/.h -- includes <zoidcom/zoidcom.h>, which
    #       now resolves fine, but EntityPropertySystem.cpp itself uses
    #       std::cout (lines ~239/252/262) without #include <iostream> --
    #       a latent, pre-existing bug in this file, unrelated to ZoidCom.
    #
    #   (B) Still GUI/Sound-tainted (unchanged from before this task):
    #     - EntityRepresentation.cpp/.h -- includes Entity.h (no longer
    #       tainted) but EntityRepresentation.cpp ALSO directly #includes
    #       "Application.h", which pulls in <GUIManager.h>/<SoundManager.h>
    #       (GUI, Sound projects, not yet ported).
    #     - BasicGameState.cpp/.h -- includes <GUIManager.h> and <SoundManager.h> directly.
    #     - GameStateManager.cpp -- #includes "BasicGameState.h" in full (GUI/Sound
    #       taint above). GameStateManager.h itself only forward-declares
    #       BasicGameState and has no other blocker, but its .cpp cannot build
    #       without BasicGameState's full definition.
    #     - RepresentationManager.cpp/.h -- includes EntityManager.h/
    #       EntityRepresentation.h/GameView.h (still tainted per (A)/(B) above)
    #       AND <GUIManager.h>/<SoundManager.h> directly (GUI, Sound, not yet ported).
    #     - Application.cpp/.h -- includes EntityManager.h/RepresentationManager.h
    #       (RepresentationManager still tainted) and <GUIManager.h>/
    #       <SoundManager.h> directly (GUI, Sound, not yet ported).
    #
    # Inert (header-only, no matching .cpp in CoreEngine.vcproj, kept for
    # source-list parity even though nothing here #includes them):
    ${HU_COREENGINE_ROOT}/EntityPropertySystem.h
    ${HU_COREENGINE_ROOT}/EntityRepresentation.h
    ${HU_COREENGINE_ROOT}/GameView.h
    ${HU_COREENGINE_ROOT}/RaceCamera.h
    ${HU_COREENGINE_ROOT}/GameStateManager.h
    ${HU_COREENGINE_ROOT}/RepresentationManager.h
    ${HU_COREENGINE_ROOT}/Application.h
    ${HU_COREENGINE_ROOT}/BasicGameState.h
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
find_library(HU_OIS_LIBRARY NAMES OIS)

# EntityPropertySystem.cpp/.h (re-enabled above) uses boost::mutex
# (<boost/thread/mutex.hpp>), which is a genuine, non-portable-fix-able
# dependency, same as hu_utils's Boost usage above. boost-thread is part of
# the vcpkg package set scripts/bootstrap-modern-deps.ps1 installs.
find_package(Boost REQUIRED COMPONENTS thread)

target_include_directories(hu_coreengine PUBLIC
    ${HU_COREENGINE_ROOT}
    ${HU_OIS_INCLUDE_DIR}
)

target_compile_definitions(hu_coreengine PRIVATE
    WIN32
    _LIB
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
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
    ${HU_OIS_LIBRARY}
    hu_utils
    hu_exceptions
    hu_networking
    hu_zoidcom_compat
    Boost::thread
)

# --- hu_havok_compat -----------------------------------------------------
# API-compatible, from-scratch replacement for the dead closed-source Havok
# Physics 6.6 SDK (compat/havok/**, owned by this porting effort -- not a
# port of anything under HovercraftUniverse/), backed by Bullet 3.25
# (vcpkg). See docs/porting/havok-compat.md.
add_subdirectory(${CMAKE_SOURCE_DIR}/../compat/havok ${CMAKE_BINARY_DIR}/compat_havok)

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

    # EXCLUDED (A) -- transitively #include GUIManager.h / SoundManager.h /
    # Scripting (lua.h, luabind.hpp, OgreLuaBindings.h), none of which are
    # ported yet. Left on disk untouched; re-add once their blocking
    # project is ported:
    #   ClientPreparationLoader.cpp  -- GUI, Sound
    #   HUApplication.cpp            -- GUI, Sound
    #   HUClient.cpp                 -- GUI, Sound, Scripting
    #   HUD.cpp                      -- GUI
    #   HovercraftAIController.cpp   -- Scripting
    #   HovercraftRepresentation.cpp -- GUI, Sound
    #   InGameState.cpp              -- GUI, Sound
    #   LobbyGUI.cpp                 -- GUI
    #   LobbyState.cpp               -- GUI, Sound
    #   MainMenu.cpp                 -- GUI
    #   MainMenuState.cpp            -- GUI, Sound
    #   ServerLoader.cpp             -- Scripting
    #   main.cpp                     -- GUI, Sound (WinMain entry point itself)
    #   ClientLoader.cpp             -- transitively Sound-tainted: includes
    #                                    HovercraftRepresentation.h, which
    #                                    #includes <Moveable3DEmitter.h>
    #                                    (angle-bracket include, missed by
    #                                    the taint scan above, which only
    #                                    followed quoted includes) --
    #                                    HovercraftUniverse/Sound/Moveable3DEmitter.h,
    #                                    Sound project, not ported.
    #   HovercraftLoader.cpp         -- same Moveable3DEmitter.h/Sound taint,
    #                                    via the same #include chain.
    #   ClientConnectThread.cpp      -- ClientConnectThread.h #includes
    #                                    "ConnectListener.h", which only
    #                                    exists under HovercraftUniverse/GUI/
    #                                    (GUI project, not ported); also
    #                                    missed by the taint scan above
    #                                    because ConnectListener.h itself
    #                                    doesn't happen to #include
    #                                    GUIManager.h/SoundManager.h.
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
    #   Portal.cpp      -- calls the Ogre 1.x `Ogre::vector<Ogre::String>::type`
    #                       + `Ogre::StringUtil::split()` idiom, both removed
    #                       in Ogre 14.5.2 -- an ogre-api-gap issue (same
    #                       class as docs/porting/ogre-api-gap.md), just not
    #                       caught by that document's original pass because
    #                       this file was Havok-blocked at the time.
    #   PathRecorder.cpp,
    #   HUServerThread.cpp -- both call `boost::xtime`/`boost::TIME_UTC`
    #                          (removed from modern Boost;
    #                          `boost::thread::sleep(xtime)` was deprecated
    #                          and dropped years ago in favor of
    #                          `boost::this_thread::sleep_for`) -- a
    #                          boost-version-api-gap issue, unrelated to
    #                          Havok. (HUServerThread.cpp's own dedicated-
    #                          server tick loop calls this to pace itself;
    #                          HavokThread.cpp, which drives the physics
    #                          step loop itself, uses plain Win32 Sleep()
    #                          instead and is unaffected.)
)

target_include_directories(hu_game_physics PUBLIC
    ${HU_GAMEPHYSICS_ROOT}
    ${HU_COREENGINE_ROOT}
    ${HU_NETWORKING_ROOT}
    ${HU_OGREMAX_ROOT}
    ${HU_ROOT}/Utils
    ${HU_OIS_INCLUDE_DIR}
)

target_compile_definitions(hu_game_physics PRIVATE
    WIN32
    _LIB
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
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
    hu_coreengine
    hu_networking
    hu_zoidcom_compat
    hu_ogremax
    hu_utils
    hu_exceptions
    OgreMain
    OgreOverlay
    Boost::thread
    Boost::random
    # winmm: timeBeginPeriod/timeEndPeriod, used by HavokThread.cpp's physics
    # timing loop (see docs/porting/timing-and-smoothing.md) to raise Windows
    # timer granularity so Sleep() is accurate enough for a 60 Hz+ physics tick.
    winmm
)
