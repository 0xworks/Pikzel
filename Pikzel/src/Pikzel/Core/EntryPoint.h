#pragma once

#include "Application.h"
#include "Pikzel/Events/EventDispatcher.h"
#include "Pikzel/Renderer/RenderCore.h"

// Must be defined in client
std::unique_ptr<Pikzel::Application> CreateApplication(int argc, const char* argv[]);

namespace Pikzel {
   static void ShowUsage(std::string name) {
      std::filesystem::path argv0 {name};
      PKZL_CORE_LOG_INFO("Usage: {0} [options]", argv0.filename().string());
      PKZL_CORE_LOG_INFO("\tOptions:");
      PKZL_CORE_LOG_INFO("\t\t-h,--help\t\tShow this help message");
      PKZL_CORE_LOG_INFO("\t\t-api [vk | gl]\t\tSpecify OpenGL or Vulkan rendering API, respectively");
      PKZL_CORE_LOG_INFO("\tThe rendering API is a hint only, and may be overridden by the application.");
      PKZL_CORE_LOG_INFO("\tGenerally, if no api is specified, then OpenGL will be chosen.");
   }
}

// Note: Windowed app also also requires WIN32 added to the add_executable command in CMakeLists.txt
#if defined(PKZL_PLATFORM_WINDOWS) && defined(PKZL_BUILD_WINDOWED_APP)
#define ARGC __argc
#define ARGV (const char**)__argv

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
#else
#define ARGC argc
#define ARGV argv

int main(int argc, const char* argv[]) {
#endif
   Pikzel::Log::Init();
   Pikzel::EventDispatcher::Init();

   // parse command line for render API
   for (int i = 1; i < argc; ++i) {
      std::string arg = argv[i];
      if ((arg == "-h") || (arg == "--help")) {
         Pikzel::ShowUsage(argv[0]);
      } else if (arg == "-api") {
         if (i + 1 >= argc) {
            PKZL_CORE_LOG_ERROR("Missing render api");
            Pikzel::ShowUsage(argv[0]);
         } else {
            std::string api = argv[i + 1];
            if (api == "gl") {
               Pikzel::RenderCore::SetAPI(Pikzel::RenderCore::API::OpenGL);
            } else if (api == "vk") {
               Pikzel::RenderCore::SetAPI(Pikzel::RenderCore::API::Vulkan);
            } else {
               PKZL_CORE_LOG_ERROR("Unknown render api");
               Pikzel::ShowUsage(argv[0]);
            }
         }
      }
   }

   try {
      std::unique_ptr<Pikzel::Application> app = CreateApplication(ARGC, ARGV);

      if (app->GetRootDir().empty()) {
         std::filesystem::path root(argv[0]);
         root.remove_filename();
         app->SetRootDir(root);
      }

      app->Run();

      app.reset();

   } catch (const std::exception& err) {
      PKZL_CORE_LOG_FATAL(err.what());
      return EXIT_FAILURE;
   }

   // The EventDispatcher needs to be destructed before unloading the RenderCore api shared library
   // (because the render core can (and does) register event classes with the dispatcher.
   // If you unload the shared library, then these classes are no longer defined, and destructing
   // the event dispatcher will then crash.
   Pikzel::EventDispatcher::DeInit();
   Pikzel::RenderCore::DeInit();
   return EXIT_SUCCESS;
}
