cmake_minimum_required(VERSION 3.20)
project(HovercraftUniverseModern CXX)

# This is a SEPARATE, modern CMake build tree, independent of the faithful
# VC9 conversion under HovercraftUniverse/CMakeLists.txt (which must not be
# touched). Its purpose is to prove that HovercraftUniverse/Utils and
# HovercraftUniverse/Exceptions compile clean under the v143 toolchain, with
# sources referenced in place (not copied).

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W3 /permissive-)
endif()

set(HU_ROOT ${CMAKE_SOURCE_DIR}/../HovercraftUniverse)

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
# boost::posix_time respectively). No third-party dependencies are allowed
# in this modern tree, and Boost is not present in the vcpkg install used
# here (only assimp/enet/libpng/... are installed). These two files are
# therefore excluded from hu_utils below; see modern/README-status notes
# in the CMake configure output / task report for details.
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

# Optional: build Config.cpp / Timing.cpp only if Boost is available and the
# caller explicitly opts in. Off by default to keep this tree free of
# third-party dependencies.
option(HU_WITH_BOOST_UTILS "Also build Config.cpp/Timing.cpp (requires Boost)" OFF)
if(HU_WITH_BOOST_UTILS)
    find_package(Boost REQUIRED COMPONENTS date_time)
    target_sources(hu_utils PRIVATE
        ${HU_ROOT}/Utils/Config.cpp
        ${HU_ROOT}/Utils/Config.h
        ${HU_ROOT}/Utils/Timing.cpp
        ${HU_ROOT}/Utils/Timing.h
    )
    target_link_libraries(hu_utils PUBLIC Boost::boost Boost::date_time)
endif()
