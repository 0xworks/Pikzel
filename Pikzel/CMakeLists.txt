﻿cmake_minimum_required(VERSION 3.20)

find_package(Vulkan REQUIRED)

include("vendor/cmrc/CMakeRC.cmake")

project (
   "Pikzel"
   VERSION 0.1
   DESCRIPTION "Pikzel Engine"
)

option(
   PKZL_PROFILE "Performance profiling instrumentation is compiled in to the code" OFF
)

# note: at the moment, GLFW is the only supported windowing system
#       and so the platform/GLFW files are included here
#       Later, we might support something else (dont hold your breath)
set(
   ProjectSources
   "src/Pikzel/Components/Model.h"
   "src/Pikzel/Components/Relationship.h"
   "src/Pikzel/Components/Transform.h"
   "src/Pikzel/Core/Application.h"
   "src/Pikzel/Core/Application.cpp"
   "src/Pikzel/Core/Core.h"
   "src/Pikzel/Core/EntryPoint.h"
   "src/Pikzel/Core/FileSystem.h"
   "src/Pikzel/Core/FileSystem.cpp"
   "src/Pikzel/Core/Instrumentor.h"
   "src/Pikzel/Core/Log.h"
   "src/Pikzel/Core/Log.cpp"
   "src/Pikzel/Core/PlatformUtility.h"
   "src/Pikzel/Core/PlatformUtility.cpp"
   "src/Pikzel/Core/Utility.h"
   "src/Pikzel/Core/Window.h"
   "src/Pikzel/Events/ApplicationEvents.h"
   "src/Pikzel/Events/EventDispatcher.h"
   "src/Pikzel/Events/KeyEvents.h"
   "src/Pikzel/Events/MouseEvents.h"
   "src/Pikzel/Events/WindowEvents.h"
   "src/Pikzel/Input/Input.h"
   "src/Pikzel/Input/Input.cpp"
   "src/Pikzel/Input/KeyCodes.h"
   "src/Pikzel/Input/MouseButtons.h"
   "src/Pikzel/Platform/GLFW/GLFWWindow.cpp"
   "src/Pikzel/Renderer/Buffer.h"
   "src/Pikzel/Renderer/Buffer.cpp"
   "src/Pikzel/Renderer/ComputeContext.h"
   "src/Pikzel/Renderer/Framebuffer.h"
   "src/Pikzel/Renderer/Framebuffer.cpp"
   "src/Pikzel/Renderer/GraphicsContext.h"
   "src/Pikzel/Renderer/Pipeline.h"
   "src/Pikzel/Renderer/RenderCore.h"
   "src/Pikzel/Renderer/RenderCore.cpp"
   "src/Pikzel/Renderer/ShaderUtil.h"
   "src/Pikzel/Renderer/ShaderUtil.cpp"
   "src/Pikzel/Renderer/sRGB.h"
   "src/Pikzel/Renderer/sRGB.cpp"
   "src/Pikzel/Renderer/Texture.h"
   "src/Pikzel/Renderer/Texture.cpp"
   "src/Pikzel/Scene/AssetCache.h"
   "src/Pikzel/Scene/AssetCache.cpp"
   "src/Pikzel/Scene/Camera.h"
   "src/Pikzel/Scene/Camera.cpp"
   "src/Pikzel/Scene/Light.h"
   "src/Pikzel/Scene/Mesh.h"
   "src/Pikzel/Scene/ModelAsset.h"
   "src/Pikzel/Scene/ModelAssetLoader.h"
   "src/Pikzel/Scene/ModelAssetLoader.cpp"
   "src/Pikzel/Scene/Scene.h"
   "src/Pikzel/Scene/Scene.cpp"
   "src/Pikzel/Scene/SceneRenderer.h"
   "src/Pikzel/Scene/SceneRenderer.cpp"
   "src/Pikzel/Serialization/SceneSerializer.h"
   "src/Pikzel/Serialization/SceneSerializer.cpp"
   "src/Pikzel/Serialization/Serializer.h"
   "src/Pikzel/Serialization/Yaml.h"
   "vendor/tinyfiledialogs/tinyfiledialogs.c"
)

