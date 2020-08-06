cmake_minimum_required(VERSION 3.10)

project ("Pikzel" VERSION 0.1 DESCRIPTION "Pikzel Interactive App Engine Library")

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
    "vendor/tracy/TracyClient.cpp"
)

set(
   ProjectIncludes
   "src"
   "vendor/glad/include"
   "vendor/glfw/include"
   "vendor/entt/src"
   "vendor/spdlog/include"
   "vendor/tracy"
)

set(
   ProjectLibs
   "glad"
   "glfw"
#   "glm"
)

add_library(${PROJECT_NAME} STATIC ${ProjectSources})

set_target_properties(
   ${PROJECT_NAME} PROPERTIES
   CXX_STANDARD 20
)

target_compile_definitions(
   ${PROJECT_NAME} PUBLIC
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


add_subdirectory("vendor/glfw")
