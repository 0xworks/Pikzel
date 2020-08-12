cmake_minimum_required (VERSION 3.10)

project (
   "Pikzelated"
   VERSION 0.1
   DESCRIPTION "Pikzelated Demo App"
)

set(
   ProjectSources
   "src/Pikzelated.cpp"
)

set(
   ProjectIncludes
)

set(
   ProjectLibs
   "Pikzel"
)

set(
   ConfigFiles
   "EditorImGui.ini"
)

set(
   FontFiles
   "assets/fonts/Cousine-Regular.ttf"
)

set(
   ModelFiles
)

set(
   ShaderHeaderFiles
)

set(
   ShaderSrcFiles
)

set(
   TextureFiles
)

copy_assets(ConfigFiles . CopiedConfig)
copy_assets(FontFiles assets/fonts CopiedFonts)
copy_assets(ModelFiles assets/models CopiedModels)
compile_shaders(ShaderSrcFiles ShaderHeaderFiles assets/shaders CompiledShaders)
copy_assets(TextureFiles assets/textures CopiedTextures)

set_source_files_properties(
   ${CopiedConfig}
   ${CopiedFonts}
   ${CopiedModels}
   ${CompiledShaders}
   ${CopiedTextures}
   ${CopiedFiles}
   PROPERTIES
   GENERATED TRUE
)

add_executable(
   ${PROJECT_NAME}
   ${ProjectSources}
   ${ConfigFiles}
   ${FontFiles}
   ${ModelFiles}
   ${ShaderHeaderFiles}
   ${ShaderSrcFiles}
   ${TextureFiles}
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
# This line is here to make the project depend on the listed files (so that cmake will then "build" (i.e. re-copy)
# them whenever if they are out-of-date (i.e. whenever the asset is changed).
# The "correct" way to do this is to add_custom_target() and then add_dependencies() on the custom target.
# However, I do not want to clutter up the project with a whole load of custom targets.
target_sources(
   ${PROJECT_NAME} PRIVATE
   ${CopiedConfig}
   ${CopiedFonts}
   ${CopiedModels}
   ${CompliedShaders}
   ${CopiedTextures}
)

