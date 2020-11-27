#include "Pikzel/Pikzel.h"
#include "Pikzel/Core/EntryPoint.h"

class Triangle final : public Pikzel::Application {
public:
   Triangle()
   : Pikzel::Application {{.Title = APP_DESCRIPTION, .ClearColor = Pikzel::sRGB{0.5f, 0.5f, 0.5f}}}
   , m_Input {GetWindow()}
   {
      CreateVertexBuffer();
      CreatePipeline();
   }


   virtual void Update(const Pikzel::DeltaTime deltaTime) override {
      float dx = m_Input.GetAxis("X"_hs) * deltaTime.count();
      float dy = m_Input.GetAxis("Z"_hs) * deltaTime.count();
      m_Transform = glm::translate(m_Transform, {dx, dy, 0.0f});
   }


   virtual void Render() override {
      Pikzel::GraphicsContext& gc = GetWindow().GetGraphicsContext();
      Pikzel::GCBinder bind {gc, *m_Pipeline};
      gc.PushConstant("constants.mvp"_hs, m_Transform);
      gc.DrawTriangles(*m_VertexBuffer, 3);
   }


private:

   struct Vertex {
      glm::vec3 Pos;
      glm::vec3 Color;
   };

   void CreateVertexBuffer() {
      Vertex vertices[] = {
         {.Pos{-0.5f, -0.5f, 0.0f}, .Color{Pikzel::sRGB{1.0f, 0.0f, 0.0f}}},
         {.Pos{ 0.5f, -0.5f, 0.0f}, .Color{Pikzel::sRGB{0.0f, 1.0f, 0.0f}}},
         {.Pos{ 0.0f,  0.5f, 0.0f}, .Color{Pikzel::sRGB{0.0f, 0.0f, 1.0f}}}
      };

      m_VertexBuffer = Pikzel::RenderCore::CreateVertexBuffer(sizeof(vertices), vertices);
      m_VertexBuffer->SetLayout({
         {"inPos",   Pikzel::DataType::Vec3},
         {"inColor", Pikzel::DataType::Vec3}
      });
   }


   void CreatePipeline() {
      Pikzel::PipelineSettings settings {
         m_VertexBuffer->GetLayout(),
         {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/Triangle.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/Triangle.frag.spv" }
         }
      };
      m_Pipeline = GetWindow().GetGraphicsContext().CreatePipeline(settings);
   }


private:
   Pikzel::Input m_Input;
   glm::mat4 m_Transform = glm::identity<glm::mat4>();
   std::shared_ptr<Pikzel::VertexBuffer> m_VertexBuffer;
   std::unique_ptr<Pikzel::Pipeline> m_Pipeline;
};


std::unique_ptr<Pikzel::Application> CreateApplication(int argc, const char* argv[]) {
   return std::make_unique<Triangle>();
}
