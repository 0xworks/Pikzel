cmake_minimum_required(VERSION 3.20)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
   message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (e.g. called build) and run CMake from there. You may need to remove CMakeCache.txt.")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include("CMakeUtils.cmake")

project ("Pikzel" VERSION 0.1 DESCRIPTION "Pikzel Interactive App Engine")

message("config int dir is: ${CMAKE_CFG_INTDIR}")


init_output_directories()

add_subdirectory("Pikzel")
add_subdirectory("Pikzelated")
add_subdirectory("Assets")
add_subdirectory("Examples")
