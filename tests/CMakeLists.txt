# Unit tests, built with Catch2.
#
# Tests cover engine code that can run without a GPU or window, such as math and data
# layout. Run them with ctest from the build directory, or run PikzelTests directly.

add_executable(PikzelTests)

target_sources(
   PikzelTests
   PRIVATE
      Renderer/CameraTests.cpp
      Renderer/ShaderTypesTests.cpp
)

target_link_libraries(PikzelTests PRIVATE Pikzel::Pikzel Catch2::Catch2WithMain)

pikzel_set_compile_options(PikzelTests)
set_target_properties(PikzelTests PROPERTIES FOLDER "Tests")

include(Catch)
catch_discover_tests(PikzelTests)
