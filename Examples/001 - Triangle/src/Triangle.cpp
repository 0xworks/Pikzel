#include "Pikzel/Pikzel.h"
#include "Pikzel/Core/EntryPoint.h"


class Triangle final : public Pikzel::Application {
public:
   Triangle() {
      struct Vertex {
         glm::vec3 Pos;
         glm::vec3 Color;
      };

      Vertex vertices[] = {
         {.Pos{-0.5f, -0.5f, 0.0f}, .Color{Pikzel::sRGB{1.0f, 0.0f, 0.0f}}},
         {.Pos{ 0.5f, -0.5f, 0.0f}, .Color{Pikzel::sRGB{0.0f, 1.0f, 0.0f}}},
         {.Pos{ 0.0f,  0.5f, 0.0f}, .Color{Pikzel::sRGB{0.0f, 0.0f, 1.0f}}}
      };

      Pikzel::BufferLayout layout {
         {"inPos",   Pikzel::DataType::Vec3},
         {"inColor", Pikzel::DataType::Vec3}
      };
      m_VertexBuffer = Pikzel::RenderCore::CreateVertexBuffer(layout, sizeof(vertices), vertices);

      Pikzel::PipelineSettings settings {
         .Shaders = {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/Triangle.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/Triangle.frag.spv" }
         },
         .BufferLayout = layout,
      };
      m_Pipeline = GetWindow().GetGraphicsContext().CreatePipeline(settings);
   }


   virtual void Render() override {
      Pikzel::GraphicsContext& gc = GetWindow().GetGraphicsContext();
      gc.Bind(*m_Pipeline);
      gc.PushConstant("constants.mvp"_hs, m_Projection * m_View);
      gc.DrawTriangles(*m_VertexBuffer, 3);
   }

private:
   glm::mat4 m_View = glm::identity<glm::mat4>();
   glm::mat4 m_Projection = glm::identity<glm::mat4>();
   std::shared_ptr<Pikzel::VertexBuffer> m_VertexBuffer;
   std::unique_ptr<Pikzel::Pipeline> m_Pipeline;
};


std::unique_ptr<Pikzel::Application> CreateApplication(int argc, const char* argv[]) {
   return std::make_unique<Triangle>();
}
