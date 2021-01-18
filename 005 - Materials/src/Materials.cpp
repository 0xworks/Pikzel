#include "Pikzel/Pikzel.h"
#include "Pikzel/Core/EntryPoint.h"

class Materials final : public Pikzel::Application {
public:
   Materials()
   : Pikzel::Application {{.Title = APP_DESCRIPTION, .ClearColor = Pikzel::sRGB{0.1f, 0.1f, 0.1f}}}
   , m_Input {GetWindow()}
   {
      CreateVertexBuffer();
      CreateUniformBuffers();
      CreatePipelines();

      // note: Pikzel uses reverse-Z so near and far planes are swapped
      m_Projection = glm::perspective(glm::radians(45.0f), static_cast<float>(GetWindow().GetWidth()) / static_cast<float>(GetWindow().GetHeight()), 100.0f, 0.1f);
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
         gc.DrawTriangles(*m_VertexBuffer, 36);
      }
      {
         gc.Bind(*m_PipelineLighting);
         gc.Bind(*m_MaterialBuffer, "Materials"_hs);
         gc.Bind(*m_LightBuffer, "Lights"_hs);

         glm::mat4 model = glm::identity<glm::mat4>();
         glm::mat4 modelInvTrans = glm::mat4(glm::transpose(glm::inverse(glm::mat3(model))));
         gc.PushConstant("constants.vp"_hs, projView);
         gc.PushConstant("constants.model"_hs, model);
         gc.PushConstant("constants.modelInvTrans"_hs, modelInvTrans);
         gc.PushConstant("constants.viewPos"_hs, m_CameraPos);
         gc.DrawTriangles(*m_VertexBuffer, 36);
      }
   }


