add_executable(HelloTriangle)

target_sources(
   HelloTriangle
   PRIVATE
      Main.cpp
)

pikzel_add_shader(HelloTriangle Triangle.slang ENTRY_POINTS VertexMain FragmentMain)

target_link_libraries(HelloTriangle PRIVATE Pikzel::Pikzel)

pikzel_set_compile_options(HelloTriangle)
set_target_properties(HelloTriangle PROPERTIES FOLDER "Examples")
