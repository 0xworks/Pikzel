#pragma once

#include "Core.h"
#include "Window.h"
#include "Pikzel/Events/ApplicationEvents.h"
#include "Pikzel/Events/WindowEvents.h"

#include <chrono>
#include <memory>

namespace Pikzel {

   class Application {
   public:
      Application(const Window::Settings& settings = {});

      virtual ~Application() = default;

      virtual void Run();

      void Exit();

      std::chrono::steady_clock::time_point GetTime();

   protected:
      virtual void Update(const DeltaTime deltaTime);

      virtual void RenderBegin();
      virtual void Render();
      virtual void RenderEnd();

      virtual void OnWindowClose(const WindowCloseEvent& event);
      virtual void OnWindowResize(const WindowResizeEvent& event);

      Window& GetWindow();

   private:
      std::chrono::steady_clock::time_point m_AppTime = {};
      std::unique_ptr<Window> m_Window;
      bool m_Running = false;

   };


   // Must be defined in client
   extern std::unique_ptr<Pikzel::Application> CreateApplication(int argc, const char* argv[]);

}
