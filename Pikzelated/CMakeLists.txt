cmake_minimum_required (VERSION 3.10)

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

#add_custom_command(
#   TARGET ${PROJECT_NAME} POST_BUILD
#   COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/Assets ${CMAKE_CURRENT_BINARY_DIR}/Assets
#)
