cmake_minimum_required (VERSION 3.16)

project (
   "011Skybox"
   VERSION 0.1
   DESCRIPTION "Pikzel Demo 011 - Skybox"
)

set(
   ProjectSources
   "src/Camera.h"
   "src/Camera.cpp"
   "src/ImGuiEx.h"
   "src/ImGuiEx.cpp"
   "src/Skybox.cpp"
)

set(
   ProjectIncludes
)

set(
   ProjectLibs
   "Pikzel"
   "ImGui"
)

set(
   ShaderSources
   "Assets/Shaders/Skybox.vert"
   "Assets/Shaders/Skybox.frag"
   "Assets/Shaders/TexturedModel.vert"
   "Assets/Shaders/TexturedModel.frag"
)

set(
   ShaderHeaders
)

set(
   Textures
   "Assets/Textures/Container.jpg"
   "Assets/Textures/Floor.png"
   "Assets/Textures/Skybox.jpg"
   "Assets/Textures/goegap.jpg"
)

source_group("src" FILES ${ProjectSources})
source_group("Assets/Shaders" FILES ${ShaderSources} ${ShaderHeaders})
source_group("Assets/Textures" FILES ${Textures})

add_executable(
   ${PROJECT_NAME}
   ${ProjectSources}
   ${ShaderSources}
   ${ShaderHeaders}
   ${Textures}
)

target_compile_features(
   ${PROJECT_NAME} PRIVATE
   cxx_std_20
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

compile_shaders(ShaderSources ShaderHeaders Assets/${PROJECT_NAME}/Shaders CompiledShaders)
copy_assets(Textures Assets/${PROJECT_NAME}/Textures CopiedTextures)

# These arent really "source" files.
# This line is here to make target depend on the listed files (so that cmake will then "build" them)
# The correct way to do this is to add_custom_target() and then add_dependencies() on the custom target.
# I do not want to clutter up the project with a whole load of custom targets, however.
set_source_files_properties(${CompiledAssets} PROPERTIES GENERATED TRUE)
set_source_files_properties(${CopiedTextures} PROPERTIES GENERATED TRUE)
target_sources(
   ${PROJECT_NAME} PRIVATE
   ${CompiledShaders}
   ${CopiedTextures}
)
