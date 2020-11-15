#pragma once

#include "Application.h"
#include "Pikzel/Events/EventDispatcher.h"

// Must be defined in client
std::unique_ptr<Pikzel::Application> CreateApplication(int argc, const char* argv[]);


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
   try {
      Pikzel::EventDispatcher::Init();
      CreateApplication(ARGC, ARGV)->Run();

      // The EventDispatcher needs to be destructed before unloading the RenderCore api shared library
      // (because the render core can (and does) register event classes with the dispatcher.
      // If you unload the shared library, then these classes are no longer defined, and destructing
      // the event dispatcher will then crash.
      Pikzel::EventDispatcher::DeInit();
      Pikzel::RenderCore::DeInit();
   } catch (std::exception err) {
      PKZL_CORE_LOG_FATAL(err.what());
      return EXIT_FAILURE;
   }
   return EXIT_SUCCESS;
}
