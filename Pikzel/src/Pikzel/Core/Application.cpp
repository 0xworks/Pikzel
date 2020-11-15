#include "Application.h"
#include "Log.h"
#include "Pikzel/Events/EventDispatcher.h"
#include "Pikzel/Scene/ModelSerializer.h"

namespace Pikzel {

   static void ShowUsage(std::string name) {
      std::filesystem::path argv0 {name};
      PKZL_CORE_LOG_INFO("Usage: {0} [options]", argv0.filename().string());
      PKZL_CORE_LOG_INFO("\tOptions:");
      PKZL_CORE_LOG_INFO("\t\t-h,--help\t\tShow this help message");
      PKZL_CORE_LOG_INFO("\t\t-api [vk | gl]\t\tSpecify OpenGL or Vulkan rendering API, respectively");
      PKZL_CORE_LOG_INFO("\tThe rendering API is a hint only, and may be overridden by the application.");
      PKZL_CORE_LOG_INFO("\tGenerally, if no api is specified, then OpenGL will be chosen.");
   }


   Application::Application(const int argc, const char* argv[], const Window::Settings& settings, const RenderCore::API api)
   : m_argc {argc}
   , m_argv {argv}
   {
      if (s_TheApplication) {
         throw std::runtime_error {"Attempted to initialize application more than once"};
      }
      s_TheApplication = this;

      if (api != RenderCore::API::None) {
         RenderCore::SetAPI(api);
      } else {
         // parse command line for API
         std::vector <std::string> sources;
         std::string destination;
         for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if ((arg == "-h") || (arg == "--help")) {
               ShowUsage(argv[0]);
            } else if (arg == "-api") {
               if (i + 1 >= argc) {
                  PKZL_CORE_LOG_ERROR("Missing render api");
                  ShowUsage(argv[0]);
               } else {
                  std::string api = argv[i + 1];
                  if (api == "gl") {
                     RenderCore::SetAPI(RenderCore::API::OpenGL);
                  } else if (api == "vk") {
                     RenderCore::SetAPI(RenderCore::API::Vulkan);
                  } else {
                     PKZL_CORE_LOG_ERROR("Unknown render api");
                     ShowUsage(argv[0]);
                  }
               }
            }
         }
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
      ModelSerializer::ClearTextureCache();
   }


   void Application::Run() {
      m_AppTime = std::chrono::steady_clock::now();
      m_Running = true;
      while (m_Running) {
         PKZL_PROFILE_FRAMEMARKER();

         const auto currentTime = std::chrono::steady_clock::now();
         {
            PKZL_PROFILE_SCOPE("EventDispatcher::Send<UpdateEvent>");
            EventDispatcher::Send<UpdateEvent>(currentTime - m_AppTime);
         }
         Update(currentTime - m_AppTime);
         m_AppTime = currentTime;

         RenderBegin();
         Render();
         RenderEnd();
      }
   }


   void Application::Exit() {
      m_Running = false;
   }


   std::chrono::steady_clock::time_point Application::GetTime() {
      return m_AppTime;
   }


   int Application::GetArgC() const {
      return m_argc;
   }


   const char** Application::GetArgV() const {
      return m_argv;
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
      if (event.Sender == m_Window->GetNativeWindow()) {
         Exit();
      }
   }


   void Application::OnWindowResize(const Pikzel::WindowResizeEvent& event) {
      if (event.Sender == m_Window->GetNativeWindow()) {
         Pikzel::RenderCore::SetViewport(0, 0, event.Width, event.Height);
      }
   }


   Pikzel::Window& Application::GetWindow() {
      PKZL_CORE_ASSERT(m_Window, "Accessing null Window!");
      return *m_Window;
   }

}
