cmake_minimum_required (VERSION 3.20)

project (
   "013.2SponzaShadows"
   VERSION 0.1
   DESCRIPTION "Pikzel Demo 013.2 - Sponza Shadows"
)

set(
   ProjectSources
   "src/Mesh.h"
   "src/Model.h"
   "src/SponzaShadows.cpp"
   "src/ModelSerializer.h"
   "src/ModelSerializer.cpp"
)

set(
   ProjectIncludes
)

set(
   ProjectLibs
   "assimp"  # Note: eventually client apps will not need to directly link to assimp
   "ImGui"
   "Pikzel"
)

set(
   ShaderSources
   "Assets/Shaders/Depth.frag"
   "Assets/Shaders/Depth.vert"
   "Assets/Shaders/DepthCube.frag"
   "Assets/Shaders/DepthCube.geom"
   "Assets/Shaders/DepthCube.vert"
   "Assets/Shaders/Light.frag"
   "Assets/Shaders/Light.vert"
   "Assets/Shaders/LitModel.frag"
   "Assets/Shaders/LitModel.vert"
)

set(
   ShaderHeaders
   "Assets/Shaders/Lights.glsl"
   "Assets/Shaders/Matrices.glsl"
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
