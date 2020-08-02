#include "Pikzel/Core/Application.h"

#include <imgui.h>
#include <imgui_internal.h>

class Pikzelated : public Pikzel::Application {
public:
   Pikzelated() {
      PKZL_PROFILE_FUNCTION();
   }

   ~Pikzelated() {}
};


std::unique_ptr<Pikzelated::Application> CreateApplication(int argc, const char* argv[]) {
   PKZL_PROFILE_FUNCTION();
   return std::make_unique<Pikzelated>();
}
