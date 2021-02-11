cmake_minimum_required(VERSION 3.16)

include("CMakeUtils.cmake")

project ("Pikzel" VERSION 0.1 DESCRIPTION "Pikzel Interactive App Engine")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory("Pikzel")
add_subdirectory("Pikzelated")
add_subdirectory("Assets")
add_subdirectory("Examples")
