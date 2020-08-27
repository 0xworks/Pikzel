#include "Pikzel/Core/Application.h"
#include "Pikzel/Core/Utility.h"
#include "Pikzel/Core/Window.h"
#include "Pikzel/Events/EventDispatcher.h"
#include "Pikzel/Events/WindowEvents.h"
#include "Pikzel/Renderer/RenderCore.h"

#include <filesystem>


// Render a triangle in a window

class Triangle final : public Pikzel::Application {
public:
   Triangle(int argc, const char* argv[])
   : m_bindir(argv[0])
   {
      PKZL_PROFILE_FUNCTION();

      m_bindir.remove_filename();

      m_Window = Pikzel::Window::Create({APP_DESCRIPTION});
      Pikzel::EventDispatcher::Connect<Pikzel::WindowCloseEvent, &Triangle::OnWindowClose>(*this);
      Pikzel::EventDispatcher::Connect<Pikzel::WindowResizeEvent, &Triangle::OnWindowResize>(*this);

      Pikzel::RenderCore::SetViewport(0, 0, m_Window->GetWidth(), m_Window->GetHeight());

      CreateVertexBuffer();
      CreateShaderProgram();
   }


   virtual void Update(Pikzel::DeltaTime deltaTime) override {
      ;
   }


   virtual void Render() override {
      PKZL_PROFILE_FUNCTION();

      m_Window->BeginFrame();

      Pikzel::RenderCore::SetClearColor({0.2f, 0.3f, 0.3f, 1.0f});
      Pikzel::RenderCore::Clear();

      //
      // TODO:  render triangle here...
      //

      m_Window->EndFrame();

   }


private:
   void OnWindowClose(const Pikzel::WindowCloseEvent& event) {
      if (event.Sender == m_Window->GetNativeWindow()) {
         Exit();
      }
   }


   void OnWindowResize(const Pikzel::WindowResizeEvent& event) {
      if (event.Sender == m_Window->GetNativeWindow()) {
         Pikzel::RenderCore::SetViewport(0, 0, event.Width, event.Height);
      }
   }


   void CreateVertexBuffer() {
      float vertices[] = {
          -0.5f, -0.5f, 0.0f,
           0.5f, -0.5f, 0.0f,
           0.0f,  0.5f, 0.0f
      };

      m_VertexBuffer = Pikzel::RenderCore::CreateVertexBuffer(vertices, sizeof(vertices));
   }


   void CreateShaderProgram() {
      auto vertShaderCode = Pikzel::ReadFile(m_bindir / "Assets/Shaders/Triangle.vert", /*readAsBinary=*/false);
      auto fragShaderCode = Pikzel::ReadFile(m_bindir / "Assets/Shaders/Triangle.frag", /*readAsBinary=*/false);
      m_Shader = Pikzel::RenderCore::CreateShader(vertShaderCode, fragShaderCode);
   }


private:
   std::filesystem::path m_bindir;
   std::unique_ptr<Pikzel::Window> m_Window;
   std::unique_ptr<Pikzel::VertexBuffer> m_VertexBuffer;
   std::unique_ptr<Pikzel::Shader> m_Shader;

};


std::unique_ptr<Pikzel::Application> Pikzel::CreateApplication(int argc, const char* argv[]) {
   PKZL_PROFILE_FUNCTION();
   PKZL_LOG_INFO(APP_DESCRIPTION);
   PKZL_LOG_INFO("Linked against {0} {1}", PKZL_DESCRIPTION, PKZL_VERSION);
#ifdef PKZL_DEBUG
   PKZL_LOG_INFO("DEBUG build");
#endif

   return std::make_unique<Triangle>(argc, argv);
}
