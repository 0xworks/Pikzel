#include "Pikzel/Core/Application.h"
#include "Pikzel/Core/Utility.h"
#include "Pikzel/Input/Input.h"
#include "Pikzel/Renderer/RenderCore.h"

#include <glm/gtc/matrix_transform.hpp>

#include <filesystem>

class Triangle final : public Pikzel::Application {
public:
   Triangle(int argc, const char* argv[])
   : Pikzel::Application {{.Title = "Triangle Demo", .ClearColor = {0.2f, 0.3f, 0.3f, 1.0f}}}
   , m_bindir {argv[0]}
   , m_Input {GetWindow()}
   {
      m_bindir.remove_filename();
      CreateVertexBuffer();
      CreateIndexBuffer();
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
      gc.DrawIndexed(*m_VertexBuffer, *m_IndexBuffer);
   }


private:

   struct Vertex {
      glm::vec3 Pos;
      glm::vec3 Color;
   };

   void CreateVertexBuffer() {
      Vertex vertices[] = {
         {.Pos{-0.5f, -0.5f, 0.0f}, .Color{1.0f, 0.0f, 0.0f}},
         {.Pos{ 0.5f, -0.5f, 0.0f}, .Color{0.0f, 1.0f, 0.0f}},
         {.Pos{ 0.0f,  0.5f, 0.0f}, .Color{0.0f, 0.0f, 1.0f}}
      };

      m_VertexBuffer = Pikzel::RenderCore::CreateVertexBuffer(sizeof(vertices), vertices);
      m_VertexBuffer->SetLayout({
         {"inPos",   Pikzel::DataType::Vec3},
         {"inColor", Pikzel::DataType::Vec3}
      });
   }


   void CreateIndexBuffer() {
      uint32_t indices[] = {
          0, 1, 2
      };

      m_IndexBuffer = Pikzel::RenderCore::CreateIndexBuffer(sizeof(indices) / sizeof(uint32_t), indices);
   }


   void CreatePipeline() {
      Pikzel::PipelineSettings settings {
         m_VertexBuffer->GetLayout(),
         {
            { Pikzel::ShaderType::Vertex, m_bindir / "Assets/Shaders/Triangle.vert.spv" },
            { Pikzel::ShaderType::Fragment, m_bindir / "Assets/Shaders/Triangle.frag.spv" }
         }
      };
      m_Pipeline = GetWindow().GetGraphicsContext().CreatePipeline(settings);
   }


private:
   Pikzel::Input m_Input;
   std::filesystem::path m_bindir;
   glm::mat4 m_Transform = glm::identity<glm::mat4>();
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
