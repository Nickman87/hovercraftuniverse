# Scripting -- static library
#
# Original: Scripting.vcproj, ConfigurationType="4" (static lib),
# CharacterSet="1" (MBCS). AdditionalIncludeDirectories identical in Debug
# and Release: dependencies\lua\include, dependencies\luabind\include,
# dependencies\boost\include, dependencies\OGRE\include\OGRE. (Debug's
# string in the original vcproj also contained a trailing "$(NOINHERIT)"
# token -- an MSVC sentinel meaning "do not inherit the project-default
# include path list", not an actual directory -- so it is not a path and is
# not reproduced here.)
#
# Oddity preserved for fidelity: unlike every other static-lib project in
# this solution, Scripting's Release configuration used
# PreprocessorDefinitions="WIN32;NDEBUG;_CONSOLE" instead of the expected
# "...;_LIB" (Debug does use "...;_LIB" correctly). This looks like a
# copy/paste mistake in the original project, but per the "faithful port"
# requirement it is reproduced exactly rather than silently corrected.

add_library(Scripting STATIC
    OgreLuaBindings.cpp
    ScriptWrapper.cpp
    OgreLuaBindings.h
    ScriptWrapper.h
)

target_include_directories(Scripting PRIVATE
    ${HU_DEPS}/lua/include
    ${HU_DEPS}/luabind/include
    ${HU_DEPS}/boost/include
    ${HU_DEPS}/OGRE/include/OGRE
)

target_compile_definitions(Scripting PRIVATE
    WIN32
    _MBCS
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Debug>:_LIB>
    $<$<CONFIG:Release>:NDEBUG>
    $<$<CONFIG:Release>:_CONSOLE>
)
