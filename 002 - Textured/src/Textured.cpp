#include "Pikzel/Pikzel.h"
#include "Pikzel/Core/EntryPoint.h"

class Textured final : public Pikzel::Application {
public:
   Textured()
   : Pikzel::Application {{.Title = APP_DESCRIPTION, .ClearColor = Pikzel::sRGB{0.2f, 0.3f, 0.3f}}}
   , m_Input {GetWindow()}
   {
      CreateVertexBuffer();
      CreateTextures();
      CreatePipeline();
   }


   virtual void Update(const Pikzel::DeltaTime deltaTime) override {
      float dx = m_Input.GetAxis("X"_hs) * deltaTime.count();
      float dy = m_Input.GetAxis("Z"_hs) * deltaTime.count();
      m_Transform = glm::translate(m_Transform, {dx, dy, 0.0f});
   }


   virtual void Render() override {
      Pikzel::GraphicsContext& gc = GetWindow().GetGraphicsContext();
      Pikzel::GCBinder bindPipeline {gc, *m_Pipeline};
      Pikzel::GCBinder bindTexture {gc, *m_Texture, "uTexture"_hs};   // Technically, we don't have to bind the texture every frame (once it's bound, it stays bound).  However, it makes things much easier if we do it this way, and so that's how it is for now.

      //Pikzel::GCBinder bindUniformBuffer {gc, *m_UBO, "ubo"_hs};    // Ditto uniform buffer objects
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

      m_VertexBuffer = Pikzel::RenderCore::CreateVertexBuffer(sizeof(vertices), vertices);
      m_VertexBuffer->SetLayout({
         { "inPos",      Pikzel::DataType::Vec3 },
         { "inColor",    Pikzel::DataType::Vec3 },
         { "inTexCoord", Pikzel::DataType::Vec2 }
      });
   }


   void CreateTextures() {
      m_Texture = Pikzel::RenderCore::CreateTexture2D("Assets/" APP_NAME "/Textures/Container.jpg");
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
   Pikzel::Input m_Input;
   glm::mat4 m_Transform = glm::identity<glm::mat4>();
   std::shared_ptr<Pikzel::VertexBuffer> m_VertexBuffer;
   std::unique_ptr<Pikzel::Texture2D> m_Texture;
   std::unique_ptr<Pikzel::Pipeline> m_Pipeline;

};


std::unique_ptr<Pikzel::Application> CreateApplication(int argc, const char* argv[]) {
   return std::make_unique<Textured>();
}
