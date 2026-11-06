# Third-party dependencies.
#
# Dependencies are fetched from source with FetchContent and built as part of the project.
# This keeps the build self-contained: a fresh clone needs only CMake and a C++23 compiler,
# with no package manager to install or configure. The trade-off is a slower first
# configure, and every dependency must be pinned to an exact release tag here.
#
# Add new dependencies sparingly, and only with a clear justification (see AGENTS.md).

include(FetchContent)

# spdlog: logging.
#
# spdlog is configured to format with std::format instead of its bundled copy of fmt, so
# the engine uses a single formatting library. A custom type then needs only one
# std::formatter specialisation to work with both logging and std::format elsewhere.
# The cost is fmt-only extras such as named arguments, fmt::join and spdlog's bin_to_hex.
# spdlog exports the resulting SPDLOG_USE_STD_FORMAT definition as a PUBLIC compile
# definition, so everything linking spdlog agrees on the setting.
set(SPDLOG_USE_STD_FORMAT ON)

FetchContent_Declare(
   spdlog
   GIT_REPOSITORY https://github.com/gabime/spdlog.git
   GIT_TAG v1.15.3
   GIT_SHALLOW TRUE
   SYSTEM
)

FetchContent_MakeAvailable(spdlog)

# Keep third-party targets out of the way in IDE solution views.
if(TARGET spdlog)
   set_target_properties(spdlog PROPERTIES FOLDER "ThirdParty")
endif()
