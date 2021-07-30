#pragma once

#include "spdlog/spdlog.h"

// On Windows, spdlog.h brings in Windows.h, which in turn defines some things which clash with our code.
// undef them here.
#undef MemoryBarrier
#undef CreateWindow

#include <filesystem>


namespace fmt {

   template <>
   struct formatter<std::filesystem::path> : formatter<std::string_view> {

      template <typename FormatContext>
      auto format(const std::filesystem::path& path, FormatContext& ctx) {
         return formatter<std::string_view>::format(path.string().data(), ctx);
      }
   };

}


namespace std {

   template <>
   struct formatter<filesystem::path> : formatter<string_view> {

      template <typename FormatContext>
      auto format(const filesystem::path& path, FormatContext& ctx) {
         return formatter<string_view>::format(path.string().data(), ctx);
      }
   };

}

namespace Pikzel {

   class PKZL_API Log {
   public:

      static void Init(int argc, const char* argv[]);

      static spdlog::logger& GetCoreLogger();
      static spdlog::logger& GetAppLogger();

   private:
      static inline std::shared_ptr<spdlog::logger> s_loggerCore;
      static inline std::shared_ptr<spdlog::logger> s_loggerApp;
   };

}

#define PKZL_CORE_LOG_TRACE(...)  ::Pikzel::Log::GetCoreLogger().trace(__VA_ARGS__)
#define PKZL_CORE_LOG_INFO(...)   ::Pikzel::Log::GetCoreLogger().info(__VA_ARGS__)
#define PKZL_CORE_LOG_WARN(...)   ::Pikzel::Log::GetCoreLogger().warn(__VA_ARGS__)
#define PKZL_CORE_LOG_ERROR(...)  ::Pikzel::Log::GetCoreLogger().error(__VA_ARGS__)
#define PKZL_CORE_LOG_FATAL(...)  ::Pikzel::Log::GetCoreLogger().critical(__VA_ARGS__)

#define PKZL_LOG_TRACE(...)       ::Pikzel::Log::GetAppLogger().trace(__VA_ARGS__)
#define PKZL_LOG_INFO(...)        ::Pikzel::Log::GetAppLogger().info(__VA_ARGS__)
#define PKZL_LOG_WARN(...)        ::Pikzel::Log::GetAppLogger().warn(__VA_ARGS__)
#define PKZL_LOG_ERROR(...)       ::Pikzel::Log::GetAppLogger().error(__VA_ARGS__)
#define PKZL_LOG_FATAL(...)       ::Pikzel::Log::GetAppLogger().critical(__VA_ARGS__)
