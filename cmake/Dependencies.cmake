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
# NRI is built as a static library. The Vulkan backend is always enabled: Vulkan runs on
# every desktop platform the engine targets, and Slang compiles shaders to SPIR-V with no
# extra tools. On Windows the D3D12 backend can be added with PKZL_ENABLE_D3D12 (see the
# top-level CMakeLists.txt). Both backends are then linked in, and the program picks one
# when it creates its GraphicsDevice.
#
# NRI fetches the Vulkan headers itself, so the Vulkan SDK is not needed to build. At run
# time the Vulkan loader (vulkan-1.dll / libvulkan.so) comes with the GPU driver; the SDK
# is only needed for the Vulkan validation layers. D3D12 comes with Windows and its
# headers with the Windows SDK.
#
# These are cache entries, rather than plain variables, because NRI declares most of them
# with cmake_dependent_option, which reads the cache. FORCE is used for the D3D12 settings
# so that toggling PKZL_ENABLE_D3D12 in an existing build directory takes effect.
set(NRI_STATIC_LIBRARY ON CACHE BOOL "")
set(NRI_ENABLE_VK_SUPPORT ON CACHE BOOL "")
set(NRI_ENABLE_D3D11_SUPPORT OFF CACHE BOOL "")
set(NRI_ENABLE_D3D12_SUPPORT ${PKZL_ENABLE_D3D12} CACHE BOOL "" FORCE)
# The Agility SDK is Microsoft's D3D12 runtime and headers distributed as a NuGet package,
# newer than the copies in Windows and the Windows SDK. NRI's D3D12 backend is written
# against its headers and does not compile with the Windows SDK's, so its support stays on
# (NRI downloads the package itself). A program only runs on the Agility runtime if its
# executable exports D3D12SDKVersion and D3D12SDKPath; Pikzel's programs do not, so they
# use the D3D12 runtime that comes with Windows, and NRI only uses features it reports.
# NRI still copies the Agility runtime into bin/AgilitySDK, where it goes unused.
#
# The NVIDIA and AMD driver extension libraries unlock vendor-specific features the engine
# does not use, so they are left out.
set(NRI_ENABLE_AGILITY_SDK_SUPPORT ${PKZL_ENABLE_D3D12} CACHE BOOL "" FORCE)
set(NRI_ENABLE_NVAPI OFF CACHE BOOL "" FORCE)
set(NRI_ENABLE_AMDAGS OFF CACHE BOOL "" FORCE)
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

# RTM (Realtime Math): CPU mathematics.
#
# RTM is header-only. Its own CMake project only builds RTM's tests, so the package is just
# downloaded, and an interface target is defined here that adds its include directory.
FetchContent_Declare(
   rtm
   GIT_REPOSITORY https://github.com/nfrechette/rtm.git
   GIT_TAG v2.4.0
   GIT_SHALLOW TRUE
   GIT_SUBMODULES "" # RTM's submodules are only for its own tests and benchmarks
   SOURCE_SUBDIR "no-cmake-project" # download only; nothing to build
)
FetchContent_MakeAvailable(rtm)

add_library(rtm INTERFACE)
add_library(rtm::rtm ALIAS rtm)
target_include_directories(rtm SYSTEM INTERFACE "${rtm_SOURCE_DIR}/includes")

# Catch2: unit testing. Only needed when the tests are built.
if(PKZL_BUILD_TESTS)
   FetchContent_Declare(
      Catch2
      GIT_REPOSITORY https://github.com/catchorg/Catch2.git
      GIT_TAG v3.9.1
      GIT_SHALLOW TRUE
      SYSTEM
   )
   FetchContent_MakeAvailable(Catch2)
   list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")
endif()

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

# DirectX Shader Compiler (DXC): DXIL code generation for D3D12.
#
# Slang does not generate DXIL itself. It translates Slang to HLSL and hands that to DXC's
# dxcompiler.dll, which the Slang package does not include. The official prebuilt DXC
# release is downloaded and its location given to slangc (see Shaders.cmake). The package
# also contains dxil.dll, which signs the DXIL: D3D12 refuses unsigned shaders unless
# experimental features are enabled, and dxcompiler.dll loads dxil.dll from its own
# directory to do the signing.
#
# The Windows SDK also includes a copy of DXC, but its version depends on which SDK is
# installed. A pinned download keeps shader output the same on every machine.
if(PKZL_ENABLE_D3D12)
   FetchContent_Declare(
      dxc
      URL "https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.9.2609/dxc_2026_09_29.zip"
      URL_HASH SHA256=ad31b1fc8443175d204f77a611fdb3ef2ec42759bdc2f1167368de24a4a7e7f1
      DOWNLOAD_EXTRACT_TIMESTAMP TRUE
      SOURCE_SUBDIR "no-cmake-project" # download and extract only; there is nothing to build
   )
   FetchContent_MakeAvailable(dxc)

   set(PKZL_DXC_DIRECTORY "${dxc_SOURCE_DIR}/bin/x64")
   if(NOT EXISTS "${PKZL_DXC_DIRECTORY}/dxcompiler.dll" OR NOT EXISTS "${PKZL_DXC_DIRECTORY}/dxil.dll")
      message(FATAL_ERROR "dxcompiler.dll and dxil.dll were not found in ${PKZL_DXC_DIRECTORY}")
   endif()
endif()

# Keep third-party targets out of the way in IDE solution views.
foreach(thirdPartyTarget IN ITEMS spdlog glfw update_mappings NRI NRI_Shared NRI_VK NRI_D3D12 Catch2 Catch2WithMain)
   if(TARGET ${thirdPartyTarget})
      set_target_properties(${thirdPartyTarget} PROPERTIES FOLDER "ThirdParty")
   endif()
endforeach()
