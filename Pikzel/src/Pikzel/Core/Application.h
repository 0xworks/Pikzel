#pragma once

#include "Core.h"
#include "Window.h"
#include "Pikzel/Events/ApplicationEvents.h"
#include "Pikzel/Events/WindowEvents.h"
#include "Pikzel/Renderer/RenderCore.h"

#include <chrono>
#include <filesystem>
#include <memory>

namespace Pikzel {

   class PKZL_API Application {
   public:
      Application(const Window::Settings& settings = {}, const RenderCore::API api = RenderCore::API::Undefined);

      virtual ~Application();

      virtual void Run();

      void Exit();

      std::chrono::steady_clock::time_point GetTime();

      const std::filesystem::path& GetRootDir() const;
      void SetRootDir(const std::filesystem::path& root);

      Window& GetWindow();

   public:
      static Application& Get();

   protected:
      virtual void Update(const DeltaTime deltaTime);

      virtual void RenderBegin();
      virtual void Render();
      virtual void RenderEnd();

      virtual void OnWindowClose(const WindowCloseEvent& event);
      virtual void OnWindowResize(const WindowResizeEvent& event);

   private:
      std::filesystem::path m_root;
      std::chrono::steady_clock::time_point m_AppTime = {};
      std::unique_ptr<Window> m_Window;
      bool m_Running = false;

      inline static Application* s_TheApplication = nullptr;
   };
}
