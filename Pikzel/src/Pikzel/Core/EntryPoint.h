#pragma once

#include "Application.h"

// Must be defined in client
std::unique_ptr<Pikzel::Application> CreateApplication(int argc, const char* argv[]);

// Later, we might consider making it a windows app instead of console...
// but until such time as we have a good place to send stdout to, lets
// just keep it as console
//
// Note: making it a windows app also requires WIN32 added to the add_executable command
//       in CMakeLists.txt
#ifdef PKZL_PLATFORM_WINDOWS
#if 0
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

   Pikzel::Log::Init();
   try {
      CreateApplication(__argc, (const char**)__argv)->Run();
   } catch (std::exception err) {
      CORE_LOG_FATAL(err.what());
      return EXIT_FAILURE;
   }
   return EXIT_SUCCESS;
}
#endif
#endif

int main(int argc, const char* argv[]) {
   Pikzel::Log::Init();
   try {
      CreateApplication(argc, argv)->Run();
   } catch (std::exception err) {
      PKZL_CORE_LOG_FATAL(err.what());
      return EXIT_FAILURE;
   }
   return EXIT_SUCCESS;
}