private:

   struct Vertex {
      glm::vec3 Pos;
      glm::vec3 Normal;
   };

   void CreateVertexBuffer() {
      Vertex vertices[] = {
         {.Pos{-0.5f, -0.5f, -0.5f}, .Normal{ 0.0f,  0.0f, -1.0f}},
         {.Pos{ 0.5f,  0.5f, -0.5f}, .Normal{ 0.0f,  0.0f, -1.0f}},
         {.Pos{ 0.5f, -0.5f, -0.5f}, .Normal{ 0.0f,  0.0f, -1.0f}},
         {.Pos{ 0.5f,  0.5f, -0.5f}, .Normal{ 0.0f,  0.0f, -1.0f}},
         {.Pos{-0.5f, -0.5f, -0.5f}, .Normal{ 0.0f,  0.0f, -1.0f}},
         {.Pos{-0.5f,  0.5f, -0.5f}, .Normal{ 0.0f,  0.0f, -1.0f}},

         {.Pos{-0.5f, -0.5f,  0.5f}, .Normal{ 0.0f,  0.0f,  1.0f}},
         {.Pos{ 0.5f, -0.5f,  0.5f}, .Normal{ 0.0f,  0.0f,  1.0f}},
         {.Pos{ 0.5f,  0.5f,  0.5f}, .Normal{ 0.0f,  0.0f,  1.0f}},
         {.Pos{ 0.5f,  0.5f,  0.5f}, .Normal{ 0.0f,  0.0f,  1.0f}},
         {.Pos{-0.5f,  0.5f,  0.5f}, .Normal{ 0.0f,  0.0f,  1.0f}},
         {.Pos{-0.5f, -0.5f,  0.5f}, .Normal{ 0.0f,  0.0f,  1.0f}},

         {.Pos{-0.5f,  0.5f,  0.5f}, .Normal{-1.0f,  0.0f,  0.0f}},
         {.Pos{-0.5f,  0.5f, -0.5f}, .Normal{-1.0f,  0.0f,  0.0f}},
         {.Pos{-0.5f, -0.5f, -0.5f}, .Normal{-1.0f,  0.0f,  0.0f}},
         {.Pos{-0.5f, -0.5f, -0.5f}, .Normal{-1.0f,  0.0f,  0.0f}},
         {.Pos{-0.5f, -0.5f,  0.5f}, .Normal{-1.0f,  0.0f,  0.0f}},
         {.Pos{-0.5f,  0.5f,  0.5f}, .Normal{-1.0f,  0.0f,  0.0f}},

         {.Pos{ 0.5f,  0.5f,  0.5f}, .Normal{ 1.0f,  0.0f,  0.0f}},
         {.Pos{ 0.5f, -0.5f, -0.5f}, .Normal{ 1.0f,  0.0f,  0.0f}},
         {.Pos{ 0.5f,  0.5f, -0.5f}, .Normal{ 1.0f,  0.0f,  0.0f}},
         {.Pos{ 0.5f, -0.5f, -0.5f}, .Normal{ 1.0f,  0.0f,  0.0f}},
         {.Pos{ 0.5f,  0.5f,  0.5f}, .Normal{ 1.0f,  0.0f,  0.0f}},
         {.Pos{ 0.5f, -0.5f,  0.5f}, .Normal{ 1.0f,  0.0f,  0.0f}},

         {.Pos{-0.5f, -0.5f, -0.5f}, .Normal{ 0.0f, -1.0f,  0.0f}},
         {.Pos{ 0.5f, -0.5f, -0.5f}, .Normal{ 0.0f, -1.0f,  0.0f}},
         {.Pos{ 0.5f, -0.5f,  0.5f}, .Normal{ 0.0f, -1.0f,  0.0f}},
         {.Pos{ 0.5f, -0.5f,  0.5f}, .Normal{ 0.0f, -1.0f,  0.0f}},
         {.Pos{-0.5f, -0.5f,  0.5f}, .Normal{ 0.0f, -1.0f,  0.0f}},
         {.Pos{-0.5f, -0.5f, -0.5f}, .Normal{ 0.0f, -1.0f,  0.0f}},

         {.Pos{-0.5f,  0.5f, -0.5f}, .Normal{ 0.0f,  1.0f,  0.0f}},
         {.Pos{ 0.5f,  0.5f,  0.5f}, .Normal{ 0.0f,  1.0f,  0.0f}},
         {.Pos{ 0.5f,  0.5f, -0.5f}, .Normal{ 0.0f,  1.0f,  0.0f}},
         {.Pos{ 0.5f,  0.5f,  0.5f}, .Normal{ 0.0f,  1.0f,  0.0f}},
         {.Pos{-0.5f,  0.5f, -0.5f}, .Normal{ 0.0f,  1.0f,  0.0f}},
         {.Pos{-0.5f,  0.5f,  0.5f}, .Normal{ 0.0f,  1.0f,  0.0f}}
      };

      Pikzel::BufferLayout layout = {
         {"inPos",    Pikzel::DataType::Vec3},
         {"inNormal", Pikzel::DataType::Vec3},
      };
      m_VertexBuffer = Pikzel::RenderCore::CreateVertexBuffer(layout, sizeof(vertices), vertices);
   }


   struct Material {
      alignas(16) glm::vec3 Ambient;
      alignas(16) glm::vec3 Diffuse;
      alignas(16) glm::vec3 Specular;
      alignas(4)  float Shininess;
   };


   struct Light {
      alignas(16) glm::vec3 Position;
      alignas(16) glm::vec3 Ambient;
      alignas(16) glm::vec3 Diffuse;
      alignas(16) glm::vec3 Specular;
   };

   inline static glm::vec3 m_LightPos = {1.2f, 1.0f, 2.0f};
   inline static glm::vec3 m_LightColor = Pikzel::sRGB{1.0f, 1.0f, 1.0f};

   void CreateUniformBuffers() {
      Material materials[] = {
         {.Ambient{Pikzel::sRGB{1.0f, 0.5f, 0.31f}}, .Diffuse{Pikzel::sRGB{1.0f, 0.5f, 0.31f}}, .Specular{0.5f, 0.5f, 0.5f}, .Shininess{32.0f}}
      };
      m_MaterialBuffer = Pikzel::RenderCore::CreateUniformBuffer(sizeof(materials), materials);

      Light lights[] = {
         {.Position{m_LightPos}, .Ambient{Pikzel::sRGB{0.1f, 0.1f, 0.1f}}, .Diffuse{m_LightColor}, .Specular{m_LightColor}}
      };
      m_LightBuffer = Pikzel::RenderCore::CreateUniformBuffer(sizeof(lights), lights);
   }


   void CreatePipelines() {
      m_PipelineLight = GetWindow().GetGraphicsContext().CreatePipeline({
         .Shaders = {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/Light.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/Light.frag.spv" }
         },
         .BufferLayout = m_VertexBuffer->GetLayout(),
      });
      m_PipelineLighting = GetWindow().GetGraphicsContext().CreatePipeline({
         .Shaders = {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/Lighting.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/Lighting.frag.spv" }
         },
         .BufferLayout = m_VertexBuffer->GetLayout(),
      });
   }


private:
   Pikzel::Input m_Input;

   glm::vec3 m_CameraPos {0.0f, 0.0f, 3.0f};
   glm::vec3 m_CameraDirection = {0.0f, 0.0f, -1.0f};
   glm::vec3 m_CameraUp = {0.0f, 1.0f, 0.0f};
   float m_CameraPitch = 0.0f;
   float m_CameraYaw = -90.0f;
   float m_CameraFoV = 45.0f;

   glm::mat4 m_Projection = glm::identity<glm::mat4>();

   std::unique_ptr<Pikzel::VertexBuffer> m_VertexBuffer;
   std::unique_ptr<Pikzel::UniformBuffer> m_MaterialBuffer;
   std::unique_ptr<Pikzel::UniformBuffer> m_LightBuffer;
   std::unique_ptr<Pikzel::Pipeline> m_PipelineLight;
   std::unique_ptr<Pikzel::Pipeline> m_PipelineLighting;

};


std::unique_ptr<Pikzel::Application> CreateApplication(int argc, const char* argv[]) {
   return std::make_unique<Materials>();
}
