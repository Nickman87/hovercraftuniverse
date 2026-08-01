# hu_zoidcom_compat -- API-compatible replacement for the dead closed-source
# ZoidCom networking library, built from scratch for the "revival" modern
# (v143, x86-windows) porting effort. See docs/porting/zoidcom-compat.md for
# the design and Phase A/B split.
#
# Public include layout mirrors the real ZoidCom SDK
# (HovercraftUniverse/dependencies/zoidcom/include/zoidcom/*.h) so that game
# code doing `#include <zoidcom/zoidcom.h>` etc. is unaffected.

add_library(hu_zoidcom_compat STATIC
    src/ZoidComGlobal.cpp
    src/BitStream.cpp
    src/Address.cpp
    src/ConnGroup.cpp
    src/Control.cpp
    src/Node.cpp
    src/Replicator.cpp
    src/zoidcom_shim_internal.h

    include/zoidcom/zoidcom.h
    include/zoidcom/zoidcom_prereq.h
    include/zoidcom/zoidcom_bitstream.h
    include/zoidcom/zoidcom_address.h
    include/zoidcom/zoidcom_conngroup.h
    include/zoidcom/zoidcom_control.h
    include/zoidcom/zoidcom_node.h
    include/zoidcom/zoidcom_node_interceptors.h
    include/zoidcom/zoidcom_replicator.h
    include/zoidcom/zoidcom_replicator_basic.h
    include/zoidcom/zoidcom_replicator_advanced.h
)

target_include_directories(hu_zoidcom_compat PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(hu_zoidcom_compat PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ENet, via vcpkg's unofficial-enet CMake config package (the port ships no
# `enet` config package, only `unofficial-enet`; confirmed by inspecting
# C:\Users\dirk\vcpkg\installed\x86-windows\share\{enet,unofficial-enet}).
find_package(unofficial-enet CONFIG REQUIRED)

target_link_libraries(hu_zoidcom_compat PRIVATE
    unofficial::enet::enet
)

if(WIN32)
    target_link_libraries(hu_zoidcom_compat PRIVATE ws2_32 winmm)
endif()

target_compile_definitions(hu_zoidcom_compat PRIVATE
    WIN32
    _LIB
    _CRT_SECURE_NO_WARNINGS
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)
