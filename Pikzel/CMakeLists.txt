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
   "src/Pikzel/Core/Utility.h"
   "src/Pikzel/Core/Utility.cpp"
   "src/Pikzel/Core/Window.h"
   "src/Pikzel/Events/EventDispatcher.h"
   "src/Pikzel/Events/EventDispatcher.cpp"
   "src/Pikzel/Events/KeyEvents.h"
   "src/Pikzel/Events/MouseEvents.h"
   "src/Pikzel/Events/WindowEvents.h"
   "src/Pikzel/Renderer/Buffer.h"
   "src/Pikzel/Renderer/Buffer.cpp"
   "src/Pikzel/Renderer/GraphicsContext.h"
   "src/Pikzel/Renderer/RenderCore.h"
   "src/Pikzel/Renderer/RenderCore.cpp"
   "src/Pikzel/Renderer/Shader.h"
   "src/Pikzel/Renderer/Texture.h"
   "src/Pikzel/Renderer/VertexArray.h"
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
   "vendor/stb"
   "vendor/tinyobjloader"
   "vendor/tracy"
)

set(
   ProjectDefines
   PKZL_VERSION="${PROJECT_VERSION}"
   PKZL_VERSION_MAJOR="${PROJECT_VERSION_MAJOR}"
   PKZL_VERSION_MINOR="${PROJECT_VERSION_MINOR}"
   PKZL_DESCRIPTION="${PROJECT_DESCRIPTION}"
   "$<$<CONFIG:DEBUG>:PKZL_DEBUG>"
   GLM_FORCE_DEPTH_ZERO_TO_ONE
   GLM_FORCE_RADIANS
)

set(
   ProjectLibs
   "glfw"
   "glm"
   "PlatformOpenGL"
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
   target_link_options(
      ${PROJECT_NAME} PUBLIC
      "$<$<CONFIG:Release>:/LTCG>"
   )
endif()

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
   "src/Pikzel/Platform/OpenGL/OpenGLBuffer.h"
   "src/Pikzel/Platform/OpenGL/OpenGLBuffer.cpp"
   "src/Pikzel/Platform/OpenGL/OpenGLGraphicsContext.h"
   "src/Pikzel/Platform/OpenGL/OpenGLGraphicsContext.cpp"
   "src/Pikzel/Platform/OpenGL/OpenGLRenderCore.h"
   "src/Pikzel/Platform/OpenGL/OpenGLRenderCore.cpp"
   "src/Pikzel/Platform/OpenGL/OpenGLShader.h"
   "src/Pikzel/Platform/OpenGL/OpenGLShader.cpp"
   "src/Pikzel/Platform/OpenGL/OpenGLTexture.h"
   "src/Pikzel/Platform/OpenGL/OpenGLTexture.cpp"
   "src/Pikzel/Platform/OpenGL/OpenGLVertexArray.h"
   "src/Pikzel/Platform/OpenGL/OpenGLVertexArray.cpp"
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

add_subdirectory("vendor/glfw")
add_subdirectory("vendor/glm")