# These at the "public" includes
# i.e. any other project referencing Pikzel will also get these includes
# There are additional "private" includes set below
set(
   ProjectIncludes
   "src"
   "vendor/entt/src"
   "vendor/glfw/include"
   "vendor/glm"
   "vendor/imgui"
   "vendor/ImGuiAl"
   "vendor/spdlog/include"
   "vendor/tracy/public"
   "vendor/yaml-cpp/include"
   ${CMRC_INCLUDE_DIR}
   ${Vulkan_INCLUDE_DIRS}
)


# These are the "public" defines
# i.e. any other project referencing Pikzel will also get these defines
# There are additional "private" defines set below
set(
   ProjectDefines
   PKZL_VERSION="${PROJECT_VERSION}"
   PKZL_VERSION_MAJOR="${PROJECT_VERSION_MAJOR}"
   PKZL_VERSION_MINOR="${PROJECT_VERSION_MINOR}"
   PKZL_DESCRIPTION="${PROJECT_DESCRIPTION}"
   $<$<CONFIG:DEBUG>:PKZL_DEBUG>
   NOMINMAX
   GLFW_INCLUDE_NONE
   GLM_FORCE_DEPTH_ZERO_TO_ONE
   GLM_FORCE_RADIANS
   TRACY_IMPORTS
)

if(PKZL_PROFILE)
   list(
      APPEND ProjectDefines
      PKZL_PROFILE
      TRACY_ENABLE
   )
endif()

# These are the "public" libraries
# i.e. any other project referencing Pikzel will also link these libraries
# There are additional "private" libraries set below
set(
   ProjectLibs
   "spdlog"
)

add_library(
   ${PROJECT_NAME} SHARED
   ${ProjectSources}
)

target_compile_features(
   ${PROJECT_NAME} PUBLIC
   cxx_std_20
)

target_compile_definitions(
   ${PROJECT_NAME} PUBLIC
   ${ProjectDefines}
)

target_compile_definitions(
   ${PROJECT_NAME} PRIVATE
   PKZL_BUILD_DLL
   PlatformOpenGL="$<TARGET_FILE_NAME:PlatformOpenGL>"
   PlatformVulkan="$<TARGET_FILE_NAME:PlatformVulkan>"
)

target_include_directories(
   ${PROJECT_NAME} PUBLIC
   ${ProjectIncludes}
)

target_include_directories(
   ${PROJECT_NAME} PRIVATE
   "vendor/dds-ktx"
   "vendor/stb"
   "vendor/tinyfiledialogs"
)

target_precompile_headers(
   ${PROJECT_NAME} PRIVATE
   [["Pikzel/Core/Core.h"]]
   <GLFW/glfw3.h>
   <glm/glm.hpp>
   <imgui.h>
)

# no precompiled header for tinyfiledialogs
set_source_files_properties(
   "vendor/tinyfiledialogs/tinyfiledialogs.c"
   PROPERTIES
   SKIP_PRECOMPILE_HEADERS ON
)

if(MSVC)
   target_compile_options(
      ${PROJECT_NAME} PUBLIC
      "$<$<CONFIG:Release>:/GL>"
   )
endif()

target_link_libraries(
   ${PROJECT_NAME} PUBLIC
   ${ProjectLibs}
)

if(MSVC)
   target_link_libraries(
      ${PROJECT_NAME} PUBLIC
      "TracyClient"
      debug "spirv-cross-cored"
      debug "spirv-cross-glsld"
      optimized "spirv-cross-core"
      optimized "spirv-cross-glsl"
   )
endif()

if(UNIX)
target_link_libraries(
   ${PROJECT_NAME} PUBLIC
   "TracyClient"
   "spirv-cross-core"
   "spirv-cross-glsl"
)
endif()

