cmake_minimum_required(VERSION 3.16)

# Currently the Vulkan package is required, even if you are only going to use the OpenGL rendering backend
# Because we are using shaderc and SPIRV-cross shipped with the Vulkan SDK (rather than trying to build
# those things + dependencies ourselves)
# TODO: use an independent copy of shaderc / SPIRV-cross so that if you are building for OpenGL then you do not
#       need the Vulkan SDK to be installed
find_package(Vulkan REQUIRED)

project (
   "Pikzel"
   VERSION 0.1
   DESCRIPTION "Pikzel Engine"
)

set(
   ProjectSources
   "src/Pikzel/Core/Application.h"
   "src/Pikzel/Core/Application.cpp"
   "src/Pikzel/Core/Core.h"
   "src/Pikzel/Core/Instrumentor.h"
   "src/Pikzel/Core/main.cpp"
   "src/Pikzel/Core/Log.h"
   "src/Pikzel/Core/Log.cpp"
   "src/Pikzel/Core/Utility.h"
   "src/Pikzel/Core/Utility.cpp"
   "src/Pikzel/Core/Window.h"
   "src/Pikzel/Events/EventDispatcher.h"
   "src/Pikzel/Events/KeyEvents.h"
   "src/Pikzel/Events/MouseEvents.h"
   "src/Pikzel/Events/WindowEvents.h"
   "src/Pikzel/Input/Input.h"
   "src/Pikzel/Input/Input.cpp"
   "src/Pikzel/Renderer/Buffer.h"
   "src/Pikzel/Renderer/Buffer.cpp"
   "src/Pikzel/Renderer/GraphicsContext.h"
   "src/Pikzel/Renderer/Pipeline.h"
   "src/Pikzel/Renderer/RenderCore.h"
   "src/Pikzel/Renderer/RenderCore.cpp"
   "src/Pikzel/Renderer/ShaderUtil.h"
   "src/Pikzel/Renderer/ShaderUtil.cpp"
   "src/Pikzel/Renderer/Texture.h"
   "vendor/imgui/imgui.cpp"
   "vendor/imgui/imgui_draw.cpp"
   "vendor/imgui/imgui_widgets.cpp"
   "vendor/tracy/TracyClient.cpp"
)

if(WIN32)
   list(
      APPEND ProjectSources
      "src/Pikzel/Platform/Windows/WindowsWindow.cpp"
   )
endif()

# Vulkan include path needed because that is where shaderc is.  TODO: improve this situation
set(
   ProjectIncludes
   "src"
   "vendor/entt/src"
   "vendor/glfw/include"
   "vendor/glm"
   "vendor/imgui"
   "vendor/spdlog/include"
   "vendor/stb"
   "vendor/tinyobjloader"
   "vendor/tracy"
   ${Vulkan_INCLUDE_DIRS}
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
)

set(
   ProjectLibs
   "glfw"
   "glm"
   #"PlatformOpenGL"
   "PlatformVulkan"
   "shaderc_combined$<$<CONFIG:Debug>:d>"
   "spirv-cross-core$<$<CONFIG:Debug>:d>"
   "spirv-cross-glsl$<$<CONFIG:Debug>:d>"
)

add_library(${PROJECT_NAME} STATIC ${ProjectSources})

target_compile_features(
   ${PROJECT_NAME} PUBLIC
   cxx_std_20
)

target_compile_definitions(
   ${PROJECT_NAME} PUBLIC
   ${ProjectDefines}
)

if(PKZL_PROFILE)
   target_compile_definitions(
      ${PROJECT_NAME} PUBLIC
      PKZL_PROFILE
      TRACY_ENABLE
)
endif()

target_include_directories(
   ${PROJECT_NAME} PUBLIC
   ${ProjectIncludes}
)

target_precompile_headers(
   ${PROJECT_NAME} PRIVATE
   [["Pikzel/Core/Core.h"]]
)

if(MSVC)
   target_compile_options(
      ${PROJECT_NAME} PUBLIC
      "$<$<CONFIG:Release>:/GL>"
   )
endif()

# Vulkan lib path needed because that is where shaderc is.  TODO: improve this situation
target_link_directories(
   ${PROJECT_NAME} PUBLIC
   "$ENV{VULKAN_SDK}/lib"
)

target_link_libraries(
   ${PROJECT_NAME} PUBLIC
   ${ProjectLibs}
)

if(MSVC)
   target_link_options(
      ${PROJECT_NAME} PUBLIC
      "$<$<CONFIG:Release>:/LTCG>"
      "$<$<CONFIG:Debug>:/ignore:4099>"
   )
endif()

# no precompiled header for ImGui or TracyClient
set_source_files_properties(
   "vendor/imgui/imgui.cpp"
   "vendor/imgui/imgui_draw.cpp"
   "vendor/imgui/imgui_widgets.cpp"
   "vendor/tracy/TracyClient.cpp"
   PROPERTIES
   SKIP_PRECOMPILE_HEADERS ON
)


