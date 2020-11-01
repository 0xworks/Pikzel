cmake_minimum_required (VERSION 3.16)

project (
   "010Framebuffers"
   VERSION 0.1
   DESCRIPTION "Pikzel Demo 010 - Framebuffers"
)

set(
   ProjectSources
   "src/Camera.h"
   "src/Camera.cpp"
   "src/ImGuiEx.h"
   "src/ImGuiEx.cpp"
   "src/Framebuffers.cpp"
)

set(
   ProjectIncludes
)

set(
   ProjectLibs
   "Pikzel"
)

set(
   ShaderSources
   "Assets/Shaders/PostProcess.vert"
   "Assets/Shaders/PostProcess.frag"
   "Assets/Shaders/TexturedModel.vert"
   "Assets/Shaders/TexturedModel.frag"
)

set(
   ShaderHeaders
)

set(
   Fonts
   "Assets/Fonts/LICENSE.txt"
   "Assets/Fonts/OpenSans-Bold.ttf"
   "Assets/Fonts/OpenSans-Regular.ttf"
)

set(
   Textures
   "Assets/Textures/Container.jpg"
   "Assets/Textures/Floor.png"
)

compile_shaders(ShaderSources ShaderHeaders Assets/Shaders CompiledShaders)
copy_assets(Fonts Assets/Fonts CopiedFonts)
copy_assets(Textures Assets/Textures CopiedTextures)

source_group("src" FILES ${ProjectSources})
source_group("Assets/Fonts" FILES ${Fonts})
source_group("Assets/Shaders" FILES ${ShaderSources} ${ShaderHeaders})
source_group("Assets/Textures" FILES ${Textures})

add_executable(
   ${PROJECT_NAME}
   ${ProjectSources}
   ${Fonts}
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

# These arent really "source" files.
# This line is here to make target depend on the listed files (so that cmake will then "build" them)
# The correct way to do this is to add_custom_target() and then add_dependencies() on the custom target.
# I do not want to clutter up the project with a whole load of custom targets, however.
set_source_files_properties(${CompiledAssets} PROPERTIES GENERATED TRUE)
set_source_files_properties(${CopiedTextures} PROPERTIES GENERATED TRUE)
target_sources(
   ${PROJECT_NAME} PRIVATE
   ${CompiledShaders}
   ${CopiedFonts}
   ${CopiedTextures}
)
