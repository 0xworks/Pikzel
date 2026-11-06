cmake_minimum_required(VERSION 3.28)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
   message(FATAL_ERROR "In-source builds are not allowed. Configure into a separate directory, for example with a preset: cmake --preset windows-x64-debug")
endif()

project(
   Pikzel
   VERSION 0.1.0
   DESCRIPTION "Pikzel game engine"
   LANGUAGES C CXX
)

# Only the top-level project decides the language standard and output layout.
# If Pikzel is ever consumed as a subproject, the parent stays in control.
if(PROJECT_IS_TOP_LEVEL)
   set(CMAKE_CXX_STANDARD 23)
   set(CMAKE_CXX_STANDARD_REQUIRED ON)
   set(CMAKE_CXX_EXTENSIONS OFF)

   # Put every executable (and any DLLs) in one directory so that examples and tools
   # can be run directly from the build tree without copying files around.
   set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
   set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
   set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

   set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

option(PKZL_BUILD_EXAMPLES "Build the example programs" ${PROJECT_IS_TOP_LEVEL})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CompilerOptions)
include(Dependencies)

add_subdirectory(src/Pikzel)

if(PKZL_BUILD_EXAMPLES)
   add_subdirectory(examples)
endif()
