#include "Pikzel/Core/Application.h"
#include "Pikzel/Core/Utility.h"
#include "Pikzel/Renderer/RenderCore.h"

#include <glm/gtc/matrix_transform.hpp>

#include <filesystem>

// Render a textured triangle in a window
class Textured final : public Pikzel::Application {
public:
   Textured(int argc, const char* argv[])
   : Pikzel::Application {Pikzel::WindowSettings{"Texture Demo", 1280, 720, {0.2f, 0.3f, 0.3f, 1.0f}}}
   , m_bindir {argv[0]}
   {
      m_bindir.remove_filename();
      CreateVertexBuffer();
      CreateIndexBuffer();
      CreateTextures();
      CreatePipeline();
   }


   virtual void Update(Pikzel::DeltaTime deltaTime) override {
      ;
   }


   virtual void Render() override {
      Pikzel::GraphicsContext& gc = GetWindow().GetGraphicsContext();
      Pikzel::GCBinder bindPipeline {gc, *m_Pipeline};
      Pikzel::GCBinder bindTexture {gc, *m_Texture, "uTexture"_hs};   // Technically, we don't have to bind the texture every frame (once it's bound, it stays bound).  However, it makes things much easier if we do it this way, and so that's how it is for now.

      //Pikzel::GCBinder bindUniformBuffer {gc, *m_UBO, "ubo"_hs};    // Ditto uniform buffer objects
      gc.PushConstant("constants.mvp"_hs, glm::identity<glm::mat4>());
      gc.DrawIndexed(*m_VertexBuffer, *m_IndexBuffer);
   }


private:
   struct Vertex {
      glm::vec3 Pos;
      glm::vec3 Color;
      glm::vec2 TexCoord;
   };

   void CreateVertexBuffer() {
      Vertex vertices[] = {
         {.Pos{-0.5f, -0.5f, 0.0f}, .Color{1.0f, 0.0f, 0.0f}, .TexCoord{0.0f, 0.0f}},
         {.Pos{ 0.5f, -0.5f, 0.0f}, .Color{0.0f, 1.0f, 0.0f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{ 0.0f,  0.5f, 0.0f}, .Color{0.0f, 0.0f, 1.0f}, .TexCoord{0.5f, 1.0f}}
      };

      m_VertexBuffer = Pikzel::RenderCore::CreateVertexBuffer(sizeof(vertices), vertices);
      m_VertexBuffer->SetLayout({
         { "inPos",      Pikzel::DataType::Vec3 },
         { "inColor",    Pikzel::DataType::Vec3 },
         { "inTexCoord", Pikzel::DataType::Vec2 }
      });
   }


   void CreateIndexBuffer() {
      uint32_t indices[] = {
          0, 1, 2
      };

      m_IndexBuffer = Pikzel::RenderCore::CreateIndexBuffer(sizeof(indices) / sizeof(uint32_t), indices);
   }


   void CreateTextures() {
      m_Texture = Pikzel::RenderCore::CreateTexture2D(m_bindir / "Assets/Textures/Container.jpg");
   }


   void CreatePipeline() {
      Pikzel::PipelineSettings settings {
         *m_VertexBuffer,
         {
            { Pikzel::ShaderType::Vertex, m_bindir / "Assets/Shaders/Textured.vert.spv" },
            { Pikzel::ShaderType::Fragment, m_bindir / "Assets/Shaders/Textured.frag.spv" }
         }
      };
      m_Pipeline = GetWindow().GetGraphicsContext().CreatePipeline(settings);
   }


private:
   std::filesystem::path m_bindir;
   std::shared_ptr<Pikzel::VertexBuffer> m_VertexBuffer;
   std::shared_ptr<Pikzel::IndexBuffer> m_IndexBuffer;
   std::unique_ptr<Pikzel::Texture2D> m_Texture;
   std::unique_ptr<Pikzel::Pipeline> m_Pipeline;

};


std::unique_ptr<Pikzel::Application> Pikzel::CreateApplication(int argc, const char* argv[]) {
   PKZL_LOG_INFO(APP_DESCRIPTION);
   PKZL_LOG_INFO("Linked against {0} {1}", PKZL_DESCRIPTION, PKZL_VERSION);
#ifdef PKZL_DEBUG
   PKZL_LOG_INFO("DEBUG build");
#endif

   return std::make_unique<Textured>(argc, argv);
}
