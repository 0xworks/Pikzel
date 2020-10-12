#include "Pikzel/Core/Application.h"
#include "Pikzel/Core/Utility.h"
#include "Pikzel/Input/Input.h"
#include "Pikzel/Renderer/RenderCore.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <filesystem>

class LightingMaps final : public Pikzel::Application {
public:
   LightingMaps(int argc, const char* argv[])
   : Pikzel::Application {argc, argv, {.Title = "Lighting Maps Demo", .ClearColor = {0.1f, 0.1f, 0.1f, 1.0f}}}
   , m_bindir {argv[0]}
   , m_Input {GetWindow()}
   {
      m_bindir.remove_filename();
      CreateVertexBuffer();
      CreateIndexBuffer();
      CreateTextures();
      CreateUniformBuffers();
      CreatePipelines();

      m_Projection = glm::perspective(glm::radians(45.0f), static_cast<float>(GetWindow().GetWidth()) / static_cast<float>(GetWindow().GetHeight()), 0.1f, 100.0f);
   }


   virtual void Update(const Pikzel::DeltaTime deltaTime) override {
      PKZL_PROFILE_FUNCTION();

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
      PKZL_PROFILE_FUNCTION();

      glm::mat4 projView = m_Projection * glm::lookAt(m_CameraPos, m_CameraPos + m_CameraDirection, m_CameraUp);

      Pikzel::GraphicsContext& gc = GetWindow().GetGraphicsContext();
      {
         gc.Bind(*m_PipelineLight);
         glm::mat4 model = glm::scale(glm::translate(glm::identity<glm::mat4>(), glm::vec3 {m_LightPos}), {0.2f, 0.2f, 0.2f});
         gc.PushConstant("constants.mvp"_hs, projView * model);
         gc.PushConstant("constants.lightColor"_hs, m_LightColor);
         gc.DrawIndexed(*m_VertexBuffer, *m_IndexBuffer);
      }
      {
         gc.Bind(*m_PipelineLighting);
         gc.Bind(*m_DiffuseTexture, "diffuseMap"_hs);
         gc.Bind(*m_SpecularTexture, "specularMap"_hs);
         gc.Bind(*m_MaterialBuffer, "Materials"_hs);
         gc.Bind(*m_LightBuffer, "Lights"_hs);

         glm::mat4 model = glm::identity<glm::mat4>();
         glm::mat4 modelInvTrans = glm::mat4(glm::transpose(glm::inverse(glm::mat3(model))));
         gc.PushConstant("constants.vp"_hs, projView);
         gc.PushConstant("constants.model"_hs, model);
         gc.PushConstant("constants.modelInvTrans"_hs, modelInvTrans);
         gc.PushConstant("constants.viewPos"_hs, m_CameraPos);

         gc.DrawIndexed(*m_VertexBuffer, *m_IndexBuffer);
      }
   }


private:

   struct Vertex {
      glm::vec3 Pos;
      glm::vec3 Normal;
      glm::vec2 TexCoord;
   };