target_link_libraries(
   ${PROJECT_NAME} PRIVATE
   "assimp"
   "glfw"
   "glm"
   "ImGui"
   "PikzelResources"
   "yaml-cpp"
)

target_link_directories(
   ${PROJECT_NAME} PUBLIC
   "$ENV{VULKAN_SDK}/lib"
)

if(MSVC)
   target_link_options(
      ${PROJECT_NAME} PRIVATE
      "$<$<CONFIG:Release>:/LTCG>"
      "$<$<CONFIG:Debug>:/ignore:4099>"
   )
endif()

if(UNIX)
   target_link_options(
      ${PROJECT_NAME} PUBLIC
      "-Wl,-rpath=$ORIGIN/Pikzel/"
   )
endif()


##################################################################

set(
   RendererShaderSources
   "src/Pikzel/Renderer/Shaders/EnvironmentIrradiance.comp"
   "src/Pikzel/Renderer/Shaders/EnvironmentPrefilter.comp"
   "src/Pikzel/Renderer/Shaders/EnvironmentSpecularBRDF.comp"
   "src/Pikzel/Renderer/Shaders/EquirectangularToCubeMap.comp"
   "src/Pikzel/Renderer/Shaders/SixFacesToCubeMap.comp"
   "src/Pikzel/Renderer/Shaders/Triangle.frag"
   "src/Pikzel/Renderer/Shaders/Triangle.vert"
)

set(
   RendererShaderHeaders
)

set(
   SceneShaderSources
)

set(
   SceneShaderHeaders
)

compile_shaders(RendererShaderSources RendererShaderHeaders "src/Pikzel/Renderer/Shaders/" "${CMAKE_CURRENT_BINARY_DIR}/Renderer" CompiledRendererShaders)
compile_shaders(SceneShaderSources SceneShaderHeaders "src/Pikzel/Scene/Shaders/" "${CMAKE_CURRENT_BINARY_DIR}/Scene/Shaders" CompiledSceneShaders)

cmrc_add_resource_library(
   PikzelResources WHENCE ${CMAKE_CURRENT_BINARY_DIR}
   ${CompiledRendererShaders}
   ${CompiledSceneShaders}
)
set_property(TARGET PikzelResources PROPERTY POSITION_INDEPENDENT_CODE ON)

##################################################################
add_library(
   "ImGui" STATIC
   "src/Pikzel/ImGui/IconsFontAwesome6.h"
   "src/Pikzel/ImGui/ImGuiEx.h"
   "src/Pikzel/ImGui/ImGuiEx.cpp"
   "vendor/imgui/imgui.cpp"
   "vendor/imgui/imgui_demo.cpp"
   "vendor/imgui/imgui_draw.cpp"
   "vendor/imgui/imgui_tables.cpp"
   "vendor/imgui/imgui_widgets.cpp"
   "vendor/imgui/misc/cpp/imgui_stdlib.cpp"
)
set_property(TARGET ImGui PROPERTY POSITION_INDEPENDENT_CODE ON)

target_compile_features(
   "ImGui" PRIVATE
   cxx_std_20
)

target_include_directories(
   "ImGui" PRIVATE
   ${ProjectIncludes}
)

##################################################################

# Platform - OpenGL
#
# GLAD Source files generated from https://glad.dav1d.de/
#   Language: C/C++
#   Specification: OpenGL
#   API: Version 4.6
#   Profile: Core
#
# The imgui_impl_opengl.h and .cpp files are the ones from vendor/imgui/backends, with:
#    * change to make viewports use sRBG framebuffer

