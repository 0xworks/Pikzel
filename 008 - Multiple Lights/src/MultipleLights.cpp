#include "Camera.h"

#include "Pikzel/Core/Application.h"
#include "Pikzel/Core/Utility.h"
#include "Pikzel/Input/Input.h"
#include "Pikzel/Renderer/RenderCore.h"

#include <filesystem>

class MultipleLights final : public Pikzel::Application {
public:
   MultipleLights(int argc, const char* argv[])
   : Pikzel::Application {argc, argv, {.Title = "Multiple Lights Demo", .ClearColor = {0.05f, 0.05f, 0.05f, 1.0f}, .IsVSync = false}}
   , m_bindir {argv[0]}
   , m_Input {GetWindow()}
   {
      m_bindir.remove_filename();
      CreateVertexBuffer();
      CreateIndexBuffer();
      CreateTextures();
      CreateUniformBuffers();
      CreatePipelines();

      m_Camera.Projection = glm::perspective(m_Camera.FoVRadians, static_cast<float>(GetWindow().GetWidth()) / static_cast<float>(GetWindow().GetHeight()), 0.1f, 100.0f);
   }


   virtual void Update(const Pikzel::DeltaTime deltaTime) override {
      PKZL_PROFILE_FUNCTION();
      if (m_Input.IsKeyPressed(Pikzel::KeyCode::Escape)) {
         Exit();
      }
      m_Camera.Update(m_Input, deltaTime);
   }


   virtual void Render() override {
      PKZL_PROFILE_FUNCTION();

      glm::mat4 projView = m_Camera.Projection * glm::lookAt(m_Camera.Position, m_Camera.Position + m_Camera.Direction, m_Camera.UpVector);

      Pikzel::GraphicsContext& gc = GetWindow().GetGraphicsContext();
      {
         gc.Bind(*m_PipelineLight);
         for (const auto& pointLight : m_PointLights) {
            glm::mat4 model = glm::scale(glm::translate(glm::identity<glm::mat4>(), pointLight.Position), {0.2f, 0.2f, 0.2f});
            gc.PushConstant("constants.mvp"_hs, projView * model);
            gc.PushConstant("constants.lightColor"_hs, pointLight.Color);
            gc.DrawIndexed(*m_VertexBuffer, *m_IndexBuffer);
         }
      }
      {
         gc.Bind(*m_PipelineLighting);
         gc.Bind(*m_DiffuseTexture, "diffuseMap"_hs);
         gc.Bind(*m_SpecularTexture, "specularMap"_hs);
         gc.Bind(*m_MaterialBuffer, "UBOMaterials"_hs);
         gc.Bind(*m_DirectionalLightBuffer, "UBODirectionalLight"_hs);
         gc.Bind(*m_PointLightBuffer, "UBOPointLights"_hs);

         gc.PushConstant("constants.vp"_hs, projView);
         gc.PushConstant("constants.viewPos"_hs, m_Camera.Position);

         for (int i = 0; i < 10; ++i) {
            glm::mat4 model = glm::rotate(glm::translate(glm::identity<glm::mat4>(), m_CubePositions[i]), glm::radians(20.0f * i), glm::vec3 {1.0f, 0.3f, 0.5f});
            glm::mat4 modelInvTrans = glm::mat4(glm::transpose(glm::inverse(glm::mat3(model))));
            gc.PushConstant("constants.model"_hs, model);
            gc.PushConstant("constants.modelInvTrans"_hs, modelInvTrans);
            gc.DrawIndexed(*m_VertexBuffer, *m_IndexBuffer);
         }
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


   struct DirectionalLight {
      alignas(16) glm::vec3 Direction;
      alignas(16) glm::vec3 Color;
      alignas(16) glm::vec3 Ambient;
   };


   struct PointLight {
      alignas(16) glm::vec3 Position;
      alignas(16) glm::vec3 Color;
      alignas(4) float Constant;
      alignas(4) float Linear;
      alignas(4) float Quadratic;
   };


   void CreateUniformBuffers() {
      Material materials[] = {
         {.Shininess{32.0f}}
      };

      // note: shader expects exactly 1
      DirectionalLight directionalLights[] = {
         {
            .Direction = {-0.2f, -1.0f, -0.3f},
            .Color = {0.0f, 0.0f, 0.0f},
            .Ambient = {0.01f, 0.01f, 0.01f}
         }
      };

      m_MaterialBuffer = Pikzel::RenderCore::CreateUniformBuffer(sizeof(materials), materials);
      m_DirectionalLightBuffer = Pikzel::RenderCore::CreateUniformBuffer(sizeof(directionalLights), directionalLights);
      m_PointLightBuffer = Pikzel::RenderCore::CreateUniformBuffer(sizeof(m_PointLights), m_PointLights);
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

   // note: shader expects exactly 4
   PointLight m_PointLights[4] = {
      {
         .Position = {0.7f, 0.2f, 2.0f},
         .Color = {0.0f, 0.0f, 1.0f},
         .Constant = 1.0f,
         .Linear = 0.09f,
         .Quadratic = 0.032f
      },
      {
         .Position = {2.3f, -3.3f, -4.0f},
         .Color = {0.0f, 1.0f, 0.0f},
         .Constant = 1.0f,
         .Linear = 0.09f,
         .Quadratic = 0.032f
      },
      {
         .Position = {-4.0f, 2.0f, -12.0f},
         .Color = {1.0f, 0.0f, 0.0f},
         .Constant = 1.0f,
         .Linear = 0.09f,
         .Quadratic = 0.032f
      },
      {
         .Position = {0.0f, 0.0f, -3.0f},
         .Color = {1.0f, 1.0f, 0.0f},
         .Constant = 1.0f,
         .Linear = 0.09f,
         .Quadratic = 0.032f
      }
   };

   Pikzel::Input m_Input;
   std::filesystem::path m_bindir;

   Camera m_Camera;

   std::unique_ptr<Pikzel::VertexBuffer> m_VertexBuffer;
   std::unique_ptr<Pikzel::IndexBuffer> m_IndexBuffer;
   std::unique_ptr<Pikzel::Texture2D> m_DiffuseTexture;
   std::unique_ptr<Pikzel::Texture2D> m_SpecularTexture;
   std::unique_ptr<Pikzel::UniformBuffer> m_MaterialBuffer;
   std::unique_ptr<Pikzel::UniformBuffer> m_DirectionalLightBuffer;
   std::unique_ptr<Pikzel::UniformBuffer> m_PointLightBuffer;
   std::unique_ptr<Pikzel::Pipeline> m_PipelineLight;
   std::unique_ptr<Pikzel::Pipeline> m_PipelineLighting;

};


std::unique_ptr<Pikzel::Application> Pikzel::CreateApplication(int argc, const char* argv[]) {
   PKZL_LOG_INFO(APP_DESCRIPTION);
   PKZL_LOG_INFO("Linked against {0} {1}", PKZL_DESCRIPTION, PKZL_VERSION);
#ifdef PKZL_DEBUG
   PKZL_LOG_INFO("DEBUG build");
#endif

   return std::make_unique<MultipleLights>(argc, argv);
}
