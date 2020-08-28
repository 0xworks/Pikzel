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
      CreateIndexBuffer();
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

      m_Shader->Bind();
      Pikzel::RenderCore::DrawIndexed(*m_VertexArray);
      m_Shader->Unbind();

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
          -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
           0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
           0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f
      };

      m_VertexBuffer = Pikzel::RenderCore::CreateVertexBuffer(vertices, sizeof(vertices));
      m_VertexBuffer->SetLayout({
         { Pikzel::ShaderDataType::Float3, "aPos" },
         { Pikzel::ShaderDataType::Float3, "aColor" }
      });

      m_VertexArray = Pikzel::RenderCore::CreateVertexArray();
      m_VertexArray->AddVertexBuffer(m_VertexBuffer);

   }


   void CreateIndexBuffer() {
      uint32_t indices[] = {
          0, 1, 2
      };

      m_IndexBuffer = Pikzel::RenderCore::CreateIndexBuffer(indices, sizeof(indices) / sizeof(uint32_t));
      m_VertexArray->SetIndexBuffer(m_IndexBuffer);
   }


   void CreateShaderProgram() {
      auto vertShaderCode = Pikzel::ReadFile(m_bindir / "Assets/Shaders/Triangle.vert", /*readAsBinary=*/false);
      auto fragShaderCode = Pikzel::ReadFile(m_bindir / "Assets/Shaders/Triangle.frag", /*readAsBinary=*/false);
      m_Shader = Pikzel::RenderCore::CreateShader(vertShaderCode, fragShaderCode);
   }


private:
   std::filesystem::path m_bindir;
   glm::vec4 m_TriangleColor = {};
   Pikzel::DeltaTime m_TotalSeconds = {};
   std::unique_ptr<Pikzel::Window> m_Window;
   std::shared_ptr<Pikzel::VertexBuffer> m_VertexBuffer;
   std::shared_ptr<Pikzel::IndexBuffer> m_IndexBuffer;
   std::unique_ptr<Pikzel::VertexArray> m_VertexArray;
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
