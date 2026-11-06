#pragma once

#include <spdlog/spdlog.h>

// Engine logging, built on spdlog.
//
// There are two loggers: "CORE" for messages from the engine itself and "APP" for
// messages from code built on top of the engine (games, examples, the editor).
// Keeping them separate makes it easy to see where a message came from, and lets each
// be filtered independently.
//
// Logging is one of the few deliberately process-wide services in the engine. Almost
// every subsystem needs it, and threading a logger reference through every constructor
// would add noise without adding clarity. The loggers are created once by Log::Init()
// and destroyed by Log::Shutdown(); everything else in the engine avoids global state.
//
// spdlog's "_mt" loggers are used, so logging from multiple threads is safe once Init()
// has returned. Init() and Shutdown() themselves must be called from the main thread
// while no other threads are logging.
namespace Pikzel::Log {

// Creates the loggers. Must be called before any logging macro is used.
void Init();

// Flushes and destroys the loggers. Logging after this point is a programmer error.
void Shutdown();

spdlog::logger& GetCoreLogger();
spdlog::logger& GetAppLogger();

}

// Macros (rather than functions) so that logging can later be compiled out of particular
// build configurations without touching call sites. Arguments use fmt format syntax:
//    PKZL_LOG_INFO("Loaded {} meshes in {:.2f} ms", meshCount, milliseconds);

#define PKZL_CORE_LOG_TRACE(...) ::Pikzel::Log::GetCoreLogger().trace(__VA_ARGS__)
#define PKZL_CORE_LOG_DEBUG(...) ::Pikzel::Log::GetCoreLogger().debug(__VA_ARGS__)
#define PKZL_CORE_LOG_INFO(...)  ::Pikzel::Log::GetCoreLogger().info(__VA_ARGS__)
#define PKZL_CORE_LOG_WARN(...)  ::Pikzel::Log::GetCoreLogger().warn(__VA_ARGS__)
#define PKZL_CORE_LOG_ERROR(...) ::Pikzel::Log::GetCoreLogger().error(__VA_ARGS__)
#define PKZL_CORE_LOG_FATAL(...) ::Pikzel::Log::GetCoreLogger().critical(__VA_ARGS__)

#define PKZL_LOG_TRACE(...) ::Pikzel::Log::GetAppLogger().trace(__VA_ARGS__)
#define PKZL_LOG_DEBUG(...) ::Pikzel::Log::GetAppLogger().debug(__VA_ARGS__)
#define PKZL_LOG_INFO(...)  ::Pikzel::Log::GetAppLogger().info(__VA_ARGS__)
#define PKZL_LOG_WARN(...)  ::Pikzel::Log::GetAppLogger().warn(__VA_ARGS__)
#define PKZL_LOG_ERROR(...) ::Pikzel::Log::GetAppLogger().error(__VA_ARGS__)
#define PKZL_LOG_FATAL(...) ::Pikzel::Log::GetAppLogger().critical(__VA_ARGS__)
