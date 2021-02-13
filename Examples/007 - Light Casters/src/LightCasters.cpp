#include "Pikzel/Pikzel.h"
#include "Pikzel/Core/EntryPoint.h"

class LightCasters final : public Pikzel::Application {
public:
   LightCasters()
   : Pikzel::Application {{.title = APP_DESCRIPTION, .clearColor = Pikzel::sRGB{0.1f, 0.1f, 0.1f}, .isVSync = false}}
   , m_Input {GetWindow()}
   , m_Light {
      .position = m_CameraPos,
      .direction = m_CameraDirection,
      .cutoff = glm::cos(glm::radians(12.5f)),
      .ambient = Pikzel::sRGB{0.1f, 0.1f, 0.1f},
      .diffuse = Pikzel::sRGB{1.0f, 1.0f, 1.0f},
      .specular = Pikzel::sRGB{1.0f, 1.0f, 1.0f},
      .constant = 1.0f,
      .linear = 0.09f,
      .quadratic = 0.032f
   }
   {
      CreateVertexBuffer();
      CreateTextures();
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

      m_Light.position = m_CameraPos;
      m_Light.direction = m_CameraDirection;
   }


   virtual void Render() override {
      PKZL_PROFILE_FUNCTION();

      glm::mat4 projView = m_Projection * glm::lookAt(m_CameraPos, m_CameraPos + m_CameraDirection, m_CameraUp);
   
      // I think there is a data race here (but nothing bad seems to happen ??)
      // How can we be sure that the commands submitted to the GPU for the previous frame have finished with light buffer
      // before we go copying in the new values?
      m_LightBuffer->CopyFromHost(0, sizeof(Light), &m_Light);

      Pikzel::GraphicsContext& gc = GetWindow().GetGraphicsContext();
      {
         gc.Bind(*m_PipelineLighting);
         gc.Bind("diffuseMap"_hs, *m_DiffuseTexture);
         gc.Bind("specularMap"_hs, *m_SpecularTexture);
         gc.Bind("Materials"_hs, *m_MaterialBuffer);
         gc.Bind("Lights"_hs, *m_LightBuffer);

         gc.PushConstant("constants.vp"_hs, projView);
         gc.PushConstant("constants.viewPos"_hs, m_CameraPos);

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


   struct Material {
      alignas(4)  float shininess;
   };


   struct Light {
      alignas(16) glm::vec3 position;
      alignas(16) glm::vec3 direction;
      alignas(4) float cutoff;
      alignas(16) glm::vec3 ambient;
      alignas(16) glm::vec3 diffuse;
      alignas(16) glm::vec3 specular;
      alignas(4) float constant;
      alignas(4) float linear;
      alignas(4) float quadratic;

   };


   void CreateUniformBuffers() {
      Material materials[] = {
         {.shininess{32.0f}}
      };
      m_MaterialBuffer = Pikzel::RenderCore::CreateUniformBuffer(sizeof(materials), materials);
      m_LightBuffer = Pikzel::RenderCore::CreateUniformBuffer(sizeof(Light));
   }


   void CreatePipelines() {
      m_PipelineLighting = GetWindow().GetGraphicsContext().CreatePipeline({
         .shaders = {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/Lighting.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/Lighting.frag.spv" }
         },
         .bufferLayout = m_VertexBuffer->GetLayout()
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

   Pikzel::Input m_Input;

   glm::vec3 m_CameraPos {0.0f, 0.0f, 3.0f};
   glm::vec3 m_CameraDirection = {0.0f, 0.0f, -1.0f};
   glm::vec3 m_CameraUp = {0.0f, 1.0f, 0.0f};
   float m_CameraPitch = 0.0f;
   float m_CameraYaw = -90.0f;
   float m_CameraFoV = 45.0f;

   glm::mat4 m_Projection = glm::identity<glm::mat4>();

   Light m_Light;

   std::unique_ptr<Pikzel::VertexBuffer> m_VertexBuffer;
   std::unique_ptr<Pikzel::Texture> m_DiffuseTexture;
   std::unique_ptr<Pikzel::Texture> m_SpecularTexture;
   std::unique_ptr<Pikzel::UniformBuffer> m_MaterialBuffer;
   std::unique_ptr<Pikzel::UniformBuffer> m_LightBuffer;
   std::unique_ptr<Pikzel::Pipeline> m_PipelineLighting;

};


std::unique_ptr<Pikzel::Application> CreateApplication(int argc, const char* argv[]) {
   return std::make_unique<LightCasters>();
}
