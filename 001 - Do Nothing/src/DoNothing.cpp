#include "Pikzel/Core/Application.h"

// This is a small test to make sure that the base class Pikzel::Application can be instantiated, and indeed does nothing.
// (i.e. it makes no assumptions about what the derived client application might be wanting to do - not even opening a window)
//
// A basic message _is_ printed to stdout
//
std::unique_ptr<Pikzel::Application> Pikzel::CreateApplication(int argc, const char* argv[]) {
   PKZL_PROFILE_FUNCTION();
   PKZL_CORE_LOG_INFO(APP_DESCRIPTION);
   PKZL_CORE_LOG_INFO("Linked against {0} {1}", PKZL_DESCRIPTION, PKZL_VERSION);
#ifdef PKZL_DEBUG
   PKZL_CORE_LOG_INFO("DEBUG build");
#endif
   return std::make_unique<Pikzel::Application>();
}
