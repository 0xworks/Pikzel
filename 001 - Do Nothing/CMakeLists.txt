cmake_minimum_required (VERSION 3.10)

project (
   "001DoNothing"
   VERSION 0.1
   DESCRIPTION "Pikzel Demo 001 - Do Nothing"
)

set(
   ProjectSources
   "src/DoNothing.cpp"
)

set(
   ProjectIncludes
)

set(
   ProjectLibs
   "Pikzel"
)


add_executable(
   ${PROJECT_NAME}
   ${ProjectSources}
)

set_target_properties(
   ${PROJECT_NAME} PROPERTIES
   VERSION ${PROJECT_VERSION}
   CXX_STANDARD 20
)

target_compile_definitions(
   ${PROJECT_NAME} PRIVATE
   APP_VERSION="${PROJECT_VERSION}"
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
