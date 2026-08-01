# hu_skyx_compat -- drop-in SkyX 0.1 API shim (sky rendering inert, see
# docs/porting/skyx-compat.md). Owned by this porting effort
# (compat/skyx/**), not a port of anything under HovercraftUniverse/.
# Needs OgreMain (Ogre::Camera/SceneManager/Vector2/Vector3 in the public
# API surface InGameState.cpp touches).

add_library(hu_skyx_compat STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SkyXAll.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/skyx_compat/SkyXAll.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SkyX.h
)

target_include_directories(hu_skyx_compat PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(hu_skyx_compat PUBLIC OgreMain)
