cmake_minimum_required(VERSION 3.20)

# ============================================================================
# HovercraftUniverse - CMake port of the original Visual Studio 2008 solution
# ============================================================================
#
# This is a *faithful* conversion of HovercraftUniverse.sln (VS2008, .vcproj
# format, "Debug|Win32" / "Release|Win32" configurations only) to CMake.
# It is intentionally NOT a modernization: preprocessor defines, include
# directories, library lists, runtime library selection (/MDd, /MD) and
# output layout are reproduced exactly as they were in the original .vcproj
# files -- including a few pre-existing oddities/asymmetries in the original
# project, which are called out with comments where they occur (see
# CoreEngine/CMakeLists.txt and HovercraftUniverse/CMakeLists.txt in
# particular).
#
# Toolset requirement
# --------------------
# All of the closed-source, prebuilt third-party libraries under
# dependencies/ (Havok/Havoc, OGRE, OIS, Hikari, FMOD, ZoidCom, SkyX, lua,
# luabind, boost) were built with the Visual C++ 9.0 (VS2008) compiler/ABI.
# They can only be *linked against* successfully with the matching v90
# platform toolset. Generate/build with, e.g.:
#
#     cmake -S HovercraftUniverse -B build -G "Visual Studio 17 2022" -A Win32 -T v90
#
# (VS2008 must be installed side-by-side with the newer IDE for the v90
# toolset to be available.) Configuring/building with any other toolset
# (v140, v141, v142, v143, ...) is expected to CONFIGURE and COMPILE fine,
# but FAIL AT LINK TIME, because the prebuilt .lib files are only ABI/CRT
# compatible with the VC9 runtime and name-mangling scheme. That is a known,
# accepted limitation of this conversion -- not a bug in these CMake files.
# Opening the generated solution in any IDE (or a newer default toolset)
# works fine up to that same link boundary.
#
# Populating dependencies/
# -------------------------
# The dependencies/ directory (already present in this working copy) is
# normally (re-)populated by scripts/bootstrap-dependencies.ps1 at the
# repository root, which fetches/unpacks the prebuilt third-party SDKs
# referenced above (OGRE, OIS, Havok, SkyX, Hikari, FMOD, ZoidCom, lua,
# luabind, boost). Run that script first if dependencies/ is missing or
# incomplete.
#
# Only Debug and Release configurations existed in the original solution
# (no RelWithDebInfo / MinSizeRel), so CMAKE_CONFIGURATION_TYPES below is
# restricted to match.
# ============================================================================

# CMP0091 must be set before the first project() call so that
# CMAKE_MSVC_RUNTIME_LIBRARY (used below to select /MDd vs /MD, matching
# RuntimeLibrary="3"/"2" in every .vcproj) actually takes effect.
cmake_policy(SET CMP0091 NEW)

project(HovercraftUniverse CXX)

# Only Debug and Release configurations existed in the original solution.
# On multi-config generators (Visual Studio, Ninja Multi-Config, ...) this
# trims the config list down to match; on single-config generators
# (Makefiles, single-config Ninja) CMAKE_CONFIGURATION_TYPES has no effect,
# so fall back to CMAKE_BUILD_TYPE=Debug there instead.
get_property(HU_IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(HU_IS_MULTI_CONFIG)
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING
        "Reduced configuration list matching the original VS2008 solution (Debug|Win32, Release|Win32 only)" FORCE)
elseif(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type (matches original VS2008 solution default)" FORCE)
endif()

# Matches RuntimeLibrary="3" (Debug -> /MDd, MultiThreadedDebugDLL) and
# RuntimeLibrary="2" (Release -> /MD, MultiThreadedDLL) used by every single
# project (CoreEngine, OgreMax, GUI, Networking, Sound, Scripting,
# Exceptions, Utils, HovercraftUniverse) in the original solution.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

# All static libs get the "_d" debug suffix (GUI_d.lib, Sound_d.lib, ...),
# matching each project's VCLibrarianTool OutputFile="$(OutDir)\$(ProjectName)_d.lib"
# override in Debug (Release falls back to the VS default $(ProjectName).lib,
# i.e. no suffix). The HovercraftUniverse.exe target explicitly clears this
# postfix back to "" since the original exe was NOT suffixed in Debug.
set(CMAKE_DEBUG_POSTFIX "_d")

# ----------------------------------------------------------------------------
# Output layout
# ----------------------------------------------------------------------------
# Static libs -> lib/Debug, lib/Release, matching every library project's
# original OutputDirectory="$(SolutionDir)lib\$(ConfigurationName)".
#
# Executable -> bin/debug, bin/release (lowercase). The original vcproj used
# OutputDirectory="$(SolutionDir)bin\$(ConfigurationName)" (i.e. literally
# "bin\Debug" / "bin\Release"), but Windows paths are case-insensitive and
# the bin/debug and bin/release folders (and their start_client.bat /
# start_server.bat / server-client.bat launcher scripts) already checked
# into this repository use lowercase, so we match that on-disk convention.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_CURRENT_SOURCE_DIR}/lib/Debug")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/lib/Release")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_CURRENT_SOURCE_DIR}/bin/debug")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/bin/release")

# Single prefix for all third-party dependency include/lib paths, mirroring
# "$(ProjectDir)..\dependencies\..." used throughout every .vcproj.
set(HU_DEPS "${CMAKE_CURRENT_SOURCE_DIR}/dependencies")

# Build order below follows the original solution's inter-project
# dependencies (see HovercraftUniverse.sln ProjectDependencies sections):
# Exceptions has no internal dependencies; Utils and OgreMax depend only on
# Exceptions/third-party; GUI/Networking/Sound/Scripting depend on
# Exceptions (and, for Networking, Utils); CoreEngine depends on all of the
# above; HovercraftUniverse (the exe) depends on everything.
add_subdirectory(Exceptions)
add_subdirectory(Utils)
add_subdirectory(OgreMax)
add_subdirectory(GUI)
add_subdirectory(Networking)
add_subdirectory(Sound)
add_subdirectory(Scripting)
add_subdirectory(CoreEngine)
add_subdirectory(HovercraftUniverse)
