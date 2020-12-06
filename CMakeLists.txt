cmake_minimum_required(VERSION 3.16)

include("CMakeUtils.cmake")

project ("Pikzel" VERSION 0.1 DESCRIPTION "Pikzel Interactive App Engine")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory("Pikzel")
add_subdirectory("Data")
add_subdirectory("001 - Triangle")
add_subdirectory("002 - Textured")
add_subdirectory("003 - Cube")
add_subdirectory("004 - Lighting")
add_subdirectory("005 - Materials")
add_subdirectory("006 - Lighting Maps")
add_subdirectory("007 - Light Casters")
add_subdirectory("008 - Multiple Lights")
add_subdirectory("009 - Model and Mesh")
add_subdirectory("010 - Framebuffers")
add_subdirectory("011 - Skybox")
add_subdirectory("012 - sRGB")
add_subdirectory("013 - Shadows")
