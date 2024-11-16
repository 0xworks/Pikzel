cmake_minimum_required (VERSION 3.20)

project (
   "009ModelAndMesh"
   VERSION 0.1
   DESCRIPTION "Pikzel Demo 009 - Model and Mesh"
)

set(
   ProjectSources
   "src/Mesh.h"
   "src/Model.h"
   "src/ModelAndMesh.cpp"
   "src/ModelSerializer.h"
   "src/ModelSerializer.cpp"
)

set(
   ProjectIncludes
)

set(
   ProjectLibs
   "Pikzel"
   "ImGui"
   "assimp"  # Note: eventually client apps will not need to directly link to assimp
)

set(
   ShaderSources
   "Assets/Shaders/Light.frag"
   "Assets/Shaders/Light.vert"
   "Assets/Shaders/Lighting.frag"
   "Assets/Shaders/Lighting.vert"
)

set(
   ShaderHeaders
)

source_group("src" FILES ${ProjectSources})
source_group("Assets/Shaders" FILES ${ShaderSources} ${ShaderHeaders})

add_executable(
   ${PROJECT_NAME}
   ${ProjectSources}
)

target_compile_definitions(
   ${PROJECT_NAME} PRIVATE
   APP_NAME="${PROJECT_NAME}"
   APP_VERSION="${PROJECT_VERSION}"
   APP_VERSION_MAJOR="${PROJECT_VERSION_MAJOR}"
   APP_VERSION_MINOR="${PROJECT_VERSION_MINOR}"
   APP_DESCRIPTION="${PROJECT_DESCRIPTION}"
)

target_include_directories(
   ${PROJECT_NAME} PRIVATE
   ${ProjectIncludes}
)

target_link_libraries(
   ${PROJECT_NAME} PRIVATE
   ${ProjectLibs}
)

add_dependencies(
   ${PROJECT_NAME}
   "Assets"
)

compile_shaders(ShaderSources ShaderHeaders "Assets/Shaders" "Assets/${PROJECT_NAME}/Shaders" CompiledShaders)

# These arent really "source" files.
# This line is here to make target depend on the listed files (so that cmake will then "build" them)
# The correct way to do this is to add_custom_target() and then add_dependencies() on the custom target.
# I do not want to clutter up the project with a whole load of custom targets, however.
set_source_files_properties(${CompiledShaders} PROPERTIES GENERATED TRUE)
target_sources(
   ${PROJECT_NAME} PRIVATE
   ${CompiledShaders}
)
