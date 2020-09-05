cmake_minimum_required(VERSION 3.10)

include("vendor/cmake-precompiled-header/PrecompiledHeader.cmake")
include("CMakeUtils.cmake")

project ("Pikzel" VERSION 0.1 DESCRIPTION "Pikzel Interactive App Engine")

add_subdirectory("Pikzel")
add_subdirectory("001 - Triangle")
add_subdirectory("002 - Textured")
