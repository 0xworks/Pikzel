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
   "src/Pikzel/Events/Events.h"
   "src/Pikzel/Events/KeyEvents.h"
   "src/Pikzel/Events/MouseEvents.h"
   "src/Pikzel/Events/WindowEvents.h"
   "src/Platform/Windows/WindowsWindow.h"
   "src/Platform/Windows/WindowsWindow.cpp"
   "vendor/tracy/TracyClient.cpp"
)

set(
   ProjectIncludes
   "src"
   "vendor/entt/src"
   "vendor/glad/include"
   "vendor/glfw/include"
   "vendor/glm"
   "vendor/imgui"
   "vendor/spdlog/include"
   "vendor/tracy"
)

set(
   ProjectLibs
   "glad"
   "glfw"
   "glm"
   "imgui"
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

# no precompiled header for TracyClient.cpp
set_source_files_properties(
   "vendor/tracy/TracyClient.cpp" PROPERTIES
   COMPILE_FLAGS /Y-
)


# TODO: We only need GLAD if using OpenGL...
#       So deal with this with some sort of "platform" option somehow.

# GLAD library
# Source files generated from https://glad.dav1d.de/
#   Language: C/C++
#   Specification: OpenGL
#   API: Version 4.6
#   Profile: Core
add_library(
   "glad" STATIC
   "vendor/glad/include/glad/glad.h"
   "vendor/glad/include/KHR/khrplatform.h"
   "vendor/glad/src/glad.c"
)

target_include_directories(
   "glad" PRIVATE
   "vendor/glad/include"
)


#
# TODO: The "examples" source files here are providing the "backend"
#       for imgui.  These are dependent on the rendering API used
#       and should not be in the ImGui library.
#       Instead, put them into some sort of "platform" dependent
#       library

# IMGui Library
# Source files per vendor/imgui/examples/example_glfw_opengl3
# Depends on an OpenGl Loader.
# We are using glad, this is autodetected via putting glad in the include path below
add_library(
   "imgui" STATIC
   "vendor/imgui/imgui.cpp"
   "vendor/imgui/imgui_draw.cpp"
   "vendor/imgui/imgui_widgets.cpp"
   "vendor/imgui/examples/imgui_impl_glfw.cpp"
   "vendor/imgui/examples/imgui_impl_opengl3.cpp"
)

target_include_directories(
   "imgui" PRIVATE
   "vendor/imgui"
)

# This is needed because Pikzel provides the OpenGL loader (glad) for ImGUI
target_link_libraries(
   "imgui" PRIVATE
   "Pikzel"
)


add_subdirectory("vendor/glfw")
add_subdirectory("vendor/glm")