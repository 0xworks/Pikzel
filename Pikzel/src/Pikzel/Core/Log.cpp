#include "Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace Pikzel {

   std::shared_ptr<spdlog::logger> Log::s_loggerCore;
   std::shared_ptr<spdlog::logger> Log::s_loggerApp;

   void Log::Init() {
      spdlog::set_pattern("%^[%T] %n: %v%$");
      s_loggerCore = spdlog::stdout_color_mt("CORE");
      s_loggerApp = spdlog::stdout_color_mt("APP");
      s_loggerCore->set_level(spdlog::level::trace);
      s_loggerApp->set_level(spdlog::level::trace);
   }


   spdlog::logger& Log::GetCoreLogger() {
      return *s_loggerCore;
   }

   spdlog::logger& Log::GetAppLogger() {
      return *s_loggerApp;
   }

}