# Platform - OpenGL
#
# GLAD Source files generated from https://glad.dav1d.de/
#   Language: C/C++
#   Specification: OpenGL
#   API: Version 4.6
#   Profile: Core
add_library(
   "PlatformOpenGL" STATIC
   
   
   "src/Pikzel/Platform/OpenGL/OpenGLBuffer.h"
   "src/Pikzel/Platform/OpenGL/OpenGLBuffer.cpp"
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
   "vendor/imgui/examples/imgui_impl_glfw.h"
   "vendor/imgui/examples/imgui_impl_glfw.cpp"
   "vendor/imgui/examples/imgui_impl_opengl3.h"
   "vendor/imgui/examples/imgui_impl_opengl3.cpp"
)

target_compile_features(
   "PlatformOpenGL" PRIVATE
   cxx_std_20
)

target_compile_definitions(
   "PlatformOpenGL" PRIVATE
   ${ProjectDefines}
)

target_include_directories(
   "PlatformOpenGL" PRIVATE
   ${ProjectIncludes}
   "src/Pikzel/Platform/OpenGL/vendor/glad/include"
)

target_precompile_headers(
   "PlatformOpenGL" PRIVATE
   [["Pikzel/Core/Core.h"]]
   <glad/glad.h>
   <GLFW/glfw3.h>
)

# add glad.c source file after setting the precompiled header
# as otherwise the add_precompiled_header macro gets confused...
target_sources(
   "PlatformOpenGL" PRIVATE
   "src/Pikzel/Platform/OpenGL/vendor/glad/src/glad.c"
)

# no precompiled header for ImGui or GLAD
set_source_files_properties(
   "src/Pikzel/Platform/OpenGL/vendor/glad/src/glad.c"
   "vendor/imgui/examples/imgui_impl_glfw.h"
   "vendor/imgui/examples/imgui_impl_glfw.cpp"
   "vendor/imgui/examples/imgui_impl_opengl3.h"
   "vendor/imgui/examples/imgui_impl_opengl3.cpp"
   PROPERTIES
   SKIP_PRECOMPILE_HEADERS ON
)

if(${Vulkan_FOUND})

   message("Found ${Vulkan_LIBRARY}")

   # Platform - Vulkan
   # This requires that the Vulkan SDK is installed.
   # ImGui vuklan backend is using the example code from the ImGui docking branch, with https://github.com/ocornut/imgui/pull/914 merged in
   add_library(
      "PlatformVulkan" STATIC
      "src/Pikzel/Platform/Vulkan/DescriptorBinding.h"
      "src/Pikzel/Platform/Vulkan/QueueFamilyIndices.h"
      "src/Pikzel/Platform/Vulkan/SwapChainSupportDetails.h"
      "src/Pikzel/Platform/Vulkan/VulkanBuffer.h"
      "src/Pikzel/Platform/Vulkan/VulkanBuffer.cpp"
      "src/Pikzel/Platform/Vulkan/VulkanDevice.h"
      "src/Pikzel/Platform/Vulkan/VulkanDevice.cpp"
      "src/Pikzel/Platform/Vulkan/VulkanGraphicsContext.h"
      "src/Pikzel/Platform/Vulkan/VulkanGraphicsContext.cpp"
      "src/Pikzel/Platform/Vulkan/VulkanImage.h"
      "src/Pikzel/Platform/Vulkan/VulkanImage.cpp"
      #"src/Pikzel/Platform/Vulkan/VulkanImageGC.h"
      #"src/Pikzel/Platform/Vulkan/VulkanImageGC.cpp"
      "src/Pikzel/Platform/Vulkan/VulkanPipeline.h"
      "src/Pikzel/Platform/Vulkan/VulkanPipeline.cpp"
      "src/Pikzel/Platform/Vulkan/VulkanRenderCore.h"
      "src/Pikzel/Platform/Vulkan/VulkanRenderCore.cpp"
      "src/Pikzel/Platform/Vulkan/VulkanTexture.h"
      "src/Pikzel/Platform/Vulkan/VulkanTexture.cpp"
      "src/Pikzel/Platform/Vulkan/VulkanUtility.h"
      "src/Pikzel/Platform/Vulkan/VulkanUtility.cpp"
      "src/Pikzel/Platform/Vulkan/VulkanWindowGC.h"
      "src/Pikzel/Platform/Vulkan/VulkanWindowGC.cpp"
      #"src/Pikzel/Platform/Vulkan/imgui_impl_vulkan.h"
      #"src/Pikzel/Platform/Vulkan/imgui_impl_vulkan.cpp"
      #"vendor/imgui/examples/imgui_impl_glfw.h"
      #"vendor/imgui/examples/imgui_impl_glfw.cpp"
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

   # HACK: these includes should be private
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
   )

   target_link_directories(
      "PlatformVulkan" PUBLIC
      "$ENV{VULKAN_SDK}/lib"
   )

   target_link_libraries(
      "PlatformVulkan" PRIVATE
      ${Vulkan_LIBRARIES}
   )

   # no precompiled header for ImGui
   set_source_files_properties(
      "src/Pikzel/Platform/Vulkan/imgui_impl_vulkan.h"
      "src/Pikzel/Platform/Vulkan/imgui_impl_vulkan.cpp"
      "vendor/imgui/examples/imgui_impl_glfw.h"
      "vendor/imgui/examples/imgui_impl_glfw.cpp"
      PROPERTIES
      SKIP_PRECOMPILE_HEADERS ON
   )

else()
   message("WARNING: Vulkan SDK not detected.  Skipping Platform Vulkan libary!")
endif()

add_subdirectory("vendor/glfw")
add_subdirectory("vendor/glm")
