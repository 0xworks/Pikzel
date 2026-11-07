# 3.30 is the minimum required by NVIDIA NRI (see cmake/Dependencies.cmake).
cmake_minimum_required(VERSION 3.30)

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

   # The engine does not use C++20 modules, so skip the dependency-scanning step CMake
   # would otherwise add to every C++ compile (third-party targets included).
   set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

   # Put every executable (and any DLLs) in one directory so that examples and tools
   # can be run directly from the build tree without copying files around.
   set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
   set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
   set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

   set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

option(PKZL_BUILD_EXAMPLES "Build the example programs" ${PROJECT_IS_TOP_LEVEL})
option(PKZL_BUILD_TESTS "Build the unit tests" ${PROJECT_IS_TOP_LEVEL})

# Vulkan support is always built. D3D12 support is optional, and Windows only. With it
# enabled the engine contains both backends, every shader is compiled to both SPIR-V and
# DXIL, and programs choose the graphics API at run time (GraphicsDeviceSettings::api).
# It is off by default because it adds a download (the DirectX Shader Compiler) and
# roughly doubles shader build time.
option(PKZL_ENABLE_D3D12 "Build the D3D12 rendering backend in addition to Vulkan (Windows only)" OFF)
if(PKZL_ENABLE_D3D12 AND NOT WIN32)
   message(FATAL_ERROR "PKZL_ENABLE_D3D12 is only supported on Windows")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CompilerOptions)
include(Dependencies)
include(Shaders)

add_subdirectory(src/Pikzel)

if(PKZL_BUILD_EXAMPLES)
   add_subdirectory(examples)
endif()

if(PKZL_BUILD_TESTS)
   enable_testing()
   add_subdirectory(tests)
endif()
