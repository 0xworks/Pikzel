cmake_minimum_required(VERSION 3.16)

find_package(Vulkan)

include("vendor/cmrc/CMakeRC.cmake")

project (
   "Pikzel"
   VERSION 0.1
   DESCRIPTION "Pikzel Engine"
)

option(
   PKZL_PROFILE "Performance profiling instrumentation is compiled in to the code" OFF
)

set(
   ProjectSources
   "src/Pikzel/Core/Application.h"
   "src/Pikzel/Core/Application.cpp"
   "src/Pikzel/Core/Core.h"
   "src/Pikzel/Core/EntryPoint.h"
   "src/Pikzel/Core/Instrumentor.h"
   "src/Pikzel/Core/Log.h"
   "src/Pikzel/Core/Log.cpp"
   "src/Pikzel/Core/PlatformUtility.h"
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
   "src/Pikzel/Renderer/Buffer.h"
   "src/Pikzel/Renderer/Buffer.cpp"
   "src/Pikzel/Renderer/ComputeContext.h"
   "src/Pikzel/Renderer/Framebuffer.h"
   "src/Pikzel/Renderer/Framebuffer.cpp"
   "src/Pikzel/Renderer/GraphicsContext.h"
   "src/Pikzel/Renderer/MeshRenderer.h"
   "src/Pikzel/Renderer/MeshRenderer.cpp"
   "src/Pikzel/Renderer/ModelRenderer.h"
   "src/Pikzel/Renderer/ModelRenderer.cpp"
   "src/Pikzel/Renderer/Pipeline.h"
   "src/Pikzel/Renderer/RenderCore.h"
   "src/Pikzel/Renderer/RenderCore.cpp"
   "src/Pikzel/Renderer/ShaderUtil.h"
   "src/Pikzel/Renderer/ShaderUtil.cpp"
   "src/Pikzel/Renderer/sRGB.h"
   "src/Pikzel/Renderer/sRGB.cpp"
   "src/Pikzel/Renderer/Texture.h"
   "src/Pikzel/Scene/Camera.h"
   "src/Pikzel/Scene/Camera.cpp"
   "src/Pikzel/Scene/Light.h"
   "src/Pikzel/Scene/Mesh.h"
   "src/Pikzel/Scene/Model.h"
   "src/Pikzel/Scene/ModelSerializer.h"
   "src/Pikzel/Scene/ModelSerializer.cpp"
   "vendor/imgui/imgui.cpp"
   "vendor/imgui/imgui_draw.cpp"
   "vendor/imgui/imgui_widgets.cpp"
)

set(
   ProjectIncludes
   "src"
   "vendor/entt/src"
   "vendor/glfw/include"
   "vendor/glm"
   "vendor/imgui"
   "vendor/spdlog/include"
   "vendor/SPIRV-cross"
   "vendor/stb"
   "vendor/tracy"
   ${CMRC_INCLUDE_DIR}
)

set(
   ProjectDefines
   PKZL_VERSION="${PROJECT_VERSION}"
   PKZL_VERSION_MAJOR="${PROJECT_VERSION_MAJOR}"
   PKZL_VERSION_MINOR="${PROJECT_VERSION_MINOR}"
   PKZL_DESCRIPTION="${PROJECT_DESCRIPTION}"
   "$<$<CONFIG:DEBUG>:PKZL_DEBUG>"
   NOMINMAX
   GLFW_INCLUDE_NONE
   GLM_FORCE_DEPTH_ZERO_TO_ONE
   GLM_FORCE_RADIANS
   TRACY_IMPORTS
)

if(WIN32)
   list(
      APPEND ProjectSources
      "src/Pikzel/Platform/Windows/WindowsPlatformUtility.cpp"
      "src/Pikzel/Platform/Windows/WindowsWindow.cpp"
   )

   list(
      APPEND ProjectDefines
      WIN32_LEAN_AND_MEAN
   )
endif()

if(PKZL_PROFILE)
   list(
      APPEND ProjectDefines
      PKZL_PROFILE
      TRACY_ENABLE
   )
endif()

set(
   ProjectLibs
   "assimp"
   "glfw"
   "glm"
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
)

target_include_directories(
   ${PROJECT_NAME} PUBLIC
   ${ProjectIncludes}
)

target_precompile_headers(
   ${PROJECT_NAME} PRIVATE
   [["Pikzel/Core/Core.h"]]
   <GLFW/glfw3.h>
   <glm/glm.hpp>
   <imgui.h>
)

if(MSVC)
   target_compile_options(
      ${PROJECT_NAME} PUBLIC
      "$<$<CONFIG:Release>:/GL>"
   )
endif()

target_link_libraries(
   ${PROJECT_NAME} PRIVATE
   ${ProjectLibs}
)

target_link_libraries(
   ${PROJECT_NAME} PUBLIC
   "TracyClient"
)

if(MSVC)
   target_link_options(
      ${PROJECT_NAME} PRIVATE
      "$<$<CONFIG:Release>:/LTCG>"
      "$<$<CONFIG:Debug>:/ignore:4099>"
   )
endif()


##################################################################

set(
   RendererShaderSources
   "src/Pikzel/Renderer/EquirectangularToCubeMap.comp"
   "src/Pikzel/Renderer/SixFacesToCubeMap.comp"
)

