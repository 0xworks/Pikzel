cmake_minimum_required(VERSION 3.10)

find_package(Vulkan)

project (
   "Pikzel"
   VERSION 0.1
   DESCRIPTION "Pikzel Engine"
)

set(
   ProjectSources
   "src/pch.cpp"
   "src/Pikzel/Core/Application.h"
   "src/Pikzel/Core/Application.cpp"
   "src/Pikzel/Core/Core.h"
   "src/Pikzel/Core/Instrumentor.h"
   "src/Pikzel/Core/main.cpp"
   "src/Pikzel/Core/Log.h"
   "src/Pikzel/Core/Log.cpp"
   "src/Pikzel/Core/Window.h"
   "src/Pikzel/Events/EventDispatcher.h"
   "src/Pikzel/Events/EventDispatcher.cpp"
   "src/Pikzel/Events/KeyEvents.h"
   "src/Pikzel/Events/MouseEvents.h"
   "src/Pikzel/Events/WindowEvents.h"
   "src/Pikzel/Renderer/GraphicsContext.h"
   "src/Pikzel/Renderer/RenderCore.h"
   "src/Pikzel/Renderer/Renderer.h"
   "src/Pikzel/Renderer/Renderer.cpp"
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

set(
   ProjectIncludes
   "src"
   "vendor/entt/src"
   "vendor/glfw/include"
   "vendor/glm"
   "vendor/imgui"
   "vendor/spdlog/include"
   "vendor/tracy"
)

set(
   ProjectLibs
   "glfw"
   "glm"
   #"PlatformOpenGL"
   "PlatformVulkan"
)

add_library(${PROJECT_NAME} STATIC ${ProjectSources})

target_compile_features(
   ${PROJECT_NAME} PUBLIC
   cxx_std_20
)

target_compile_definitions(
   ${PROJECT_NAME} PUBLIC
   PKZL_VERSION="${PROJECT_VERSION}"
   PKZL_VERSION_MAJOR="${PROJECT_VERSION_MAJOR}"
   PKZL_VERSION_MINOR="${PROJECT_VERSION_MINOR}"
   PKZL_DESCRIPTION="${PROJECT_DESCRIPTION}"
   "$<$<CONFIG:DEBUG>:PKZL_DEBUG>"
   GLM_FORCE_DEPTH_ZERO_TO_ONE
   GLM_FORCE_RADIANS
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

target_link_libraries(
   ${PROJECT_NAME} PUBLIC
   ${ProjectLibs}
)

add_precompiled_header(${PROJECT_NAME} "pch.h" SOURCE_CXX "src/pch.cpp")

# no precompiled header for ImGui or TracyClient
set_source_files_properties(
   "vendor/imgui/imgui.cpp"
   "vendor/imgui/imgui_draw.cpp"
   "vendor/imgui/imgui_widgets.cpp"
   "vendor/tracy/TracyClient.cpp"
   PROPERTIES
   COMPILE_FLAGS /Y-
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
   "src/Pikzel/Platform/OpenGL/glpch.cpp"
   "src/Pikzel/Platform/OpenGL/OpenGLGraphicsContext.h"
   "src/Pikzel/Platform/OpenGL/OpenGLGraphicsContext.cpp"
   "src/Pikzel/Platform/OpenGL/OpenGLRenderCore.h"
   "src/Pikzel/Platform/OpenGL/OpenGLRenderCore.cpp"
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
    GLM_FORCE_DEPTH_ZERO_TO_ONE
    GLM_FORCE_RADIANS
)

target_include_directories(
   "PlatformOpenGL" PRIVATE
   ${ProjectIncludes}
   "src/Pikzel/Platform/OpenGL/vendor/glad/include"
)

add_precompiled_header("PlatformOpenGL" "glpch.h" SOURCE_CXX "src/Pikzel/Platform/OpenGL/glpch.cpp")

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
   COMPILE_FLAGS /Y-
)

if(${Vulkan_FOUND})

   message("Found ${Vulkan_LIBRARY}")

   # Platform - Vulkan
   # This requires that the Vulkan SDK is installed
   add_library(
      "PlatformVulkan" STATIC
      "src/Pikzel/Platform/Vulkan/vkpch.cpp"
      "src/Pikzel/Platform/Vulkan/Buffer.h"
      "src/Pikzel/Platform/Vulkan/Buffer.cpp"
      "src/Pikzel/Platform/Vulkan/Image.h"
      "src/Pikzel/Platform/Vulkan/Image.cpp"
      "src/Pikzel/Platform/Vulkan/QueueFamilyIndices.h"
      "src/Pikzel/Platform/Vulkan/SwapChainSupportDetails.h"
      "src/Pikzel/Platform/Vulkan/VulkanGraphicsContext.h"
      "src/Pikzel/Platform/Vulkan/VulkanGraphicsContext.cpp"
      "src/Pikzel/Platform/Vulkan/VulkanRenderCore.h"
      "src/Pikzel/Platform/Vulkan/VulkanRenderCore.cpp"
      "vendor/imgui/examples/imgui_impl_glfw.h"
      "vendor/imgui/examples/imgui_impl_glfw.cpp"
      "vendor/imgui/examples/imgui_impl_vulkan.h"
      "vendor/imgui/examples/imgui_impl_vulkan.cpp"
   )

   target_compile_features(
      "PlatformVulkan" PRIVATE
      cxx_std_20
   )

   target_compile_definitions(
      "PlatformVulkan" PRIVATE
       VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
       GLFW_INCLUDE_NONE
       GLM_FORCE_DEPTH_ZERO_TO_ONE
       GLM_FORCE_RADIANS
   )

   target_include_directories(
      "PlatformVulkan" PRIVATE
      ${ProjectIncludes}
      ${Vulkan_INCLUDE_DIR}
   )

   target_link_libraries(
      "PlatformVulkan" PUBLIC
      ${Vulkan_LIBRARY}
   )

   add_precompiled_header("PlatformVulkan" "vkpch.h" SOURCE_CXX "src/Pikzel/Platform/Vulkan/vkpch.cpp")

   # no precompiled header for ImGui
   set_source_files_properties(
      "vendor/imgui/examples/imgui_impl_glfw.h"
      "vendor/imgui/examples/imgui_impl_glfw.cpp"
      "vendor/imgui/examples/imgui_impl_vulkan.h"
      "vendor/imgui/examples/imgui_impl_vulkan.cpp"
      PROPERTIES
      COMPILE_FLAGS /Y-
   )

else()
   message("WARNING: Vulkan SDK not detected.  Skipping Platform Vulkan libary!")
endif()


add_subdirectory("vendor/glfw")
add_subdirectory("vendor/glm")
