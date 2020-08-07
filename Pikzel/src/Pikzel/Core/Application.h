#pragma once

#include "Core.h"

#include <chrono>
#include <memory>

namespace Pikzel {

using DeltaTime = std::chrono::steady_clock::duration;

class Application {

public:
   Application();
   virtual ~Application();

   virtual void Run();

protected:
   virtual void Update(DeltaTime deltaTime);
   virtual void Render();

protected:
   std::chrono::steady_clock::time_point m_AppTime = {};
   bool m_Running = false;

};


// Must be defined in client
extern std::unique_ptr<Pikzel::Application> CreateApplication(int argc, const char* argv[]);

}

