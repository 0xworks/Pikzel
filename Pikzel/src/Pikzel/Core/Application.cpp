#include "Application.h"
#include "Log.h"
#include "Pikzel/Events/EventDispatcher.h"
#include "Pikzel/Scene/AssetCache.h"

namespace Pikzel {

   Application::Application(const RenderCore::API api)
   : Application{Window::Settings{}, api}
   {}
   
   Application::Application(const Window::Settings& settings, const RenderCore::API api)
   {
      if (s_TheApplication) {
         throw std::runtime_error {"Attempted to initialize application more than once"};
      }
      s_TheApplication = this;

      PKZL_LOG_INFO("{0} {1}", PKZL_DESCRIPTION, PKZL_VERSION);
#ifdef PKZL_DEBUG
      PKZL_LOG_INFO("DEBUG build");
#endif

      if (api != RenderCore::API::Undefined) {
         RenderCore::SetAPI(api);
      }

      // Every application has to have a window whether you like it or not.
      // You cannot, for example, initialize OpenGL rendering backend without a window.
      // A workaround for applications that just want to do "offline" rendering is
      // to create the window and then immediately hide it.
      m_Window = Pikzel::Window::Create(settings);
      EventDispatcher::Connect<WindowCloseEvent, &Application::OnWindowClose>(*this);
      EventDispatcher::Connect<WindowResizeEvent, &Application::OnWindowResize>(*this);
   }


   Application::~Application() {
      EventDispatcher::Disconnect<WindowCloseEvent, &Application::OnWindowClose>(*this);
      EventDispatcher::Disconnect<WindowResizeEvent, &Application::OnWindowResize>(*this);
      AssetCache::Clear();
   }


   void Application::Run() {
      m_AppTime = std::chrono::steady_clock::now();
      m_IsRunning = true;
      while (m_IsRunning) {
         PKZL_PROFILE_FRAMEMARKER();
         {
            PKZL_PROFILE_SCOPE("glfwPollEvents");
            glfwPollEvents();
         }

         if (!m_IsPaused) {
            const auto currentTime = std::chrono::steady_clock::now();
            {
               PKZL_PROFILE_SCOPE("EventDispatcher::Send<UpdateEvent>");
               EventDispatcher::Send<UpdateEvent>(currentTime - m_AppTime);
            }
            Update(currentTime - m_AppTime);
            m_AppTime = currentTime;
         }

         if (!m_IsMinimized) {
            RenderBegin();
            Render();
            RenderEnd();
         }
      }
   }


   void Application::Exit() {
      m_IsRunning = false;
   }


   std::chrono::steady_clock::time_point Application::GetTime() {
      return m_AppTime;
   }


   const std::filesystem::path& Application::GetRootDir() const {
      return m_root;
   }


   void Application::SetRootDir(const std::filesystem::path& root) {
      m_root = root;
   }


   Application& Application::Get() {
      return *s_TheApplication;
   }


   void Application::Update(DeltaTime) {}


   void Application::RenderBegin() {
      m_Window->BeginFrame();
   }


   void Application::Render() {
   }


   void Application::RenderEnd() {
      m_Window->EndFrame();
   }


   void Application::OnWindowClose(const Pikzel::WindowCloseEvent& event) {
      if (event.sender == m_Window->GetNativeWindow()) {
         Exit();
      }
   }


   void Application::OnWindowResize(const Pikzel::WindowResizeEvent& event) {
      if (event.sender == m_Window->GetNativeWindow()) {
         if ((event.width > 0) && (event.height > 0)) {
            Pikzel::RenderCore::SetViewport(0, 0, event.width, event.height);
            m_IsPaused = false;
            m_IsMinimized = false;
         } else {
            m_IsPaused = true;
            m_IsMinimized = true;
         }
      }
   }


   Pikzel::Window& Application::GetWindow() {
      PKZL_CORE_ASSERT(m_Window, "Accessing null Window!");
      return *m_Window;
   }

}
