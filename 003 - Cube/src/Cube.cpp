#include "Pikzel/Core/Application.h"
#include "Pikzel/Core/Utility.h"
#include "Pikzel/Input/Input.h"
#include "Pikzel/Renderer/RenderCore.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <filesystem>

class Cube final : public Pikzel::Application {
public:
   Cube(int argc, const char* argv[])
   : Pikzel::Application {argc, argv, {.Title = APP_DESCRIPTION, .ClearColor = {0.2f, 0.3f, 0.3f, 1.0f}}}
   , m_bindir {argv[0]}
   , m_Input {GetWindow()}
   {
      m_bindir.remove_filename();
      CreateVertexBuffer();
      CreateIndexBuffer();
      CreateTextures();
      CreatePipeline();

      m_Projection = glm::perspective(glm::radians(45.0f), static_cast<float>(GetWindow().GetWidth()) / static_cast<float>(GetWindow().GetHeight()), 0.1f, 100.0f);
      
   }


   virtual void Update(const Pikzel::DeltaTime deltaTime) override {
      constexpr float cameraSpeed = 2.5f;

      if (m_Input.IsKeyPressed(Pikzel::KeyCode::Escape)) {
         Exit();
      }

      float dx = m_Input.GetAxis("X"_hs) * deltaTime.count() * cameraSpeed;
      float dy = m_Input.GetAxis("Y"_hs) * deltaTime.count() * cameraSpeed;
      float dz = m_Input.GetAxis("Z"_hs) * deltaTime.count() * cameraSpeed;

      m_CameraPos += dx * glm::normalize(glm::cross(m_CameraDirection, m_CameraUp));
      m_CameraPos += dy * m_CameraUp;
      m_CameraPos += dz * m_CameraDirection;

      if (m_Input.IsMouseButtonPressed(Pikzel::MouseButton::Right)) {
         float dYawRadians = glm::radians(m_Input.GetAxis("MouseX"_hs) * deltaTime.count() * cameraSpeed);
         float dPitchRadians = glm::radians(m_Input.GetAxis("MouseY"_hs) * deltaTime.count() * cameraSpeed);

         m_CameraDirection = glm::rotateY(m_CameraDirection, -dYawRadians);
         m_CameraUp = glm::rotateY(m_CameraUp, dYawRadians);

         glm::vec3 right = glm::cross(m_CameraDirection, m_CameraUp);
         m_CameraDirection = glm::rotate(m_CameraDirection, dPitchRadians, right);
         m_CameraUp = glm::rotate(m_CameraUp, dPitchRadians, right);
      }
   }


   virtual void Render() override {
      Pikzel::GraphicsContext& gc = GetWindow().GetGraphicsContext();
      Pikzel::GCBinder bindPipeline {gc, *m_Pipeline};
      Pikzel::GCBinder bindTexture {gc, *m_Texture, "uTexture"_hs};

      glm::mat4 projView = m_Projection * glm::lookAt(m_CameraPos, m_CameraPos + m_CameraDirection, m_CameraUp);

      for (int i = 0; i < 10; ++i) {
         glm::mat4 model = glm::translate(glm::identity<glm::mat4>(), m_CubePositions[i]);
         model = glm::rotate(model, glm::radians(20.0f * i), {1.0f, 0.3f, 0.5f});

         gc.PushConstant("constants.mvp"_hs, projView * model);
         gc.DrawIndexed(*m_VertexBuffer, *m_IndexBuffer);
      }
   }


private:

   struct Vertex {
      glm::vec3 Pos;
      glm::vec2 TexCoord;
   };

