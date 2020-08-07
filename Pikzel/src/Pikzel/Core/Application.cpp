#include "pch.h"
#include "Application.h"
#include "Log.h"

namespace Pikzel {

Application::Application() {}


Application::~Application() {
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


void Application::Update(DeltaTime deltaTime) {
   m_Running = false;
}


void Application::Render() {
}


}