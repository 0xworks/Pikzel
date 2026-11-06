#include "Pikzel/Core/Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>

#include <cassert>
#include <memory>

namespace Pikzel::Log {

namespace {

// The logger objects are owned here. spdlog also keeps them in its own registry (so
// they can be looked up by name), which is why Shutdown() calls spdlog::shutdown() to
// release that second reference.
std::shared_ptr<spdlog::logger> s_CoreLogger;
std::shared_ptr<spdlog::logger> s_AppLogger;

}


void Init() {
   assert(!s_CoreLogger && !s_AppLogger && "Log::Init() called twice");

   // Both loggers write to standard output through one shared sink, so that their
   // messages interleave in the order they were logged.
   const auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
   consoleSink->set_pattern("%^[%T] %n: %v%$");

   s_CoreLogger = std::make_shared<spdlog::logger>("CORE", consoleSink);
   s_AppLogger = std::make_shared<spdlog::logger>("APP", consoleSink);

#ifdef NDEBUG
   const auto level = spdlog::level::info;
#else
   const auto level = spdlog::level::debug;
#endif
   s_CoreLogger->set_level(level);
   s_AppLogger->set_level(level);

   // Errors are flushed immediately so that they are not lost if the process then crashes.
   s_CoreLogger->flush_on(spdlog::level::err);
   s_AppLogger->flush_on(spdlog::level::err);

   spdlog::register_logger(s_CoreLogger);
   spdlog::register_logger(s_AppLogger);

   PKZL_CORE_LOG_DEBUG("Logging initialised");
}


void Shutdown() {
   if (s_CoreLogger) {
      PKZL_CORE_LOG_DEBUG("Logging shut down");
   }
   s_AppLogger.reset();
   s_CoreLogger.reset();
   spdlog::shutdown(); // flushes and drops every registered logger
}


spdlog::logger& GetCoreLogger() {
   assert(s_CoreLogger && "Log::Init() must be called before logging");
   return *s_CoreLogger;
}


spdlog::logger& GetAppLogger() {
   assert(s_AppLogger && "Log::Init() must be called before logging");
   return *s_AppLogger;
}

}
