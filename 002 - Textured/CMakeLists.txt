cmake_minimum_required (VERSION 3.10)

project (
   "002Textured"
   VERSION 0.1
   DESCRIPTION "Pikzel Demo 002 - Textured"
)

set(
   ProjectSources
   "src/Textured.cpp"
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
   "Assets/Shaders/Textured.vert"
   "Assets/Shaders/Textured.frag"
)

set(
   Textures
   "Assets/Textures/Container.jpg"
)

copy_assets(ShaderSources Assets/Shaders copiedShaders)
copy_assets(Textures Assets/Textures copiedTextures)

source_group("src" FILES ${ProjectSources})
source_group("Assets/Shaders" FILES ${ShaderSources})
source_group("Assets/Textures" FILES ${Textures})

add_executable(
   ${PROJECT_NAME}
   ${ProjectSources}
   ${ShaderSources}
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
set_source_files_properties(${copiedShaders} PROPERTIES GENERATED TRUE)
set_source_files_properties(${copiedTextures} PROPERTIES GENERATED TRUE)
target_sources(
   ${PROJECT_NAME} PRIVATE
   ${copiedShaders}
   ${copiedTextures}
)
