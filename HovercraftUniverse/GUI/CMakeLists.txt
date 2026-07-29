# GUI -- static library
#
# Original: GUI.vcproj, ConfigurationType="4" (static lib), CharacterSet="1"
# (MBCS). AdditionalIncludeDirectories identical in Debug and Release:
# dependencies\Hikari\includes, dependencies\OGRE\include\OIS,
# dependencies\OGRE\include\OGRE, ..\Exceptions, dependencies\boost\include.
# Debug VCLibrarianTool explicitly set OutputFile="$(OutDir)\GUI_d.lib"
# (handled generically by CMAKE_DEBUG_POSTFIX at the root).
#
# Test.cpp exists on disk in this directory but is NOT referenced anywhere
# in GUI.vcproj -- it was excluded from the original build and is
# deliberately omitted here too, for fidelity.

add_library(GUI STATIC
    GUIManager.cpp
    MouseVisualisation.cpp
    OverlayContainer.cpp
    Resolution.cpp
    ScalingResolution.cpp
    BasicOverlay.cpp
    OverlayManager.cpp
    Chat.cpp
    ConnectWindow.cpp
    Countdown.cpp
    Direction.cpp
    LoadingOverlay.cpp
    LobbyOverlay.cpp
    MenuButton.cpp
    NMessageBox.cpp
    Position.cpp
    Results.cpp
    ServerMenu.cpp
    Speedometer.cpp
    TimeBehind.cpp
    Timer.cpp
    # Test.cpp intentionally excluded -- not part of GUI.vcproj's file list
    GUIManager.h
    MouseVisualisation.h
    OverlayContainer.h
    IResolution.h
    Resolution.h
    ScalingResolution.h
    BasicOverlay.h
    OverlayManager.h
    OverlayParameters.h
    Chat.h
    ConnectListener.h
    ConnectWindow.h
    Countdown.h
    Direction.h
    LoadingOverlay.h
    LobbyOverlay.h
    MenuButton.h
    NMessagebox.h
    Position.h
    Results.h
    ServerMenu.h
    ServerMenuListener.h
    Speedometer.h
    TimeBehind.h
    Timer.h
)

target_include_directories(GUI PRIVATE
    ${HU_DEPS}/Hikari/includes
    ${HU_DEPS}/OGRE/include/OIS
    ${HU_DEPS}/OGRE/include/OGRE
    ${CMAKE_SOURCE_DIR}/Exceptions
    ${HU_DEPS}/boost/include
)

target_compile_definitions(GUI PRIVATE
    WIN32
    _CONSOLE
    _MBCS
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)
