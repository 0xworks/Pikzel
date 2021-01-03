#include "Pikzel/Pikzel.h"
#include "Pikzel/Core/EntryPoint.h"

class Textured final : public Pikzel::Application {
public:
   Textured()
   : Pikzel::Application {{.Title = APP_DESCRIPTION, .ClearColor = Pikzel::sRGB{0.2f, 0.3f, 0.3f}}}
   {
      CreateVertexBuffer();
      CreateTextures();
      CreatePipeline();
   }


   virtual void Render() override {
      Pikzel::GraphicsContext& gc = GetWindow().GetGraphicsContext();
      gc.Bind(*m_Pipeline);
      gc.Bind(*m_Texture, "uTexture"_hs);   // Technically, we don't have to bind the texture every frame (once it's bound, it stays bound).
      gc.PushConstant("constants.mvp"_hs, glm::identity<glm::mat4>());
      gc.DrawTriangles(*m_VertexBuffer, 3);
   }


private:

   struct Vertex {
      glm::vec3 Pos;
      glm::vec3 Color;
      glm::vec2 TexCoord;
   };

   void CreateVertexBuffer() {
      Vertex vertices[] = {
         {.Pos{-0.5f, -0.5f, 0.0f}, .Color{Pikzel::sRGB{1.0f, 0.0f, 0.0f}}, .TexCoord{0.0f, 0.0f}},
         {.Pos{ 0.5f, -0.5f, 0.0f}, .Color{Pikzel::sRGB{0.0f, 1.0f, 0.0f}}, .TexCoord{1.0f, 0.0f}},
         {.Pos{ 0.0f,  0.5f, 0.0f}, .Color{Pikzel::sRGB{0.0f, 0.0f, 1.0f}}, .TexCoord{0.5f, 1.0f}}
      };

      Pikzel::BufferLayout layout = {
         {"inPos",       Pikzel::DataType::Vec3},
         {"inColor",     Pikzel::DataType::Vec3},
         {"inTexCoords", Pikzel::DataType::Vec2},
      };
      m_VertexBuffer = Pikzel::RenderCore::CreateVertexBuffer(layout, sizeof(vertices), vertices);
   }


   void CreateTextures() {
      // There are many different parameters for creating a texture.
      // The simplest is to provide the path to an image file asset.
      // This creates a 2D texture and will automatically set the width, height, texture format, and mip map levels.
      m_Texture = Pikzel::RenderCore::CreateTexture({.Path = "Assets/" APP_NAME "/Textures/Container.jpg"});
   }


   void CreatePipeline() {
      Pikzel::PipelineSettings settings {
         m_VertexBuffer->GetLayout(),
         {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/Textured.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/Textured.frag.spv" }
         }
      };
      m_Pipeline = GetWindow().GetGraphicsContext().CreatePipeline(settings);
   }


private:
   glm::mat4 m_Transform = glm::identity<glm::mat4>();
   std::shared_ptr<Pikzel::VertexBuffer> m_VertexBuffer;
   std::unique_ptr<Pikzel::Texture> m_Texture;
   std::unique_ptr<Pikzel::Pipeline> m_Pipeline;

};


std::unique_ptr<Pikzel::Application> CreateApplication(int argc, const char* argv[]) {
   return std::make_unique<Textured>();
}
