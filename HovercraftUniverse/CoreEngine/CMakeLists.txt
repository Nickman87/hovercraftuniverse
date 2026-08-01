# CoreEngine -- static library
#
# Original: CoreEngine.vcproj, ConfigurationType="4" (static lib),
# CharacterSet="2" (Unicode).
#
# Files on disk in this directory but NOT referenced anywhere in
# CoreEngine.vcproj (and therefore intentionally omitted below, for
# fidelity):
#   - MovableTextOverlay.cpp / MovableTextOverlay.h  (orphaned, unused)
#   - RectLayoutManager.h                            (orphaned, unused)
#   - ControllerActionType.h                          (only ever referenced
#     from ..\HovercraftUniverse\HovercraftUniverse.vcproj's own Header
#     Files list, as a convenience browsing entry -- CoreEngine.vcproj
#     itself never lists it; harmless either way since it's a header)

add_library(CoreEngine STATIC
    CameraSpring.cpp
    FreeroamCameraController.cpp
    RaceCamera.cpp
    BasicGameState.cpp
    GameStateManager.cpp
    BasicEntityEvent.cpp
    ControllerEvent.cpp
    ControllerEventParser.cpp
    InputManager.cpp
    KeyManager.cpp
    Entity.cpp
    EntityManager.cpp
    EntityMapping.cpp
    EntityPropertySystem.cpp
    EntityRepresentation.cpp
    ObjectTextDisplay.cpp
    RepresentationManager.cpp
    Application.cpp
    DedicatedServer.cpp
    GameView.cpp
    OgreWindowListener.cpp
    PathReader.cpp
    BasicView.cpp
    CameraSpring.h
    FreeroamCameraController.h
    RaceCamera.h
    BasicGameState.h
    GameStateManager.h
    BasicEntityEvent.h
    CameraControllerActionType.h
    Controller.h
    ControllerEvent.h
    ControllerEventParser.h
    ControllerEventType.h
    InputManager.h
    KeyManager.h
    Entity.h
    EntityManager.h
    EntityMapping.h
    EntityPropertySystem.h
    EntityRepresentation.h
    EntityResource.h
    ObjectTextDisplay.h
    RepresentationManager.h
    Application.h
    DedicatedServer.h
    GameView.h
    OgreWindowListener.h
    PathReader.h
    BasicView.h
    MenuView.h
)

# Common to both configs (from AdditionalIncludeDirectories).
target_include_directories(CoreEngine PRIVATE
    ${CMAKE_SOURCE_DIR}/OgreMax
    ${HU_DEPS}/Havoc/Source
    ${HU_DEPS}/OGRE/include/OIS
    ${HU_DEPS}/OGRE/include/OGRE
    ${HU_DEPS}/OGRE/include
    ${CMAKE_SOURCE_DIR}/GUI
    ${HU_DEPS}/Hikari/includes
    ${HU_DEPS}/FMOD/includes
    ${CMAKE_SOURCE_DIR}/Sound
    ${CMAKE_SOURCE_DIR}/Networking
    ${HU_DEPS}/zoidcom/include
    ${CMAKE_SOURCE_DIR}/data/sound
    ${CMAKE_SOURCE_DIR}/Exceptions
    ${CMAKE_SOURCE_DIR}/Scripting
    ${CMAKE_SOURCE_DIR}/Utils
    ${HU_DEPS}/lua/include
    ${HU_DEPS}/luabind/include
    ${HU_DEPS}/boost/include
)

# Asymmetry preserved for fidelity: the original vcproj's Debug
# AdditionalIncludeDirectories included dependencies\SkyX\include, but the
# Release AdditionalIncludeDirectories did NOT. This is very likely an
# oversight in the original project (nothing else in CoreEngine's sources
# suggests SkyX should only be visible in Debug), but per the "faithful
# port" requirement it is preserved exactly rather than silently unified.
target_include_directories(CoreEngine PRIVATE $<$<CONFIG:Debug>:${HU_DEPS}/SkyX/include>)

# Asymmetry preserved for fidelity: the original vcproj's Release
# AdditionalIncludeDirectories additionally listed
# dependencies\CCS\include, which does not exist anywhere in this
# repository or its dependency tree (no "CCS" third-party package is used
# by anything else in the solution). Kept as a Release-only include path
# for fidelity; it is inert since the directory is absent.
target_include_directories(CoreEngine PRIVATE $<$<CONFIG:Release>:${HU_DEPS}/CCS/include>)

target_compile_definitions(CoreEngine PRIVATE
    WIN32
    _LIB
    UNICODE
    _UNICODE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)
