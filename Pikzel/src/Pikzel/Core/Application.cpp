#include "pch.h"
#include "Application.h"
#include "Log.h"
#include "Pikzel/Renderer/Renderer.h"

namespace Pikzel {

   Application::Application() {
      Renderer::Init();
   }


   Application::~Application() {
      Renderer::Shutdown();
   }


   void Application::Run() {
      m_AppTime = std::chrono::steady_clock::now();
      m_Running = true;
      while (m_Running) {
         PKZL_PROFILE_FRAMEMARKER();

         const auto currentTime = std::chrono::steady_clock::now();
         Update(currentTime - m_AppTime);
         m_AppTime = currentTime;

         Render();
      }
   }


   void Application::Exit() {
      m_Running = false;
   }


   std::chrono::steady_clock::time_point Application::GetTime() {
      return m_AppTime;
   }


   void Application::Update(DeltaTime deltaTime) {
      Exit();
   }


   void Application::Render() {
   }


}
