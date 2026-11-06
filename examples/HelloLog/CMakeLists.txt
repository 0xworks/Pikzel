add_executable(HelloLog)

target_sources(
   HelloLog
   PRIVATE
      Main.cpp
)

target_link_libraries(HelloLog PRIVATE Pikzel::Pikzel)

pikzel_set_compile_options(HelloLog)
set_target_properties(HelloLog PROPERTIES FOLDER "Examples")