   void CreateVertexBuffer() {
      Vertex vertices[] = {
         {.Pos{-0.5f, -0.5f, -0.5f}, .Normal{ 0.0f,  0.0f, -1.0f}, .TexCoord{0.0f, 0.0f}},
         {.Pos{ 0.5f, -0.5f, -0.5f}, .Normal{ 0.0f,  0.0f, -1.0f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{ 0.5f,  0.5f, -0.5f}, .Normal{ 0.0f,  0.0f, -1.0f}, .TexCoord{1.0f, 1.0f}},
         {.Pos{ 0.5f,  0.5f, -0.5f}, .Normal{ 0.0f,  0.0f, -1.0f}, .TexCoord{1.0f, 1.0f}},
         {.Pos{-0.5f,  0.5f, -0.5f}, .Normal{ 0.0f,  0.0f, -1.0f}, .TexCoord{0.0f, 1.0f}},
         {.Pos{-0.5f, -0.5f, -0.5f}, .Normal{ 0.0f,  0.0f, -1.0f}, .TexCoord{0.0f, 0.0f}},

         {.Pos{-0.5f, -0.5f,  0.5f}, .Normal{ 0.0f,  0.0f,  1.0f}, .TexCoord{0.0f, 0.0f}},
         {.Pos{ 0.5f, -0.5f,  0.5f}, .Normal{ 0.0f,  0.0f,  1.0f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{ 0.5f,  0.5f,  0.5f}, .Normal{ 0.0f,  0.0f,  1.0f}, .TexCoord{1.0f, 1.0f}},
         {.Pos{ 0.5f,  0.5f,  0.5f}, .Normal{ 0.0f,  0.0f,  1.0f}, .TexCoord{1.0f, 1.0f}},
         {.Pos{-0.5f,  0.5f,  0.5f}, .Normal{ 0.0f,  0.0f,  1.0f}, .TexCoord{0.0f, 1.0f}},
         {.Pos{-0.5f, -0.5f,  0.5f}, .Normal{ 0.0f,  0.0f,  1.0f}, .TexCoord{0.0f, 0.0f}},

         {.Pos{-0.5f,  0.5f,  0.5f}, .Normal{-1.0f,  0.0f,  0.0f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{-0.5f,  0.5f, -0.5f}, .Normal{-1.0f,  0.0f,  0.0f}, .TexCoord{1.0f, 1.0f}},
         {.Pos{-0.5f, -0.5f, -0.5f}, .Normal{-1.0f,  0.0f,  0.0f}, .TexCoord{0.0f, 1.0f}},
         {.Pos{-0.5f, -0.5f, -0.5f}, .Normal{-1.0f,  0.0f,  0.0f}, .TexCoord{0.0f, 1.0f}},
         {.Pos{-0.5f, -0.5f,  0.5f}, .Normal{-1.0f,  0.0f,  0.0f}, .TexCoord{0.0f, 0.0f}},
         {.Pos{-0.5f,  0.5f,  0.5f}, .Normal{-1.0f,  0.0f,  0.0f}, .TexCoord{1.0f, 0.0f}},

         {.Pos{ 0.5f,  0.5f,  0.5f}, .Normal{ 1.0f,  0.0f,  0.0f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{ 0.5f,  0.5f, -0.5f}, .Normal{ 1.0f,  0.0f,  0.0f}, .TexCoord{1.0f, 1.0f}},
         {.Pos{ 0.5f, -0.5f, -0.5f}, .Normal{ 1.0f,  0.0f,  0.0f}, .TexCoord{0.0f, 1.0f}},
         {.Pos{ 0.5f, -0.5f, -0.5f}, .Normal{ 1.0f,  0.0f,  0.0f}, .TexCoord{0.0f, 1.0f}},
         {.Pos{ 0.5f, -0.5f,  0.5f}, .Normal{ 1.0f,  0.0f,  0.0f}, .TexCoord{0.0f, 0.0f}},
         {.Pos{ 0.5f,  0.5f,  0.5f}, .Normal{ 1.0f,  0.0f,  0.0f}, .TexCoord{1.0f, 0.0f}},

         {.Pos{-0.5f, -0.5f, -0.5f}, .Normal{ 0.0f, -1.0f,  0.0f}, .TexCoord{0.0f, 1.0f}},
         {.Pos{ 0.5f, -0.5f, -0.5f}, .Normal{ 0.0f, -1.0f,  0.0f}, .TexCoord{1.0f, 1.0f}},
         {.Pos{ 0.5f, -0.5f,  0.5f}, .Normal{ 0.0f, -1.0f,  0.0f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{ 0.5f, -0.5f,  0.5f}, .Normal{ 0.0f, -1.0f,  0.0f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{-0.5f, -0.5f,  0.5f}, .Normal{ 0.0f, -1.0f,  0.0f}, .TexCoord{0.0f, 0.0f}},
         {.Pos{-0.5f, -0.5f, -0.5f}, .Normal{ 0.0f, -1.0f,  0.0f}, .TexCoord{0.0f, 1.0f}},

         {.Pos{-0.5f,  0.5f, -0.5f}, .Normal{ 0.0f,  1.0f,  0.0f}, .TexCoord{0.0f, 1.0f}},
         {.Pos{ 0.5f,  0.5f, -0.5f}, .Normal{ 0.0f,  1.0f,  0.0f}, .TexCoord{1.0f, 1.0f}},
         {.Pos{ 0.5f,  0.5f,  0.5f}, .Normal{ 0.0f,  1.0f,  0.0f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{ 0.5f,  0.5f,  0.5f}, .Normal{ 0.0f,  1.0f,  0.0f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{-0.5f,  0.5f,  0.5f}, .Normal{ 0.0f,  1.0f,  0.0f}, .TexCoord{0.0f, 0.0f}},
         {.Pos{-0.5f,  0.5f, -0.5f}, .Normal{ 0.0f,  1.0f,  0.0f}, .TexCoord{0.0f, 1.0f}}
      };

      m_VertexBuffer = Pikzel::RenderCore::CreateVertexBuffer(sizeof(vertices), vertices);
      m_VertexBuffer->SetLayout({
         { "inPos",      Pikzel::DataType::Vec3 },
         { "inNormal",   Pikzel::DataType::Vec3 },
         { "inTexCoord", Pikzel::DataType::Vec2 },
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
      m_DiffuseTexture = Pikzel::RenderCore::CreateTexture2D(m_bindir / "Assets/Textures/Diffuse.png");
      m_SpecularTexture = Pikzel::RenderCore::CreateTexture2D(m_bindir / "Assets/Textures/Specular.png");
   }


   struct Material {
      alignas(4)  float Shininess;
   };


   struct Light {
      alignas(16) glm::vec3 Position;
      alignas(16) glm::vec3 Ambient;
      alignas(16) glm::vec3 Diffuse;
      alignas(16) glm::vec3 Specular;
   };

   constexpr static glm::vec3 m_LightPos = {1.2f, 1.0f, 2.0f};
   constexpr static glm::vec3 m_LightColor = {1.0f, 1.0f, 1.0f};

   void CreateUniformBuffers() {
      Material materials[] = {
         {.Shininess{32.0f}}
      };
      m_MaterialBuffer = Pikzel::RenderCore::CreateUniformBuffer(sizeof(materials), materials);

      Light lights[] = {
         {.Position{m_LightPos}, .Ambient{0.1f, 0.1f, 0.1f}, .Diffuse{m_LightColor}, .Specular{m_LightColor}}
      };
      m_LightBuffer = Pikzel::RenderCore::CreateUniformBuffer(sizeof(lights), lights);
   }


   void CreatePipelines() {
      m_PipelineLight = GetWindow().GetGraphicsContext().CreatePipeline({
         m_VertexBuffer->GetLayout(),
         {
            { Pikzel::ShaderType::Vertex, m_bindir / "Assets/Shaders/Light.vert.spv" },
            { Pikzel::ShaderType::Fragment, m_bindir / "Assets/Shaders/Light.frag.spv" }
         }
      });
      m_PipelineLighting = GetWindow().GetGraphicsContext().CreatePipeline({
         m_VertexBuffer->GetLayout(),
         {
            { Pikzel::ShaderType::Vertex, m_bindir / "Assets/Shaders/Lighting.vert.spv" },
            { Pikzel::ShaderType::Fragment, m_bindir / "Assets/Shaders/Lighting.frag.spv" }
         }
      });
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

   std::unique_ptr<Pikzel::VertexBuffer> m_VertexBuffer;
   std::unique_ptr<Pikzel::IndexBuffer> m_IndexBuffer;
   std::unique_ptr<Pikzel::Texture2D> m_DiffuseTexture;
   std::unique_ptr<Pikzel::Texture2D> m_SpecularTexture;
   std::unique_ptr<Pikzel::UniformBuffer> m_MaterialBuffer;
   std::unique_ptr<Pikzel::UniformBuffer> m_LightBuffer;
   std::unique_ptr<Pikzel::Pipeline> m_PipelineLight;
   std::unique_ptr<Pikzel::Pipeline> m_PipelineLighting;

};


std::unique_ptr<Pikzel::Application> Pikzel::CreateApplication(int argc, const char* argv[]) {
   PKZL_LOG_INFO(APP_DESCRIPTION);
   PKZL_LOG_INFO("Linked against {0} {1}", PKZL_DESCRIPTION, PKZL_VERSION);
#ifdef PKZL_DEBUG
   PKZL_LOG_INFO("DEBUG build");
#endif

   return std::make_unique<LightingMaps>(argc, argv);
}