add_library(
   "PlatformOpenGL" SHARED
   "src/Pikzel/Platform/OpenGL/OpenGLBuffer.h"
   "src/Pikzel/Platform/OpenGL/OpenGLBuffer.cpp"
   "src/Pikzel/Platform/OpenGL/OpenGLComputeContext.h"
   "src/Pikzel/Platform/OpenGL/OpenGLComputeContext.cpp"
   "src/Pikzel/Platform/OpenGL/OpenGLFramebuffer.h"
   "src/Pikzel/Platform/OpenGL/OpenGLFramebuffer.cpp"
   "src/Pikzel/Platform/OpenGL/OpenGLGraphicsContext.h"
   "src/Pikzel/Platform/OpenGL/OpenGLGraphicsContext.cpp"
   "src/Pikzel/Platform/OpenGL/OpenGLPipeline.h"
   "src/Pikzel/Platform/OpenGL/OpenGLPipeline.cpp"
   "src/Pikzel/Platform/OpenGL/OpenGLRenderCore.h"
   "src/Pikzel/Platform/OpenGL/OpenGLRenderCore.cpp"
   "src/Pikzel/Platform/OpenGL/OpenGLTexture.h"
   "src/Pikzel/Platform/OpenGL/OpenGLTexture.cpp"
   "src/Pikzel/Platform/OpenGL/vendor/glad/include/glad/glad.h"
   "src/Pikzel/Platform/OpenGL/vendor/glad/include/KHR/khrplatform.h"
   "src/Pikzel/Platform/OpenGL/vendor/glad/src/glad.c"
   "src/Pikzel/Platform/OpenGL/imgui_impl_opengl3.h"
   "src/Pikzel/Platform/OpenGL/imgui_impl_opengl3.cpp"
   "vendor/imgui/backends/imgui_impl_glfw.h"
   "vendor/imgui/backends/imgui_impl_glfw.cpp"
)

target_compile_features(
   "PlatformOpenGL" PRIVATE
   cxx_std_20
)

target_compile_definitions(
   "PlatformOpenGL" PRIVATE
)

# note: needs vulkan include dirs for spirv_cross
target_include_directories(
   "PlatformOpenGL" PRIVATE
   "src/Pikzel/Platform/OpenGL/vendor/glad/include"
)

target_link_libraries(
   "PlatformOpenGL" PRIVATE
   "Pikzel"
   "glfw"
   "glm"
   "ImGui"
)

target_precompile_headers(
   "PlatformOpenGL" PRIVATE
   [["Pikzel/Core/Core.h"]]
   <glad/glad.h>
   <GLFW/glfw3.h>
   <glm/glm.hpp>
)

# no precompiled header for glad
set_source_files_properties(
   "src/Pikzel/Platform/OpenGL/vendor/glad/src/glad.c"
   PROPERTIES
   SKIP_PRECOMPILE_HEADERS ON
)

##################################################################

