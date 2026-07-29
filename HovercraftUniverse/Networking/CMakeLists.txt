# Networking -- static library
#
# Original: Networking.vcproj, ConfigurationType="4" (static lib),
# CharacterSet="1" (MBCS). AdditionalIncludeDirectories identical in Debug
# and Release: dependencies\zoidcom\include, ..\Exceptions,
# dependencies\OGRE\include\OGRE, ..\Utils. Debug VCLibrarianTool explicitly
# set OutputFile="$(OutDir)\Networking_d.lib" (handled generically by
# CMAKE_DEBUG_POSTFIX at the root).

add_library(Networking STATIC
    OgreQuaternion_Replicator.cpp
    OgreVector3_Replicator.cpp
    String_Replicator.cpp
    ChatClient.cpp
    ChatEntity.cpp
    ChatEvent.cpp
    ChatEventParser.cpp
    ChatServer.cpp
    NotifyEvent.cpp
    TextEvent.cpp
    NetworkClient.cpp
    NetworkEntity.cpp
    NetworkIDManager.cpp
    NetworkServer.cpp
    OgreQuaternion_Replicator.h
    OgreVector3_Replicator.h
    String_Replicator.h
    ChatClient.h
    ChatEntity.h
    ChatEvent.h
    ChatEventParser.h
    ChatEventType.h
    ChatListener.h
    ChatServer.h
    NotifyEvent.h
    TextEvent.h
    NetworkClient.h
    NetworkEntity.h
    NetworkEvent.h
    NetworkEventImpl.h
    NetworkEventParser.h
    NetworkIDManager.h
    NetworkServer.h
)

target_include_directories(Networking PRIVATE
    ${HU_DEPS}/zoidcom/include
    ${CMAKE_SOURCE_DIR}/Exceptions
    ${HU_DEPS}/OGRE/include/OGRE
    ${CMAKE_SOURCE_DIR}/Utils
)

target_compile_definitions(Networking PRIVATE
    WIN32
    _WINDOWS
    _MBCS
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)
