#include "Pikzel/Core/Application.h"
#include "Pikzel/Core/Utility.h"
#include "Pikzel/Renderer/RenderCore.h"

#include <filesystem>


// Render a triangle in a window

class Triangle final : public Pikzel::Application {
public:
   Triangle(int argc, const char* argv[])
   : Pikzel::Application { Pikzel::WindowSettings{"Triangle Demo", 1280, 720, {0.2f, 0.3f, 0.3f, 1.0f}} }
   , m_bindir {argv[0]}
   {
      m_bindir.remove_filename();
      CreateVertexBuffer();
      CreateIndexBuffer();
      CreatePipeline();
   }


   virtual void Update(Pikzel::DeltaTime deltaTime) override {
      ;
   }


   virtual void Render() override {
      Pikzel::GraphicsContext& gc = GetWindow().GetGraphicsContext();
      Pikzel::GCBinder bind {gc, *m_Pipeline};
      gc.DrawIndexed(*m_VertexBuffer, *m_IndexBuffer);
   }


private:

   void CreateVertexBuffer() {
      float vertices[] = {
          -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
           0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
           0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f
      };

      m_VertexBuffer = Pikzel::RenderCore::CreateVertexBuffer(vertices, sizeof(vertices));
      m_VertexBuffer->SetLayout({
         { Pikzel::DataType::Float3, "aPos" },
         { Pikzel::DataType::Float3, "aColor" }
      });

   }


   void CreateIndexBuffer() {
      uint32_t indices[] = {
          0, 1, 2
      };

      m_IndexBuffer = Pikzel::RenderCore::CreateIndexBuffer(indices, sizeof(indices) / sizeof(uint32_t));
   }


   void CreatePipeline() {
      Pikzel::PipelineSettings settings {
         *m_VertexBuffer,
         {
            { Pikzel::ShaderType::Vertex, m_bindir / "Assets/Shaders/Triangle.vert" },
            { Pikzel::ShaderType::Fragment, m_bindir / "Assets/Shaders/Triangle.frag" }
         }
      };
      m_Pipeline = GetWindow().GetGraphicsContext().CreatePipeline(settings);
   }


private:
   std::filesystem::path m_bindir;
   std::shared_ptr<Pikzel::VertexBuffer> m_VertexBuffer;
   std::shared_ptr<Pikzel::IndexBuffer> m_IndexBuffer;
   std::unique_ptr<Pikzel::Pipeline> m_Pipeline;
};


std::unique_ptr<Pikzel::Application> Pikzel::CreateApplication(int argc, const char* argv[]) {
   PKZL_LOG_INFO(APP_DESCRIPTION);
   PKZL_LOG_INFO("Linked against {0} {1}", PKZL_DESCRIPTION, PKZL_VERSION);
#ifdef PKZL_DEBUG
   PKZL_LOG_INFO("DEBUG build");
#endif

   return std::make_unique<Triangle>(argc, argv);
}