if(${Vulkan_FOUND})

   message("Found ${Vulkan_LIBRARY}")

   # Platform - Vulkan
   # This requires that the Vulkan SDK is installed.
   # The imgui_impl_vulkan.h and .cpp files are the ones from vendor/imgui/backends, with:
   #    * https://github.com/ocornut/imgui/pull/914 merged in
   #    * some extensions of my own to PR#914
   #    * change to make viewports use sRBG framebuffer
   add_library(
      "PlatformVulkan" SHARED
      "src/Pikzel/Platform/Vulkan/DescriptorBinding.h"
      "src/Pikzel/Platform/Vulkan/QueueFamilyIndices.h"
      "src/Pikzel/Platform/Vulkan/SwapChainSupportDetails.h"
      "src/Pikzel/Platform/Vulkan/VulkanBuffer.h"
      "src/Pikzel/Platform/Vulkan/VulkanBuffer.cpp"
      "src/Pikzel/Platform/Vulkan/VulkanComputeContext.h"
      "src/Pikzel/Platform/Vulkan/VulkanComputeContext.cpp"
      "src/Pikzel/Platform/Vulkan/VulkanDevice.h"
      "src/Pikzel/Platform/Vulkan/VulkanDevice.cpp"
      "src/Pikzel/Platform/Vulkan/VulkanFence.h"
      "src/Pikzel/Platform/Vulkan/VulkanFramebuffer.h"
      "src/Pikzel/Platform/Vulkan/VulkanFramebuffer.cpp"
      "src/Pikzel/Platform/Vulkan/VulkanGraphicsContext.h"
      "src/Pikzel/Platform/Vulkan/VulkanGraphicsContext.cpp"
      "src/Pikzel/Platform/Vulkan/VulkanImage.h"
      "src/Pikzel/Platform/Vulkan/VulkanImage.cpp"
      "src/Pikzel/Platform/Vulkan/VulkanMemoryAllocator.hpp"
      "src/Pikzel/Platform/Vulkan/VulkanMemoryAllocator.cpp"
      "src/Pikzel/Platform/Vulkan/VulkanPipeline.h"
      "src/Pikzel/Platform/Vulkan/VulkanPipeline.cpp"
      "src/Pikzel/Platform/Vulkan/VulkanRenderCore.h"
      "src/Pikzel/Platform/Vulkan/VulkanRenderCore.cpp"
      "src/Pikzel/Platform/Vulkan/VulkanTexture.h"
      "src/Pikzel/Platform/Vulkan/VulkanTexture.cpp"
      "src/Pikzel/Platform/Vulkan/VulkanUtility.h"
      "src/Pikzel/Platform/Vulkan/VulkanUtility.cpp"
      "vendor/imgui/backends/imgui_impl_vulkan.h"
      "vendor/imgui/backends/imgui_impl_vulkan.cpp"
      "vendor/imgui/backends/imgui_impl_glfw.h"
      "vendor/imgui/backends/imgui_impl_glfw.cpp"
   )

   target_compile_features(
      "PlatformVulkan" PRIVATE
      cxx_std_20
   )

   target_compile_definitions(
      "PlatformVulkan" PRIVATE
       VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
   )

   target_include_directories(
      "PlatformVulkan" PRIVATE
      "vendor/VulkanMemoryAllocator-Hpp/include"
      "$ENV{VULKAN_SDK}/include/vma"
      ${Vulkan_INCLUDE_DIRS}
   )

   target_precompile_headers(
      "PlatformVulkan" PRIVATE
      [["Pikzel/Core/Core.h"]]
      <vulkan/vulkan.hpp>
      <GLFW/glfw3.h>
      <glm/glm.hpp>
   )

   target_link_directories(
      "PlatformVulkan" PRIVATE
      "$ENV{VULKAN_SDK}/lib"
   )

   target_link_libraries(
      "PlatformVulkan" PRIVATE
      "Pikzel"
      "glfw"
      "glm"
      "ImGui"
      ${Vulkan_LIBRARIES}
   )

else()
   message("WARNING: Vulkan SDK not detected.  Skipping Platform Vulkan libary!")
endif()

##################################################################

add_library(
   "TracyClient" SHARED
   "vendor/tracy/public/TracyClient.cpp"
)

target_compile_features(
   "TracyClient" PRIVATE
   cxx_std_20
)

target_compile_definitions(
   "TracyClient" PRIVATE
   TRACY_ENABLE
   TRACY_EXPORTS
)

target_include_directories(
   "TracyClient" PRIVATE
   ${ProjectIncludes}
)

##################################################################
set(BUILD_SHARED_LIBS OFF)

set(ASSIMP_INJECT_DEBUG_POSTFIX OFF)
set(ASSIMP_NO_EXPORT ON)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_BUILD_ZLIB ON)
set(ASSIMP_INSTALL OFF)
add_subdirectory("vendor/assimp")

set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
set(BUILD_SHARED_LIBS ON)
add_subdirectory("vendor/glfw")
set(BUILD_SHARED_LIBS OFF)

add_subdirectory("vendor/glm")
add_subdirectory("vendor/yaml-cpp")

set(SPDLOG_USE_STD_FORMAT ON)
set(SPDLOG_BUILD_SHARED ON)
set(CMAKE_CXX_STANDARD 20)
add_subdirectory("vendor/spdlog")

# spdlog cmake is hard-coded to put spdlog shared library in the $CONFIG subdirectory.
# We need it in the same directory as our executables.
# Copy it here.
add_custom_command(
   TARGET ${PROJECT_NAME}
   POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:spdlog> ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)