set(
   RendererShaderHeaders
)

set(
   SceneShaderSources
   "src/Pikzel/Scene/Shaders/Depth.vert"
   "src/Pikzel/Scene/Shaders/Depth.frag"
   "src/Pikzel/Scene/Shaders/DepthCube.vert"
   "src/Pikzel/Scene/Shaders/DepthCube.geom"
   "src/Pikzel/Scene/Shaders/DepthCube.frag"
   "src/Pikzel/Scene/Shaders/LitModel.vert"
   "src/Pikzel/Scene/Shaders/LitModel.frag"
)

set(
   SceneShaderHeaders
   "src/Pikzel/Scene/Shaders/Lights.glsl"
   "src/Pikzel/Scene/Shaders/Matrices.glsl"
)

compile_shaders(RendererShaderSources RendererShaderHeaders ${CMAKE_CURRENT_BINARY_DIR}/Renderer CompiledRendererShaders)
compile_shaders(SceneShaderSources SceneShaderHeaders ${CMAKE_CURRENT_BINARY_DIR}/Scene/Shaders CompiledSceneShaders)

cmrc_add_resource_library(
   PikzelResources WHENCE ${CMAKE_CURRENT_BINARY_DIR}
   ${CompiledRendererShaders}
   ${CompiledSceneShaders}
)

##################################################################
add_library(
   "ImGui" STATIC
   "src/Pikzel/ImGui/ImGuiEx.h"
   "src/Pikzel/ImGui/ImGuiEx.cpp"
   "vendor/imgui/imgui.cpp"
   "vendor/imgui/imgui_draw.cpp"
   "vendor/imgui/imgui_widgets.cpp"
)

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
   "src/Pikzel/Platform/OpenGL/OpenGLMeshRenderer.cpp"
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
   "PlatformOpenGL" PUBLIC
   ${ProjectDefines}
)

target_include_directories(
   "PlatformOpenGL" PRIVATE
   ${ProjectIncludes}
   "src/Pikzel/Platform/OpenGL/vendor/glad/include"
)


target_link_libraries(
   "PlatformOpenGL" PRIVATE
   "PikzelResources"
   "Pikzel"
   "glfw"
   "glm"
   "ImGui"
   "spirv-cross-core"
   "spirv-cross-glsl"
   "TracyClient"
)

target_precompile_headers(
   "PlatformOpenGL" PRIVATE
   [["Pikzel/Core/Core.h"]]
   <glad/glad.h>
   <GLFW/glfw3.h>
   <glm/glm.hpp>
)

# no precompiled header for ImGui or GLAD
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
      "src/Pikzel/Platform/Vulkan/VulkanMeshRenderer.h"
      "src/Pikzel/Platform/Vulkan/VulkanMeshRenderer.cpp"
      "src/Pikzel/Platform/Vulkan/VulkanPipeline.h"
      "src/Pikzel/Platform/Vulkan/VulkanPipeline.cpp"
      "src/Pikzel/Platform/Vulkan/VulkanRenderCore.h"
      "src/Pikzel/Platform/Vulkan/VulkanRenderCore.cpp"
      "src/Pikzel/Platform/Vulkan/VulkanTexture.h"
      "src/Pikzel/Platform/Vulkan/VulkanTexture.cpp"
      "src/Pikzel/Platform/Vulkan/VulkanUtility.h"
      "src/Pikzel/Platform/Vulkan/VulkanUtility.cpp"
      "src/Pikzel/Platform/Vulkan/imgui_impl_vulkan.h"
      "src/Pikzel/Platform/Vulkan/imgui_impl_vulkan.cpp"
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
       ${ProjectDefines}
   )

   target_include_directories(
      "PlatformVulkan" PRIVATE
      ${ProjectIncludes}
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
      "PikzelResources"
      "Pikzel"
      "glfw"
      "glm"
      "ImGui"
      "spirv-cross-core"
      "spirv-cross-glsl"
      "TracyClient"
      ${Vulkan_LIBRARIES}
   )

else()
   message("WARNING: Vulkan SDK not detected.  Skipping Platform Vulkan libary!")
endif()

##################################################################

add_library(
   "TracyClient" SHARED
   "vendor/tracy/TracyClient.cpp"
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
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
set(ASSIMP_INJECT_DEBUG_POSTFIX OFF CACHE BOOL "Inject debug postfix in .a/.so/.dll lib names" FORCE)

set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "If the supplementary tools for Assimp are built in addition to the library." FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "If the test suite for Assimp is built in addition to the library." FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "Disable this if you want to use assimp as a submodule." FORCE)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

set(SPIRV_CROSS_SKIP_INSTALL ON CACHE BOOL "Skips installation targets." FORCE)
set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "Enable SPIRV-Cross tests." FORCE)
set(SPIRV_CROSS_CLI OFF CACHE BOOL "Build the CLI binary. Requires SPIRV_CROSS_STATIC." FORCE)

add_subdirectory("vendor/assimp")

set(BUILD_SHARED_LIBS ON)
add_subdirectory("vendor/glfw")
set(BUILD_SHARED_LIBS OFF)

add_subdirectory("vendor/glm")
add_subdirectory("vendor/SPIRV-Cross")
