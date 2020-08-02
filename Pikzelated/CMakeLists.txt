cmake_minimum_required (VERSION 3.10)

include("../CmakeMacros.txt")

project ("Pikzelated" VERSION 0.1 DESCRIPTION "Pikzelated Demo App")

add_executable(
   ${PROJECT_NAME}
   "src/Pikzelated.cpp"
)

set_target_properties(
   ${PROJECT_NAME} PROPERTIES
   VERSION ${PROJECT_VERSION}
)

target_link_libraries(
   ${PROJECT_NAME} PRIVATE
   Pikzel
)

set(
   config_files
   "EditorImGui.ini"
)

copy_assets(config_files . copied_files)

set_source_files_properties(
   ${copied_files} PROPERTIES
   GENERATED TRUE
)

# These arent really "source" files.
# This line is here to make target depend on the listed files (so that cmake will then build them)
# The "correct" way to do this is to add_custom_target() and then add_dependencies() on the custom target.
# I do not want to clutter up the project with a whole load of custom targets, however.
target_sources(
   ${PROJECT_NAME} PRIVATE
   ${copied_files}
)


#add_custom_command(
#   TARGET ${PROJECT_NAME} POST_BUILD
#   COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/Assets ${CMAKE_CURRENT_BINARY_DIR}/Assets
#)
