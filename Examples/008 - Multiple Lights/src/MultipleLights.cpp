
#include "Pikzel/Pikzel.h"
#include "Pikzel/Core/EntryPoint.h"

class MultipleLights final : public Pikzel::Application {
public:
   MultipleLights()
   : Pikzel::Application {{.title = APP_DESCRIPTION, .clearColor = Pikzel::sRGB{0.05f, 0.05f, 0.05f}, .isVSync = true}}
   , m_Input {GetWindow()}
   {
      CreateVertexBuffer();
      CreateTextures();
      CreateUniformBuffers();
      CreatePipelines();

      // note: Pikzel uses reverse-Z so near and far planes are swapped
      m_Camera.projection = glm::perspective(m_Camera.fovRadians, static_cast<float>(GetWindow().GetWidth()) / static_cast<float>(GetWindow().GetHeight()), 100.0f, 0.1f);
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

      glm::mat4 projView = m_Camera.projection * glm::lookAt(m_Camera.position, m_Camera.position + m_Camera.direction, m_Camera.upVector);

      Pikzel::GraphicsContext& gc = GetWindow().GetGraphicsContext();
      {
         gc.Bind(*m_PipelineLight);
         for (const auto& pointLight : m_PointLights) {
            glm::mat4 model = glm::scale(glm::translate(glm::identity<glm::mat4>(), pointLight.position), {0.2f, 0.2f, 0.2f});
            gc.PushConstant("constants.mvp"_hs, projView * model);
            gc.PushConstant("constants.lightColor"_hs, pointLight.color);
            gc.DrawTriangles(*m_VertexBuffer, 36);
         }
      }
      {
         gc.Bind(*m_PipelineLighting);
         gc.Bind("diffuseMap"_hs, *m_DiffuseTexture);
         gc.Bind("specularMap"_hs, *m_SpecularTexture);
         gc.Bind("UBODirectionalLight"_hs, *m_DirectionalLightBuffer);
         gc.Bind("UBOPointLights"_hs, *m_PointLightBuffer);

         gc.PushConstant("constants.vp"_hs, projView);
         gc.PushConstant("constants.viewPos"_hs, m_Camera.position);
         gc.PushConstant("constants.shininess"_hs, 32.0f);

         for (int i = 0; i < 10; ++i) {
            glm::mat4 model = glm::rotate(glm::translate(glm::identity<glm::mat4>(), m_CubePositions[i]), glm::radians(20.0f * i), glm::vec3 {1.0f, 0.3f, 0.5f});
            glm::mat4 modelInvTrans = glm::mat4(glm::transpose(glm::inverse(glm::mat3(model))));
            gc.PushConstant("constants.model"_hs, model);
            gc.PushConstant("constants.modelInvTrans"_hs, modelInvTrans);
            gc.DrawTriangles(*m_VertexBuffer, 36);
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
         {.Pos{ 0.5f,  0.5f, -0.5f}, .Normal{ 0.0f,  0.0f, -1.0f}, .TexCoord{1.0f, 1.0f}},
         {.Pos{ 0.5f, -0.5f, -0.5f}, .Normal{ 0.0f,  0.0f, -1.0f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{ 0.5f,  0.5f, -0.5f}, .Normal{ 0.0f,  0.0f, -1.0f}, .TexCoord{1.0f, 1.0f}},
         {.Pos{-0.5f, -0.5f, -0.5f}, .Normal{ 0.0f,  0.0f, -1.0f}, .TexCoord{0.0f, 0.0f}},
         {.Pos{-0.5f,  0.5f, -0.5f}, .Normal{ 0.0f,  0.0f, -1.0f}, .TexCoord{0.0f, 1.0f}},

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
         {.Pos{ 0.5f, -0.5f, -0.5f}, .Normal{ 1.0f,  0.0f,  0.0f}, .TexCoord{0.0f, 1.0f}},
         {.Pos{ 0.5f,  0.5f, -0.5f}, .Normal{ 1.0f,  0.0f,  0.0f}, .TexCoord{1.0f, 1.0f}},
         {.Pos{ 0.5f, -0.5f, -0.5f}, .Normal{ 1.0f,  0.0f,  0.0f}, .TexCoord{0.0f, 1.0f}},
         {.Pos{ 0.5f,  0.5f,  0.5f}, .Normal{ 1.0f,  0.0f,  0.0f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{ 0.5f, -0.5f,  0.5f}, .Normal{ 1.0f,  0.0f,  0.0f}, .TexCoord{0.0f, 0.0f}},

         {.Pos{-0.5f, -0.5f, -0.5f}, .Normal{ 0.0f, -1.0f,  0.0f}, .TexCoord{0.0f, 1.0f}},
         {.Pos{ 0.5f, -0.5f, -0.5f}, .Normal{ 0.0f, -1.0f,  0.0f}, .TexCoord{1.0f, 1.0f}},
         {.Pos{ 0.5f, -0.5f,  0.5f}, .Normal{ 0.0f, -1.0f,  0.0f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{ 0.5f, -0.5f,  0.5f}, .Normal{ 0.0f, -1.0f,  0.0f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{-0.5f, -0.5f,  0.5f}, .Normal{ 0.0f, -1.0f,  0.0f}, .TexCoord{0.0f, 0.0f}},
         {.Pos{-0.5f, -0.5f, -0.5f}, .Normal{ 0.0f, -1.0f,  0.0f}, .TexCoord{0.0f, 1.0f}},

         {.Pos{-0.5f,  0.5f, -0.5f}, .Normal{ 0.0f,  1.0f,  0.0f}, .TexCoord{0.0f, 1.0f}},
         {.Pos{ 0.5f,  0.5f,  0.5f}, .Normal{ 0.0f,  1.0f,  0.0f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{ 0.5f,  0.5f, -0.5f}, .Normal{ 0.0f,  1.0f,  0.0f}, .TexCoord{1.0f, 1.0f}},
         {.Pos{ 0.5f,  0.5f,  0.5f}, .Normal{ 0.0f,  1.0f,  0.0f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{-0.5f,  0.5f, -0.5f}, .Normal{ 0.0f,  1.0f,  0.0f}, .TexCoord{0.0f, 1.0f}},
         {.Pos{-0.5f,  0.5f,  0.5f}, .Normal{ 0.0f,  1.0f,  0.0f}, .TexCoord{0.0f, 0.0f}}
      };

      Pikzel::BufferLayout layout = {
         {"inPos",       Pikzel::DataType::Vec3},
         {"inNormal",    Pikzel::DataType::Vec3},
         {"inTexCoords", Pikzel::DataType::Vec2},
      };
      m_VertexBuffer = Pikzel::RenderCore::CreateVertexBuffer(layout, sizeof(vertices), vertices);
   }


   void CreateTextures() {
      m_DiffuseTexture = Pikzel::RenderCore::CreateTexture({.path = "Assets/" APP_NAME "/Textures/Diffuse.png"});
      m_SpecularTexture = Pikzel::RenderCore::CreateTexture({.path = "Assets/" APP_NAME "/Textures/Specular.png", .format = Pikzel::TextureFormat::RGBA8});
   }


   void CreateUniformBuffers() {
      // note: shader expects exactly 1
      Pikzel::DirectionalLight directionalLights[] = {
         {
            .direction = {-0.2f, -1.0f, -0.3f},
            .color = Pikzel::sRGB{1.0f, 1.0f, 1.0f},
            .ambient = Pikzel::sRGB{0.01f, 0.01f, 0.01f}
         }
      };

      m_DirectionalLightBuffer = Pikzel::RenderCore::CreateUniformBuffer(sizeof(directionalLights), directionalLights);
      m_PointLightBuffer = Pikzel::RenderCore::CreateUniformBuffer(sizeof(m_PointLights), m_PointLights);
   }


   void CreatePipelines() {
      m_PipelineLight = GetWindow().GetGraphicsContext().CreatePipeline({
         .shaders = {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/Light.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/Light.frag.spv" }
         },
         .bufferLayout = m_VertexBuffer->GetLayout()
      });
      m_PipelineLighting = GetWindow().GetGraphicsContext().CreatePipeline({
         .shaders = {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/Lighting.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/Lighting.frag.spv" }
         },
         .bufferLayout = m_VertexBuffer->GetLayout(),
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
   Pikzel::PointLight m_PointLights[4] = {
      {
         .position = {0.7f, 0.2f, 2.0f},
         .color = Pikzel::sRGB{0.0f, 0.0f, 1.0f},
         .power = 30.0f
      },
      {
         .position = {2.3f, -3.3f, -4.0f},
         .color = Pikzel::sRGB{0.0f, 1.0f, 0.0f},
         .power = 30.0f
      },
      {
         .position = {-4.0f, 2.0f, -12.0f},
         .color = Pikzel::sRGB{1.0f, 0.0f, 0.0f},
         .power = 30.0f
      },
      {
         .position = {0.0f, 0.0f, -3.0f},
         .color = Pikzel::sRGB{1.0f, 1.0f, 0.0f},
         .power = 30.0f
      }
   };

   Pikzel::Input m_Input;

   Camera m_Camera = {
      .position = {0.0f, 0.0f, 3.0f}
   };

   std::unique_ptr<Pikzel::VertexBuffer> m_VertexBuffer;
   std::unique_ptr<Pikzel::Texture> m_DiffuseTexture;
   std::unique_ptr<Pikzel::Texture> m_SpecularTexture;
   std::unique_ptr<Pikzel::UniformBuffer> m_DirectionalLightBuffer;
   std::unique_ptr<Pikzel::UniformBuffer> m_PointLightBuffer;
   std::unique_ptr<Pikzel::Pipeline> m_PipelineLight;
   std::unique_ptr<Pikzel::Pipeline> m_PipelineLighting;

};


std::unique_ptr<Pikzel::Application> CreateApplication(int argc, const char* argv[]) {
   return std::make_unique<MultipleLights>();
}
