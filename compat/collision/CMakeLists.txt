# hu_collision -- Phase B collision geometry reconstruction. Rebuilds Bullet
# collision shapes from the .scene + .mesh render data the game already
# ships (the original Havok .hkx packfiles are unparseable -- Havok 6.6 is
# dead and closed-source). Implements compat/havok's CollisionProvider hook
# so hu_havok_compat itself stays Ogre-free. See
# docs/porting/phase-b-collision.md for the full design.
#
# Owned by this porting effort (compat/collision/**), not a port of anything
# under HovercraftUniverse/.

add_library(hu_collision STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/OgreCollisionProvider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/hu_collision/OgreCollisionProvider.h
)

target_include_directories(hu_collision PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# TinyXML: reuse the same vendored copy hu_ogremax already compiles from
# (HovercraftUniverse/OgreMax/tinyxml) by linking against hu_ogremax itself,
# rather than recompiling tinyxml.cpp/tinyxmlparser.cpp/tinyxmlerror.cpp a
# second time here -- that would risk duplicate-symbol link errors if both
# copies ever end up in the same final binary. Game code #includes it as
# <tinyxml/tinyxml.h>, so the OgreMax root itself (parent of the tinyxml/
# subdirectory) needs to be on this target's own include path too, since
# hu_ogremax's own include dirs for it are PRIVATE (not propagated).
target_include_directories(hu_collision PRIVATE
    ${HU_OGREMAX_ROOT}
)

find_package(OGRE CONFIG REQUIRED)
find_package(Bullet CONFIG REQUIRED)

target_compile_definitions(hu_collision PRIVATE
    _CRT_SECURE_NO_WARNINGS
)

target_link_libraries(hu_collision PUBLIC
    hu_havok_compat
    hu_ogremax
    OgreMain
    ${BULLET_LIBRARIES}
)

# --- hu_collision_smoketest ------------------------------------------------
# Standalone verification tool (NOT part of the shipped game/exe): drives
# hkpHavokSnapshot::load()/findRigidBodyByName() directly against real .hkx
# paths, independent of ZoidCom/RaceState/Lobby -- see
# docs/porting/phase-b-collision.md's verification section, which calls out
# step 1 ("findRigidBodyByName returns non-null for every name requested")
# as independently checkable this way, and tools/smoketest.cpp's own header
# comment for the rationale.
add_executable(hu_collision_smoketest
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/smoketest.cpp
)
target_link_libraries(hu_collision_smoketest PRIVATE
    hu_collision
    OgreMain
)
