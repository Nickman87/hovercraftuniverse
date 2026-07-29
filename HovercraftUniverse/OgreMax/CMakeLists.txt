# OgreMax -- static library
#
# Original: OgreMax.vcproj, ConfigurationType="4" (static lib),
# CharacterSet="2" (Unicode). AdditionalIncludeDirectories identical in
# Debug and Release: dependencies\OGRE\include\OIS, ...\include\OGRE,
# ...\include. Debug VCLibrarianTool explicitly set
# OutputFile="$(OutDir)\OgreMax_d.lib" (handled generically by
# CMAKE_DEBUG_POSTFIX at the root).
#
# readme.txt is part of the original project (loose file, not compiled) and
# is intentionally not added as a source here.

add_library(OgreMax STATIC
    OgreMaxModel.cpp
    OgreMaxScene.cpp
    OgreMaxUtilities.cpp
    ProgressCalculator.cpp
    tinyxml/tinyxml.cpp
    tinyxml/tinyxmlerror.cpp
    tinyxml/tinyxmlparser.cpp
    Version.cpp
    OgreMaxModel.hpp
    OgreMaxPlatform.hpp
    OgreMaxRenderWindowIterator.hpp
    OgreMaxScene.hpp
    OgreMaxTypes.hpp
    OgreMaxUtilities.hpp
    ProgressCalculator.hpp
    tinyxml/tinyxml.h
    Version.hpp
)

target_include_directories(OgreMax PRIVATE
    ${HU_DEPS}/OGRE/include/OIS
    ${HU_DEPS}/OGRE/include/OGRE
    ${HU_DEPS}/OGRE/include
)

target_compile_definitions(OgreMax PRIVATE
    WIN32
    _LIB
    UNICODE
    _UNICODE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)
