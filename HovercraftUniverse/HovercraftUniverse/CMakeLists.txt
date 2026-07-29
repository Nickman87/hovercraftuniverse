# HovercraftUniverse -- WIN32 executable (WinMain, SubSystem="2")
#
# Original: HovercraftUniverse.vcproj, ConfigurationType="1" (application),
# CharacterSet="2" (Unicode), SubSystem="2" (Windows subsystem, i.e. a
# WinMain-based GUI app, not a console app).
#
# Files on disk in this directory but NOT referenced anywhere in
# HovercraftUniverse.vcproj (intentionally omitted below, for fidelity):
#   - AdvancedCollisionPrevention.cpp / .h  (orphaned, unused)
#   - EntityEvent.h                         (empty/0-byte placeholder file)
#   - PortalData.cpp                        (orphaned .cpp with no matching
#                                             header, never added to the
#                                             project)
#   - ServerWorldLoader.cpp / .h            (orphaned, unused)
#
# ..\data\user\game.userdatatypes.xml and ..\data\user\types.userdatatypes.xml
# are listed as loose (non-compiled) files in the original project, for
# Solution Explorer browsing only. They carry no build meaning and are not
# referenced here.

add_executable(HovercraftUniverse WIN32
    HUApplication.cpp
    HUDedicatedServer.cpp
    PathRecorder.cpp
    ClientConnectThread.cpp
    EntityRegister.cpp
    HUClient.cpp
    HUServer.cpp
    HUServerCore.cpp
    HUServerThread.cpp
    AbstractHavokWorld.cpp
    Havok.cpp
    HavokThread.cpp
    HoverCraftUniverseWorld.cpp
    CharacterContextContainer.cpp
    HavokEntity.cpp
    HavokEntityType.cpp
    HavokHovercraft.cpp
    StaticBodyEntity.cpp
    BasePhantom.cpp
    CheckpointPhantom.cpp
    FinishPhantom.cpp
    PlanetGravityPhantom.cpp
    PortalPhantom.cpp
    PowerupPhantom.cpp
    SpeedBoostPhantom.cpp
    StartPhantom.cpp
    EntityCollisionPrevention.cpp
    BoostAction.cpp
    HavokHovercraftCollisionEffect.cpp
    HoverAction.cpp
    PhantomTrackAction.cpp
    PlanetGravityAction.cpp
    UpdatePositionAction.cpp
    Console.cpp
    Asteroid.cpp
    CheckPoint.cpp
    Finish.cpp
    Hovercraft.cpp
    HovercraftUniverseEntity.cpp
    Portal.cpp
    PowerupSpawn.cpp
    ResetSpawn.cpp
    SpeedBoost.cpp
    Start.cpp
    StartPosition.cpp
    StaticBody.cpp
    Track.cpp
    AsteroidRepresentation.cpp
    BoostRepresentation.cpp
    CheckPointRepresentation.cpp
    FinishRepresentation.cpp
    HovercraftRepresentation.cpp
    PortalRepresentation.cpp
    StartRepresentation.cpp
    StaticBodyRepresentation.cpp
    CustomOgreMaxScene.cpp
    ClientLoader.cpp
    ClientPreparationLoader.cpp
    HovercraftLoader.cpp
    Loader.cpp
    ProgressMonitor.cpp
    ServerLoader.cpp
    TrackInfoLoader.cpp
    HovercraftAIController.cpp
    HovercraftController.cpp
    HovercraftPlayerController.cpp
    Lobby.cpp
    PlayerSettings.cpp
    PlayerSettingsInterceptor.cpp
    InGameState.cpp
    LobbyState.cpp
    MainMenuState.cpp
    ControlsReader.cpp
    HUD.cpp
    LobbyGUI.cpp
    MainMenu.cpp
    RacePlayer.cpp
    RaceState.cpp
    BoostProperty.cpp
    CheckpointEvent.cpp
    GameEvent.cpp
    GameEventParser.cpp
    InitEvent.cpp
    StateEvent.cpp
    CollisionEvent.cpp
    VisualEvent.cpp
    VisualEventParser.cpp
    main.cpp
    HUApplication.h
    HUDedicatedServer.h
    PathRecorder.h
    ClientConnectThread.h
    EntityRegister.h
    HUClient.h
    HUServer.h
    HUServerCore.h
    HUServerThread.h
    AbstractHavokWorld.h
    Havok.h
    HavokThread.h
    HoverCraftUniverseWorld.h
    CharacterContextContainer.h
    HavokEntity.h
    HavokEntityType.h
    HavokHovercraft.h
    StaticBodyEntity.h
    BasePhantom.h
    CheckpointPhantom.h
    FinishPhantom.h
    PlanetGravityPhantom.h
    PortalPhantom.h
    PowerupPhantom.h
    SpeedBoostPhantom.h
    StartPhantom.h
    EntityCollisionPrevention.h
    BoostAction.h
    HavokHovercraftCollisionEffect.h
    HoverAction.h
    PhantomTrackAction.h
    PlanetGravityAction.h
    UpdatePositionAction.h
    Console.h
    Asteroid.h
    CheckPoint.h
    Finish.h
    GameEntities.h
    GameEntityDeclarations.h
    Hovercraft.h
    HovercraftUniverseEntity.h
    Portal.h
    PowerupSpawn.h
    ResetSpawn.h
    SpeedBoost.h
    Start.h
    StartPosition.h
    StaticBody.h
    Track.h
    AsteroidRepresentation.h
    BoostRepresentation.h
    CheckPointRepresentation.h
    FinishRepresentation.h
    HovercraftRepresentation.h
    PortalRepresentation.h
    StartRepresentation.h
    StaticBodyRepresentation.h
    CustomOgreMaxScene.h
    CustomOgreMaxSceneCallback.h
    OgreMaxTypesExtension.h
    ClientLoader.h
    ClientPreparationLoader.h
    HovercraftLoader.h
    Loader.h
    ProgressMonitor.h
    ProgressMonitorListener.h
    ServerLoader.h
    TrackInfoLoader.h
    HovercraftAIController.h
    HovercraftController.h
    HovercraftPlayerController.h
    Lobby.h
    LobbyListener.h
    PlayerSettings.h
    PlayerSettingsInterceptor.h
    PlayerSettingsListener.h
    InGameState.h
    LobbyState.h
    MainMenuState.h
    ../CoreEngine/ControllerActionType.h
    ControlsReader.h
    HUD.h
    LobbyGUI.h
    MainMenu.h
    RacePlayer.h
    RaceState.h
    RaceStateListener.h
    BoostProperty.h
    CheckpointEvent.h
    GameEvent.h
    GameEventParser.h
    GameEventType.h
    InitEvent.h
    StateEvent.h
    CollisionEvent.h
    VisualEvent.h
    VisualEventParser.h
    VisualEventType.h
    HovercraftUniverse.rc
    HovercraftUniverseIcon.ico
    HovercraftUniverseIconSmall.ico
    HovercraftUniverseVista.ico
)

