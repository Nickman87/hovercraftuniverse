# Sound -- static library
#
# Original: Sound.vcproj, ConfigurationType="4" (static lib),
# CharacterSet="1" (MBCS). AdditionalIncludeDirectories identical in Debug
# and Release: dependencies\FMOD\includes, dependencies\OGRE\include\OGRE.
#
# Oddity preserved for the record (not replicated): Sound.vcproj's
# VCLibrarianTool additionally specified AdditionalDependencies="fmodex_vc.lib
# fmod_event.lib OgreMain[_d].lib" and matching AdditionalLibraryDirectories.
# For a static-library project this is VS2008's "merge additional libraries
# into the output archive" feature (lib.exe /OUT with multiple inputs), i.e.
# the original Sound_d.lib/Sound.lib were "fat" archives that also embedded
# copies of the FMOD and OgreMain objects. This has no effect on final
# link-ability of HovercraftUniverse.exe, because fmod_event.lib,
# fmodex_vc.lib and OgreMain[_d].lib are ALSO linked directly by the exe
# project (see HovercraftUniverse/CMakeLists.txt) -- so omitting the
# archive-merge here changes nothing observable about the build outcome,
# while avoiding a non-portable custom lib.exe /OUT build step that
# wouldn't work outside the MSVC toolset ("or opening in any IDE").
#
# ReadMe.txt is referenced by Sound.vcproj but does not exist anywhere in
# this Sound/ directory on disk -- a stale reference in the original
# project. Intentionally omitted here.

add_library(Sound STATIC
    Moveable3DEmitter.cpp
    SoundManager.cpp
    Moveable3DEmitter.h
    SoundManager.h
)

target_include_directories(Sound PRIVATE
    ${HU_DEPS}/FMOD/includes
    ${HU_DEPS}/OGRE/include/OGRE
)

target_compile_definitions(Sound PRIVATE
    WIN32
    _LIB
    _MBCS
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)
