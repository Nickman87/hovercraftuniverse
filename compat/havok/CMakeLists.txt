# hu_havok_compat -- drop-in Havok Physics 6.6 API shim backed by Bullet
# 3.25. Owned by this porting effort (compat/havok/**), not a port of
# anything under HovercraftUniverse/. See docs/porting/havok-compat.md.

find_package(Bullet CONFIG REQUIRED)

add_library(hu_havok_compat STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HavokAll.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/havok_compat/HavokAll.h
    # Phase B collision-reconstruction provider hook -- see
    # docs/porting/phase-b-collision.md section 3.1. Ogre-free by design;
    # implemented by the new hu_collision module (modern/CMakeLists.txt).
    ${CMAKE_CURRENT_SOURCE_DIR}/include/havok_compat/CollisionProvider.h
)

target_include_directories(hu_havok_compat PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${BULLET_INCLUDE_DIRS}
)

target_compile_definitions(hu_havok_compat PRIVATE
    _CRT_SECURE_NO_WARNINGS
)

# BULLET_LIBRARIES from BulletConfig.cmake is a list of plain target/library
# names (LinearMath;Bullet3Common;BulletInverseDynamics;BulletCollision;
# BulletDynamics;BulletSoftBody), not fully-qualified imported targets --
# resolved via the library search path CMake sets up from find_package.
target_link_libraries(hu_havok_compat PUBLIC ${BULLET_LIBRARIES})
