# The Pikzel engine runtime library.
#
# Built as a static library: there is no DLL boundary to manage, so no export macros,
# and the linker can discard anything a given program does not use.
#
# Source files are listed explicitly (no globbing) so that adding or removing a file is
# a visible change, and CMake re-configures reliably when it happens.

add_library(Pikzel STATIC)
add_library(Pikzel::Pikzel ALIAS Pikzel)

target_sources(
   Pikzel
   PRIVATE
      Core/Log.h
      Core/Log.cpp
)

# Engine headers are included by their path from src, e.g. "Pikzel/Core/Log.h".
target_include_directories(Pikzel PUBLIC "${PROJECT_SOURCE_DIR}/src")

# spdlog is PUBLIC because the logging macros in Log.h expand to spdlog calls.
target_link_libraries(Pikzel PUBLIC spdlog::spdlog)

pikzel_set_compile_options(Pikzel)
set_target_properties(Pikzel PROPERTIES FOLDER "Engine")
