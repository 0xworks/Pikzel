# Third-party dependencies.
#
# Dependencies are fetched from source with FetchContent and built as part of the project.
# This keeps the build self-contained: a fresh clone needs only CMake and a C++23 compiler,
# with no package manager to install or configure. The trade-off is a slower first
# configure, and every dependency must be pinned to an exact release tag here.
#
# Add new dependencies sparingly, and only with a clear justification (see AGENTS.md).

include(FetchContent)

# spdlog: logging.
#
# spdlog is configured to format with std::format instead of its bundled copy of fmt, so
# the engine uses a single formatting library. A custom type then needs only one
# std::formatter specialisation to work with both logging and std::format elsewhere.
# The cost is fmt-only extras such as named arguments, fmt::join and spdlog's bin_to_hex.
# spdlog exports the resulting SPDLOG_USE_STD_FORMAT definition as a PUBLIC compile
# definition, so everything linking spdlog agrees on the setting.
set(SPDLOG_USE_STD_FORMAT ON)

FetchContent_Declare(
   spdlog
   GIT_REPOSITORY https://github.com/gabime/spdlog.git
   GIT_TAG v1.15.3
   GIT_SHALLOW TRUE
   SYSTEM
)

# GLFW: windowing and input.
#
# Only the library is needed; its docs, tests and examples are switched off.
set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_INSTALL OFF)

FetchContent_Declare(
   glfw
   GIT_REPOSITORY https://github.com/glfw/glfw.git
   GIT_TAG 3.4
   GIT_SHALLOW TRUE
   SYSTEM
)

# NVIDIA NRI: graphics API abstraction.
#
# NRI is built as a static library with only the Vulkan backend enabled. Vulkan runs on
# every desktop platform the engine targets, and Slang compiles shaders to SPIR-V with no
# extra tools. Enabling D3D12 later is a matter of switching its backend on here and also
# compiling shaders to DXIL (which needs dxcompiler.dll alongside slangc).
#
# NRI fetches the Vulkan headers itself, so the Vulkan SDK is not needed to build. At run
# time the Vulkan loader (vulkan-1.dll / libvulkan.so) comes with the GPU driver; the SDK
# is only needed for the Vulkan validation layers.
#
# These are cache entries, rather than plain variables, because NRI declares most of them
# with cmake_dependent_option, which reads the cache.
set(NRI_STATIC_LIBRARY ON CACHE BOOL "")
set(NRI_ENABLE_VK_SUPPORT ON CACHE BOOL "")
set(NRI_ENABLE_D3D11_SUPPORT OFF CACHE BOOL "")
set(NRI_ENABLE_D3D12_SUPPORT OFF CACHE BOOL "")
set(NRI_ENABLE_WGPU_SUPPORT OFF CACHE BOOL "")
set(NRI_ENABLE_METAL_SUPPORT OFF CACHE BOOL "")
set(NRI_ENABLE_NONE_SUPPORT OFF CACHE BOOL "")
set(NRI_ENABLE_NVTX_SUPPORT OFF CACHE BOOL "")
set(NRI_ENABLE_IMGUI_EXTENSION OFF CACHE BOOL "")

FetchContent_Declare(
   nri
   GIT_REPOSITORY https://github.com/NVIDIA-RTX/NRI.git
   GIT_TAG v191
   GIT_SHALLOW TRUE
   SYSTEM
)

FetchContent_MakeAvailable(spdlog glfw nri)

# Slang: shader compiler.
#
# Shaders are compiled at build time by the slangc command-line compiler (see Shaders.cmake),
# so the engine does not need to link the Slang library yet. Building Slang from source
# takes a long time, so the official prebuilt release is downloaded instead. When the
# engine starts using Slang's reflection API, the same package also provides the library.
set(PKZL_SLANG_VERSION 2026.20)

if(CMAKE_HOST_WIN32)
   set(PKZL_SLANG_PACKAGE "slang-${PKZL_SLANG_VERSION}-windows-x86_64.zip")
   set(PKZL_SLANG_HASH e6dd32bbddf1720ebb4e679d55e9635ce792865a5bd84f3d2cd9af28647c0174)
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
   set(PKZL_SLANG_PACKAGE "slang-${PKZL_SLANG_VERSION}-linux-x86_64.tar.gz")
   set(PKZL_SLANG_HASH e863e568c204810d6e8bc31852f056aef6763c55ea9925db2db7affa060347c9)
else()
   message(FATAL_ERROR "No prebuilt Slang package is configured for ${CMAKE_HOST_SYSTEM_NAME}")
endif()

FetchContent_Declare(
   slang
   URL "https://github.com/shader-slang/slang/releases/download/v${PKZL_SLANG_VERSION}/${PKZL_SLANG_PACKAGE}"
   URL_HASH SHA256=${PKZL_SLANG_HASH}
   DOWNLOAD_EXTRACT_TIMESTAMP TRUE
   SOURCE_SUBDIR "no-cmake-project" # download and extract only; there is nothing to build
)
FetchContent_MakeAvailable(slang)

find_program(
   PKZL_SLANGC_EXECUTABLE
   NAMES slangc
   PATHS "${slang_SOURCE_DIR}/bin"
   NO_DEFAULT_PATH
   REQUIRED
)

# Keep third-party targets out of the way in IDE solution views.
foreach(thirdPartyTarget IN ITEMS spdlog glfw update_mappings NRI NRI_Shared NRI_VK)
   if(TARGET ${thirdPartyTarget})
      set_target_properties(${thirdPartyTarget} PROPERTIES FOLDER "ThirdParty")
   endif()
endforeach()
