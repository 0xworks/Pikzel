#include "Pikzel/Core/Application.h"
#include "Pikzel/Core/Utility.h"
#include "Pikzel/Renderer/RenderCore.h"

#include <filesystem>


// Render a textured triangle in a window

class Textured final : public Pikzel::Application {
public:
   Textured(int argc, const char* argv[])
   : m_bindir(argv[0])
   {
      PKZL_PROFILE_FUNCTION();

      m_bindir.remove_filename();

      CreateVertexBuffer();
      CreateIndexBuffer();
      CreateTextures();
      CreateShaderProgram();

      Pikzel::RenderCore::SetClearColor({0.2f, 0.3f, 0.3f, 1.0f});
   }


   virtual void Update(Pikzel::DeltaTime deltaTime) override {
      ;
   }


   virtual void Render() override {
      PKZL_PROFILE_FUNCTION();

      Pikzel::RenderCore::Clear();

      m_Texture->Bind(0);
      m_Shader->Bind();
      Pikzel::RenderCore::DrawIndexed(*m_VertexArray);
      m_Shader->Unbind();
   }


private:

   void CreateVertexBuffer() {
      float vertices[] = {
          -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
           0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
           0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f
      };

      m_VertexBuffer = Pikzel::RenderCore::CreateVertexBuffer(vertices, sizeof(vertices));
      m_VertexBuffer->SetLayout({
         { Pikzel::ShaderDataType::Float3, "aPos" },
         { Pikzel::ShaderDataType::Float3, "aColor" },
         { Pikzel::ShaderDataType::Float2, "aTexCoord" }
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


   void CreateTextures() {
      m_Texture = Pikzel::RenderCore::CreateTexture2D(m_bindir / "Assets/Textures/Container.jpg");
   }


   void CreateShaderProgram() {
      auto vertShaderCode = Pikzel::ReadFile(m_bindir / "Assets/Shaders/Textured.vert", /*readAsBinary=*/false);
      auto fragShaderCode = Pikzel::ReadFile(m_bindir / "Assets/Shaders/Textured.frag", /*readAsBinary=*/false);
      m_Shader = Pikzel::RenderCore::CreateShader(vertShaderCode, fragShaderCode);
   }


private:
   std::filesystem::path m_bindir;
   std::shared_ptr<Pikzel::VertexBuffer> m_VertexBuffer;
   std::shared_ptr<Pikzel::IndexBuffer> m_IndexBuffer;
   std::unique_ptr<Pikzel::VertexArray> m_VertexArray;
   std::unique_ptr<Pikzel::Texture2D> m_Texture;
   std::unique_ptr<Pikzel::Shader> m_Shader;

};


std::unique_ptr<Pikzel::Application> Pikzel::CreateApplication(int argc, const char* argv[]) {
   PKZL_PROFILE_FUNCTION();
   PKZL_LOG_INFO(APP_DESCRIPTION);
   PKZL_LOG_INFO("Linked against {0} {1}", PKZL_DESCRIPTION, PKZL_VERSION);
#ifdef PKZL_DEBUG
   PKZL_LOG_INFO("DEBUG build");
#endif

   return std::make_unique<Textured>(argc, argv);
}
