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
      CreatePipeline();

      Pikzel::RenderCore::SetClearColor({0.2f, 0.3f, 0.3f, 1.0f});
   }


   virtual void Update(Pikzel::DeltaTime deltaTime) override {
      ;
   }


   virtual void Render() override {
      PKZL_PROFILE_FUNCTION();

      Pikzel::RenderCore::Clear();

      m_Texture->Bind(0);
      m_Pipeline->Bind();
      Pikzel::RenderCore::DrawIndexed(*m_VertexBuffer, *m_IndexBuffer);
      m_Pipeline->Unbind();
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
         { Pikzel::DataType::Float3, "aPos" },
         { Pikzel::DataType::Float3, "aColor" },
         { Pikzel::DataType::Float2, "aTexCoord" }
      });
   }


   void CreateIndexBuffer() {
      uint32_t indices[] = {
          0, 1, 2
      };

      m_IndexBuffer = Pikzel::RenderCore::CreateIndexBuffer(indices, sizeof(indices) / sizeof(uint32_t));
   }


   void CreateTextures() {
      m_Texture = Pikzel::RenderCore::CreateTexture2D(m_bindir / "Assets/Textures/Container.jpg");
   }


   void CreatePipeline() {
      Pikzel::PipelineSettings settings {
         *m_VertexBuffer,
         {
            { Pikzel::ShaderType::Vertex, Pikzel::ReadFile(m_bindir / "Assets/Shaders/Textured.vert", /*readAsBinary=*/false) },
            { Pikzel::ShaderType::Fragment, Pikzel::ReadFile(m_bindir / "Assets/Shaders/Textured.frag", /*readAsBinary=*/false) }
         }
      };
      m_Pipeline = Pikzel::RenderCore::CreatePipeline(GetWindow(), settings);
   }


private:
   std::filesystem::path m_bindir;
   std::shared_ptr<Pikzel::VertexBuffer> m_VertexBuffer;
   std::shared_ptr<Pikzel::IndexBuffer> m_IndexBuffer;
   std::unique_ptr<Pikzel::Texture2D> m_Texture;
   std::unique_ptr<Pikzel::Pipeline> m_Pipeline;

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
