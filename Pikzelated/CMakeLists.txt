cmake_minimum_required (VERSION 3.10)

include("../CmakeMacros.txt")

project ("Pikzelated" VERSION 0.1 DESCRIPTION "Pikzelated Demo App")

# Pikzelated Application
set(
   ProjectSources
   "src/Pikzelated.cpp"
)

set(
   ProjectIncludes
   "src"
   "vendor/imgui"
)

set(
   ProjectLibs
   "Pikzel"
   "imgui"
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


# IMGui Library
# Source files per vendor/imgui/examples/example_glfw_opengl3
# Depends on an OpenGl Loader.
# We are using glad, this is autodetected via putting glad in the include path below
add_library(
   "imgui" STATIC
   "vendor/imgui/imgui.cpp"
   "vendor/imgui/imgui_draw.cpp"
   "vendor/imgui/imgui_widgets.cpp"
   "vendor/imgui/examples/imgui_impl_glfw.cpp"
   "vendor/imgui/examples/imgui_impl_opengl3.cpp"
)

target_include_directories(
   "imgui" PRIVATE
   "vendor/imgui"
)

# This is needed because Pikzel provides the OpenGL loader (glad) for ImGUI
target_link_libraries(
   "imgui" PRIVATE
   "Pikzel"
)