   void CreateVertexBuffer() {
      Vertex vertices[] = {
         {.Pos{-0.5f, -0.5f, -0.5f}, .TexCoord{0.0f, 0.0f}},
         {.Pos{ 0.5f,  0.5f, -0.5f}, .TexCoord{1.0f, 1.0f}},
         {.Pos{ 0.5f, -0.5f, -0.5f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{ 0.5f,  0.5f, -0.5f}, .TexCoord{1.0f, 1.0f}},
         {.Pos{-0.5f, -0.5f, -0.5f}, .TexCoord{0.0f, 0.0f}},
         {.Pos{-0.5f,  0.5f, -0.5f}, .TexCoord{0.0f, 1.0f}},

         {.Pos{-0.5f, -0.5f,  0.5f}, .TexCoord{0.0f, 0.0f}},
         {.Pos{ 0.5f, -0.5f,  0.5f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{ 0.5f,  0.5f,  0.5f}, .TexCoord{1.0f, 1.0f}},
         {.Pos{ 0.5f,  0.5f,  0.5f}, .TexCoord{1.0f, 1.0f}},
         {.Pos{-0.5f,  0.5f,  0.5f}, .TexCoord{0.0f, 1.0f}},
         {.Pos{-0.5f, -0.5f,  0.5f}, .TexCoord{0.0f, 0.0f}},

         {.Pos{-0.5f,  0.5f,  0.5f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{-0.5f,  0.5f, -0.5f}, .TexCoord{1.0f, 1.0f}},
         {.Pos{-0.5f, -0.5f, -0.5f}, .TexCoord{0.0f, 1.0f}},
         {.Pos{-0.5f, -0.5f, -0.5f}, .TexCoord{0.0f, 1.0f}},
         {.Pos{-0.5f, -0.5f,  0.5f}, .TexCoord{0.0f, 0.0f}},
         {.Pos{-0.5f,  0.5f,  0.5f}, .TexCoord{1.0f, 0.0f}},

         {.Pos{ 0.5f,  0.5f,  0.5f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{ 0.5f, -0.5f, -0.5f}, .TexCoord{0.0f, 1.0f}},
         {.Pos{ 0.5f,  0.5f, -0.5f}, .TexCoord{1.0f, 1.0f}},
         {.Pos{ 0.5f, -0.5f, -0.5f}, .TexCoord{0.0f, 1.0f}},
         {.Pos{ 0.5f,  0.5f,  0.5f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{ 0.5f, -0.5f,  0.5f}, .TexCoord{0.0f, 0.0f}},

         {.Pos{-0.5f, -0.5f, -0.5f}, .TexCoord{0.0f, 1.0f}},
         {.Pos{ 0.5f, -0.5f, -0.5f}, .TexCoord{1.0f, 1.0f}},
         {.Pos{ 0.5f, -0.5f,  0.5f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{ 0.5f, -0.5f,  0.5f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{-0.5f, -0.5f,  0.5f}, .TexCoord{0.0f, 0.0f}},
         {.Pos{-0.5f, -0.5f, -0.5f}, .TexCoord{0.0f, 1.0f}},

         {.Pos{-0.5f,  0.5f, -0.5f}, .TexCoord{0.0f, 1.0f}},
         {.Pos{ 0.5f,  0.5f,  0.5f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{ 0.5f,  0.5f, -0.5f}, .TexCoord{1.0f, 1.0f}},
         {.Pos{ 0.5f,  0.5f,  0.5f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{-0.5f,  0.5f, -0.5f}, .TexCoord{0.0f, 1.0f}},
         {.Pos{-0.5f,  0.5f,  0.5f}, .TexCoord{0.0f, 0.0f}}
      };

      m_VertexBuffer = Pikzel::RenderCore::CreateVertexBuffer(sizeof(vertices), vertices);
      m_VertexBuffer->SetLayout({
         { "inPos",      Pikzel::DataType::Vec3 },
         { "inTexCoord", Pikzel::DataType::Vec2 }
      });
   }


   void CreateIndexBuffer() {
      uint32_t indices[] = {
         0,1,2,
         3,4,5,
         6,7,8,
         9,10,11,
         12,13,14,
         15,16,17,
         18,19,20,
         21,22,23,
         24,25,26,
         27,28,29,
         30,31,32,
         33,34,35
      };

      m_IndexBuffer = Pikzel::RenderCore::CreateIndexBuffer(sizeof(indices) / sizeof(uint32_t), indices);
   }


   void CreateTextures() {
      m_Texture = Pikzel::RenderCore::CreateTexture2D(m_bindir / "Assets/Textures/Container.jpg");
   }


   void CreatePipeline() {
      Pikzel::PipelineSettings settings {
         m_VertexBuffer->GetLayout(),
         {
            { Pikzel::ShaderType::Vertex, m_bindir / "Assets/Shaders/Cube.vert.spv" },
            { Pikzel::ShaderType::Fragment, m_bindir / "Assets/Shaders/Cube.frag.spv" }
         }
      };
      m_Pipeline = GetWindow().GetGraphicsContext().CreatePipeline(settings);
   }


private:
   Pikzel::Input m_Input;
   std::filesystem::path m_bindir;

   glm::vec3 m_CameraPos {0.0f, 0.0f, 3.0f};
   glm::vec3 m_CameraDirection = {0.0f, 0.0f, -1.0f};
   glm::vec3 m_CameraUp = {0.0f, 1.0f, 0.0f};
   float m_CameraPitch = 0.0f;
   float m_CameraYaw = -90.0f;
   float m_CameraFoV = 45.0f;

   glm::mat4 m_Projection = glm::identity<glm::mat4>();

   std::vector<glm::vec3> m_CubePositions = {
       glm::vec3(0.0f,  0.0f,  0.0f),
       glm::vec3(2.0f,  5.0f, -15.0f),
       glm::vec3(-1.5f, -2.2f, -2.5f),
       glm::vec3(-3.8f, -2.0f, -12.3f),
       glm::vec3(2.4f, -0.4f, -3.5f),
       glm::vec3(-1.7f,  3.0f, -7.5f),
       glm::vec3(1.3f, -2.0f, -2.5f),
       glm::vec3(1.5f,  2.0f, -2.5f),
       glm::vec3(1.5f,  0.2f, -1.5f),
       glm::vec3(-1.3f,  1.0f, -1.5f)
   };

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

   return std::make_unique<Cube>(argc, argv);
}
