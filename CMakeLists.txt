cmake_minimum_required(VERSION 3.10)

include("vendor/cmake-precompiled-header/PrecompiledHeader.cmake")
include("CMakeMacros.txt")

project ("Pikzel" VERSION 0.1 DESCRIPTION "Pikzel Interactive App Engine")

add_subdirectory("Pikzel")
add_subdirectory("001 - Do Nothing")
add_subdirectory("002 - Offline Triangle")
add_subdirectory("Pikzelated")
