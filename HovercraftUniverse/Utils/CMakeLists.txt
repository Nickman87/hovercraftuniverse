# Utils -- static library
#
# Original: Utils.vcproj, ConfigurationType="4" (static lib),
# CharacterSet="1" (MBCS). AdditionalIncludeDirectories identical in Debug
# and Release: ..\Exceptions, dependencies\boost\include.
# Release VCLibrarianTool explicitly set OutputFile="$(OutDir)\Utils.lib"
# (i.e. the plain, non-suffixed name) -- this is just the CMake default for
# a non-debug config and needs no special handling here.

add_library(Utils STATIC
    Timing.cpp
    Config.cpp
    inifile.cpp
    StringUtils.cpp
    Listenable.h
    ListenableImpl.h
    PlayerMap.h
    PlayerMapImpl.h
    Timing.h
    Config.h
    inifile.h
    tstdlibs.h
    StringUtils.h
)

target_include_directories(Utils PRIVATE
    ${CMAKE_SOURCE_DIR}/Exceptions
    ${HU_DEPS}/boost/include
)

target_compile_definitions(Utils PRIVATE
    WIN32
    _LIB
    _MBCS
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)
