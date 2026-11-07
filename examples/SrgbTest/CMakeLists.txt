add_executable(SrgbTest)

target_sources(
   SrgbTest
   PRIVATE
      Main.cpp
)

pikzel_add_shader(SrgbTest SrgbTest.slang ENTRY_POINTS VertexMain FragmentMain)

target_link_libraries(SrgbTest PRIVATE Pikzel::Pikzel)

pikzel_set_compile_options(SrgbTest)
set_target_properties(SrgbTest PROPERTIES FOLDER "Examples")
