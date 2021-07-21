#include "Log.h"

#include <spdlog/cfg/argv.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace Pikzel {

   void Log::Init(int argc, const char* argv[]) {
      spdlog::set_pattern("%^[%T] %n: %v%$");
      s_loggerCore = spdlog::stdout_color_mt("CORE");
      s_loggerApp = spdlog::stdout_color_mt("APP");
      s_loggerCore->set_level(spdlog::level::info);
      s_loggerApp->set_level(spdlog::level::info);
      spdlog::cfg::load_argv_levels(argc, argv);
   }


   spdlog::logger& Log::GetCoreLogger() {
      return *s_loggerCore;
   }

   spdlog::logger& Log::GetAppLogger() {
      return *s_loggerApp;
   }

}
