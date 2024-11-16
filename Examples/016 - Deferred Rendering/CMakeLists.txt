cmake_minimum_required (VERSION 3.20)

project (
   "016DeferredRendering"
   VERSION 0.1
   DESCRIPTION "Pikzel Demo 016 - Deferred Rendering"
)

set(
   ProjectSources
   "src/DeferredRendering.cpp"
)

set(
   ProjectIncludes
)

set(
   ProjectLibs
   "ImGui"
   "Pikzel"
)

set(
   ShaderSources
   "Assets/Shaders/GeometryPass.vert"
   "Assets/Shaders/GeometryPass.frag"
   "Assets/Shaders/LightingPass.vert"
   "Assets/Shaders/LightingPass.frag"
)

set(
   ShaderHeaders
   "Assets/Shaders/Lights.glsl"
   "Assets/Shaders/Matrices.glsl"
)

set(
   Textures
   "Assets/Textures/Container.png"
   "Assets/Textures/ContainerSpecular.png"
   "Assets/Textures/ContainerNormal.png"
   "Assets/Textures/ContainerDisplacement.png"
   "Assets/Textures/Floor.jpg"
   "Assets/Textures/FloorSpecular.jpg"
   "Assets/Textures/FloorNormal.jpg"
   "Assets/Textures/FloorDisplacement.jpg"
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

compile_shaders(ShaderSources ShaderHeaders "Assets/Shaders" "Assets/${PROJECT_NAME}/Shaders" CompiledShaders)
copy_assets(Textures "Assets/Textures" "Assets/${PROJECT_NAME}/Textures" CopiedTextures)

# These arent really "source" files.
# This line is here to make target depend on the listed files (so that cmake will then "build" them)
# The correct way to do this is to add_custom_target() and then add_dependencies() on the custom target.
# I do not want to clutter up the project with a whole load of custom targets, however.
set_source_files_properties(${CompiledShaders} PROPERTIES GENERATED TRUE)
set_source_files_properties(${CopiedTextures} PROPERTIES GENERATED TRUE)
target_sources(
   ${PROJECT_NAME} PRIVATE
   ${CompiledShaders}
   ${CopiedTextures}
)
