# Exceptions -- static library
#
# Original: Exceptions.vcproj, ConfigurationType="4" (static lib),
# CharacterSet="1" (MBCS). No AdditionalIncludeDirectories were specified in
# either configuration -- only the project's own directory (implicit).
# UsePrecompiledHeader="0" in both configs (no PCH anywhere in this
# solution, mirrored by simply not configuring one here).

add_library(Exceptions STATIC
    Exception.cpp
    Exception.h
)

target_compile_definitions(Exceptions PRIVATE
    WIN32
    _LIB
    _MBCS
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)
