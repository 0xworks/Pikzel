#include "Pikzel/Core/Application.h"
#include "Pikzel/Renderer/Renderer.h"

// This is a small test to make sure that the base class Pikzel::Application can be instantiated,
// and the resulting application does indeed do nothing.
//
// During CreateApplication() we print out a basic introductory message.
//
std::unique_ptr<Pikzel::Application> Pikzel::CreateApplication(int argc, const char* argv[]) {
   PKZL_PROFILE_FUNCTION();
   PKZL_LOG_INFO(APP_DESCRIPTION);
   PKZL_LOG_INFO("Linked against {0} {1}", PKZL_DESCRIPTION, PKZL_VERSION);
#ifdef PKZL_DEBUG
   PKZL_LOG_INFO("DEBUG build");
#endif

   auto app = std::make_unique<Pikzel::Application>();
   PKZL_LOG_INFO("Using {0} render API", to_string(Renderer::GetAPI()));
   return app;
}
