#pragma once

#include "spdlog/spdlog.h"

// On Windows, spdlog.h brings in Windows.h, which in turn defines some things which clash with our code.
// undef them here.
#undef MemoryBarrier
#undef CreateWindow


namespace Pikzel {

class Log {
public:

   static void Init();

   static spdlog::logger& GetCoreLogger();
   static spdlog::logger& GetAppLogger();

private:
   static std::shared_ptr<spdlog::logger> s_loggerCore;
   static std::shared_ptr<spdlog::logger> s_loggerApp;
};

}

#define CORE_LOG_TRACE(...)        ::Pikzel::Log::GetCoreLogger().trace(__VA_ARGS__)
#define CORE_LOG_INFO(...)         ::Pikzel::Log::GetCoreLogger().info(__VA_ARGS__)
#define CORE_LOG_WARN(...)         ::Pikzel::Log::GetCoreLogger().warn(__VA_ARGS__)
#define CORE_LOG_ERROR(...)        ::Pikzel::Log::GetCoreLogger().error(__VA_ARGS__)
#define CORE_LOG_FATAL(...)        ::Pikzel::Log::GetCoreLogger().critical(__VA_ARGS__)

#define LOG_TRACE(...)        ::Pikzel::Log::GetAppLogger().trace(__VA_ARGS__)
#define LOG_INFO(...)         ::Pikzel::Log::GetAppLogger().info(__VA_ARGS__)
#define LOG_WARN(...)         ::Pikzel::Log::GetAppLogger().warn(__VA_ARGS__)
#define LOG_ERROR(...)        ::Pikzel::Log::GetAppLogger().error(__VA_ARGS__)
#define LOG_FATAL(...)        ::Pikzel::Log::GetAppLogger().critical(__VA_ARGS__)
