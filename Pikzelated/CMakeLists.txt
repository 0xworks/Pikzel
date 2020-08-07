cmake_minimum_required (VERSION 3.10)

include("../CMakeMacros.txt")

project ("Pikzelated" VERSION 0.1 DESCRIPTION "Pikzelated Demo App")

# Pikzelated Application
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

copy_assets(ConfigFiles . CopiedFiles)

set_source_files_properties(
   ${CopiedFiles} PROPERTIES
   GENERATED TRUE
)

source_group("Config" FILES ${ConfigFiles})

add_executable(
   ${PROJECT_NAME}
   ${ProjectSources}
   ${ConfigFiles}
)

set_target_properties(
   ${PROJECT_NAME} PROPERTIES
   VERSION ${PROJECT_VERSION}
   CXX_STANDARD 20
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
   ${CopiedFiles}
)