# The exe is never suffixed with "_d" in Debug in the original project (only
# the static libs were), so clear the postfix inherited from the root
# CMAKE_DEBUG_POSTFIX="_d" setting.
set_target_properties(HovercraftUniverse PROPERTIES DEBUG_POSTFIX "")

# AdditionalIncludeDirectories -- identical between Debug and Release aside
# from "..\data\sound" (Debug) vs "..\Data\sound" (Release), which are the
# same directory on a case-insensitive Windows filesystem.
target_include_directories(HovercraftUniverse PRIVATE
    ${CMAKE_SOURCE_DIR}/OgreMax
    ${HU_DEPS}/Havoc/Source
    ${HU_DEPS}/SkyX/include
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
    ${HU_DEPS}/lua/include
    ${HU_DEPS}/luabind/include
    ${HU_DEPS}/boost/include
    ${CMAKE_SOURCE_DIR}/CoreEngine
    ${CMAKE_SOURCE_DIR}/Utils
)

target_compile_definitions(HovercraftUniverse PRIVATE
    WIN32
    _WINDOWS
    UNICODE
    _UNICODE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

# ----------------------------------------------------------------------------
# Linking
# ----------------------------------------------------------------------------
# HovercraftUniverse.vcproj's own VCLinkerTool AdditionalDependencies list
# (in both Debug and Release) never explicitly names CoreEngine[_d].lib or
# Networking[_d].lib, even though HovercraftUniverse.sln declares CoreEngine
# and Networking as solution-level project dependencies of the exe. This is
# NOT an omission/bug: VS2008 project-to-project references default to
# "Link Library Dependencies" = true, which causes the IDE/msbuild(vcbuild)
# to automatically append the static-library output of every referenced
# project to the link line, on top of whatever is explicitly listed in
# AdditionalDependencies. So in the original 2008 build, CoreEngine[_d].lib
# and Networking[_d].lib WERE linked in -- implicitly, via that solution
# dependency mechanism, rather than being spelled out in the vcproj text.
#
# CMake has no equivalent "implicit link via build-order dependency" concept
# -- target_link_libraries() is how you express "link against this", and it
# is also what makes CMake pick the right per-config artifact (CoreEngine_d
# vs CoreEngine, etc.) automatically. So all eight internal static libraries
# (the six that WERE explicit in the original AdditionalDependencies, plus
# CoreEngine and Networking which were linked implicitly by VS2008) are
# linked here as CMake targets, faithfully reproducing what actually ended
# up on the original link line.
target_link_libraries(HovercraftUniverse PRIVATE
    CoreEngine
    Networking
)

target_link_directories(HovercraftUniverse PRIVATE
    $<$<CONFIG:Debug>:${CMAKE_SOURCE_DIR}/lib/Debug>
    $<$<CONFIG:Debug>:${HU_DEPS}/Havoc/Lib/win32_net_9-0/debug_multithreaded_dll>
    $<$<CONFIG:Debug>:${HU_DEPS}/SkyX/lib>
    $<$<CONFIG:Debug>:${HU_DEPS}/OGRE/lib/Debug>
    $<$<CONFIG:Debug>:${HU_DEPS}/Hikari/lib>
    $<$<CONFIG:Debug>:${HU_DEPS}/FMOD/lib>
    $<$<CONFIG:Debug>:${HU_DEPS}/zoidcom/lib/debug>
    $<$<CONFIG:Debug>:${HU_DEPS}/luabind/msvc-9.0-sp1/lib-x86>
    $<$<CONFIG:Debug>:${HU_DEPS}/boost/lib/debug>
    $<$<CONFIG:Debug>:${HU_DEPS}/lua/msvc-9.0-sp1/lib-x86>

    $<$<CONFIG:Release>:${CMAKE_SOURCE_DIR}/lib/Release>
    $<$<CONFIG:Release>:${HU_DEPS}/Havoc/Lib/win32_net_9-0/release_multithreaded_dll>
    $<$<CONFIG:Release>:${HU_DEPS}/SkyX/lib>
    $<$<CONFIG:Release>:${HU_DEPS}/OGRE/lib/release>
    $<$<CONFIG:Release>:${HU_DEPS}/Hikari/lib>
    $<$<CONFIG:Release>:${HU_DEPS}/FMOD/lib>
    $<$<CONFIG:Release>:${HU_DEPS}/boost/lib/release>
    $<$<CONFIG:Release>:${HU_DEPS}/zoidcom/lib/release>
    $<$<CONFIG:Release>:${HU_DEPS}/luabind/msvc-9.0-sp1/lib-x86>
    $<$<CONFIG:Release>:${HU_DEPS}/lua/msvc-9.0-sp1/lib-x86>
)

# The remaining six internal static-lib targets -- these WERE explicit in
# the original AdditionalDependencies (OgreMax[_d].lib, GUI[_d].lib,
# Sound[_d].lib, Exceptions[_d].lib, Scripting[_d].lib, Utils[_d].lib).
# Combined with the CoreEngine/Networking target_link_libraries() call
# above (linked implicitly by VS2008, see note above), all eight internal
# libraries are now linked.
target_link_libraries(HovercraftUniverse PRIVATE
    OgreMax
    GUI
    Sound
    Exceptions
    Scripting
    Utils
)

# Prebuilt third-party import libs, referenced by raw file name exactly as
# in the original AdditionalDependencies strings (resolved via the
# target_link_directories search paths above). Names that differ between
# Debug/Release (the "_d" family, plus OgreMain/OIS/Hikari/zoidcom/lua/
# luabind which do not follow the generic "_d" suffix convention) are
# selected with generator expressions.
target_link_libraries(HovercraftUniverse PRIVATE
    $<$<CONFIG:Debug>:OgreMain_d.lib>
    $<$<CONFIG:Debug>:OIS_d.lib>
    $<$<CONFIG:Debug>:Hikari_d.lib>
    $<$<CONFIG:Debug>:zoidcom_vcd.lib>
    $<$<CONFIG:Debug>:lua.debug.lib>
    $<$<CONFIG:Debug>:luabind.debug.lib>
    $<$<CONFIG:Debug>:SkyX_d.lib>

    $<$<CONFIG:Release>:OgreMain.lib>
    $<$<CONFIG:Release>:OIS.lib>
    $<$<CONFIG:Release>:Hikari.lib>
    $<$<CONFIG:Release>:zoidcom_vc.lib>
    $<$<CONFIG:Release>:lua.release.lib>
    $<$<CONFIG:Release>:luabind.release.lib>
    $<$<CONFIG:Release>:SkyX.lib>

    # Havok/Havoc set -- identical library names in both configurations,
    # only the search directory (debug_multithreaded_dll vs
    # release_multithreaded_dll, see target_link_directories above) differs.
    hkaAnimation.lib
    hkaInternal.lib
    hkaRagdoll.lib
    hkBase.lib
    hkCompat.lib
    hkGeometryUtilities.lib
    hkInternal.lib
    hkpCollide.lib
    hkpConstraintSolver.lib
    hkpDynamics.lib
    hkpInternal.lib
    hkpUtilities.lib
    hkpVehicle.lib
    hkSceneData.lib
    hksCommon.lib
    hkSerialize.lib
    hksXAudio2.lib
    hkVisualize.lib

    # FMOD -- identical library names in both configurations.
    fmod_event.lib
    fmodex_vc.lib
)
