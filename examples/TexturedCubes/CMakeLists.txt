add_executable(TexturedCubes)

target_sources(
   TexturedCubes
   PRIVATE
      Main.cpp
)

pikzel_add_shader(TexturedCubes Cube.slang ENTRY_POINTS VertexMain FragmentMain)

target_link_libraries(TexturedCubes PRIVATE Pikzel::Pikzel)

pikzel_set_compile_options(TexturedCubes)
set_target_properties(TexturedCubes PROPERTIES FOLDER "Examples")
