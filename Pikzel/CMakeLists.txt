cmake_minimum_required(VERSION 3.10)

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
   "PlatformOpenGL"
)

add_library(${PROJECT_NAME} STATIC ${ProjectSources})

set_target_properties(
   ${PROJECT_NAME} PROPERTIES
   CXX_STANDARD 20
)

target_compile_definitions(
   ${PROJECT_NAME} PUBLIC
   PKZL_VERSION="${PROJECT_VERSION}"
   PKZL_DESCRIPTION="${PROJECT_DESCRIPTION}"
   "$<$<CONFIG:DEBUG>:PKZL_DEBUG>"
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

# GLAD Source files generated from https://glad.dav1d.de/
#   Language: C/C++
#   Specification: OpenGL
#   API: Version 4.6
#   Profile: Core
add_library(
   "PlatformOpenGL" STATIC
   "src/Pikzel/Platform/OpenGL/OpenGLGraphicsContext.h"
   "src/Pikzel/Platform/OpenGL/OpenGLGraphicsContext.cpp"
   "src/Pikzel/Platform/OpenGL/OpenGLRenderCore.h"
   "src/Pikzel/Platform/OpenGL/OpenGLRenderCore.cpp"
   "src/Pikzel/Platform/OpenGL/vendor/glad/include/glad/glad.h"
   "src/Pikzel/Platform/OpenGL/vendor/glad/include/KHR/khrplatform.h"
   "src/Pikzel/Platform/OpenGL/vendor/glad/src/glad.c"
   "vendor/imgui/examples/imgui_impl_glfw.cpp"
   "vendor/imgui/examples/imgui_impl_opengl3.cpp"
)

target_include_directories(
   "PlatformOpenGL" PRIVATE
   ${ProjectIncludes}
   "src/Pikzel/Platform/OpenGL/vendor/glad/include"
)

add_subdirectory("vendor/glfw")
add_subdirectory("vendor/glm")
