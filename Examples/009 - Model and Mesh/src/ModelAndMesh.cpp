#include "ModelSerializer.h"

#include "Pikzel/Pikzel.h"
#include "Pikzel/Core/EntryPoint.h"

// note: Pikzel uses reverse-Z so near and far planes are swapped
const float nearPlane = 10000.0f;
const float farPlane = 1.f;

class ModelAndMeshApp final : public Pikzel::Application {
using super = Pikzel::Application;
public:
   ModelAndMeshApp()
   : Pikzel::Application {{.title = APP_DESCRIPTION, .clearColor = Pikzel::sRGB{0.1f, 0.1f, 0.2f}, .isVSync = true}}
   , m_Input {GetWindow()}
   {

      m_Camera.projection = glm::perspective(m_Camera.fovRadians, static_cast<float>(GetWindow().GetWidth()) / static_cast<float>(GetWindow().GetHeight()), nearPlane, farPlane);

      CreateVertexBuffer();  // for rendering point lights as cubes
      CreateUniformBuffers();
      CreatePipelines();

      // POI: We use the ModelSerializer to load the model from an asset file.  In this case "sponza.gltf"
      m_Model = ModelAndMeshDemo::ModelSerializer::Import("Assets/Models/Sponza/Sponza.gltf");

      Pikzel::ImGuiEx::Init(GetWindow());
   }


   ~ModelAndMeshApp() {
      ModelAndMeshDemo::ModelSerializer::ClearTextureCache();
   }


   virtual void Update(const Pikzel::DeltaTime deltaTime) override {
      PKZL_PROFILE_FUNCTION();
      m_DeltaTime = deltaTime;
      if (m_Input.IsKeyPressed(Pikzel::KeyCode::Escape)) {
         Exit();
      }
      m_Camera.Update(m_Input, deltaTime);
   }


   virtual void Render() override {
      PKZL_PROFILE_FUNCTION();

      // update buffers
      glm::mat4 view = glm::lookAt(m_Camera.position, m_Camera.position + m_Camera.direction, m_Camera.upVector);
      glm::mat4 projView = m_Camera.projection * view;
      m_BufferDirectionalLight->CopyFromHost(0, sizeof(Pikzel::DirectionalLight) * m_DirectionalLights.size(), m_DirectionalLights.data());
      m_BufferPointLights->CopyFromHost(0, sizeof(Pikzel::PointLight) * m_PointLights.size(), m_PointLights.data());

      // Render scene
      Pikzel::GraphicsContext& gc = GetWindow().GetGraphicsContext();
      {
         PKZL_PROFILE_SCOPE("Render scene");
         gc.Bind(*m_PipelineScene);
         gc.Bind("UBODirectionalLight"_hs, *m_BufferDirectionalLight);
         gc.Bind("UBOPointLights"_hs, *m_BufferPointLights);

         glm::mat4 transform = glm::identity<glm::mat4>();
         gc.PushConstant("constants.vp"_hs, projView);
         gc.PushConstant("constants.model"_hs, transform);
         gc.PushConstant("constants.modelInvTrans"_hs, glm::mat4(glm::transpose(glm::inverse(glm::mat3(transform)))));
         gc.PushConstant("constants.viewPos"_hs, m_Camera.position);
         gc.PushConstant("constants.shininess"_hs, 32.0f);

         // POI: To render, we can iterate over the model's meshes, and draw each one
         //      Eventually, we will have a more sophisticated material system rather than the
         //      mesh having a hard-coded set of textures (and a fixed shader to match)
         for (const auto& mesh : m_Model->Meshes) {
            gc.Bind("diffuseMap"_hs, *mesh.DiffuseTexture);
            gc.Bind("specularMap"_hs, *mesh.SpecularTexture);
            gc.DrawIndexed(*mesh.VertexBuffer, *mesh.IndexBuffer);
         }
      }

      // Render lights as little cubes
      {
         PKZL_PROFILE_SCOPE("Render light cubes");
         {
            gc.Bind(*m_PipelineLight);
            for (const auto& pointLight : m_PointLights) {
               glm::mat4 model = glm::translate(glm::identity<glm::mat4>(), pointLight.position);
               gc.PushConstant("constants.mvp"_hs, projView * model);
               gc.PushConstant("constants.lightColor"_hs, pointLight.color);
               gc.DrawTriangles(*m_VertexBuffer, 36);
            }
         }
      }

      // Render ImGUI layer
      {
         PKZL_PROFILE_SCOPE("Render ImGui");
         GetWindow().BeginImGuiFrame();
         {
            ImGui::Begin("Lighting");
            for (size_t i = 0; i < m_PointLights.size(); ++i) {
               ImGuiDrawPointLight(fmt::format("light {}", i).data(), m_PointLights[i]);
            }
            ImGui::Text("Frame time: %.3fms (%.0f FPS)", m_DeltaTime.count() * 1000.0f, 1.0f / m_DeltaTime.count());
            ImGui::End();
         }
         GetWindow().EndImGuiFrame();
      }
      GetWindow().EndFrame();
   }


   virtual void RenderEnd() override {}

private:

   void CreateVertexBuffer() {
      glm::vec3 vertices[] = {
         {-0.5f, -0.5f, -0.5f},
         { 0.5f,  0.5f, -0.5f},
         { 0.5f, -0.5f, -0.5f},
         { 0.5f,  0.5f, -0.5f},
         {-0.5f, -0.5f, -0.5f},
         {-0.5f,  0.5f, -0.5f},

         {-0.5f, -0.5f,  0.5f},
         { 0.5f, -0.5f,  0.5f},
         { 0.5f,  0.5f,  0.5f},
         { 0.5f,  0.5f,  0.5f},
         {-0.5f,  0.5f,  0.5f},
         {-0.5f, -0.5f,  0.5f},

         {-0.5f,  0.5f,  0.5f},
         {-0.5f,  0.5f, -0.5f},
         {-0.5f, -0.5f, -0.5f},
         {-0.5f, -0.5f, -0.5f},
         {-0.5f, -0.5f,  0.5f},
         {-0.5f,  0.5f,  0.5f},

         { 0.5f,  0.5f,  0.5f},
         { 0.5f, -0.5f, -0.5f},
         { 0.5f,  0.5f, -0.5f},
         { 0.5f, -0.5f, -0.5f},
         { 0.5f,  0.5f,  0.5f},
         { 0.5f, -0.5f,  0.5f},

         {-0.5f, -0.5f, -0.5f},
         { 0.5f, -0.5f, -0.5f},
         { 0.5f, -0.5f,  0.5f},
         { 0.5f, -0.5f,  0.5f},
         {-0.5f, -0.5f,  0.5f},
         {-0.5f, -0.5f, -0.5f},

         {-0.5f,  0.5f, -0.5f},
         { 0.5f,  0.5f,  0.5f},
         { 0.5f,  0.5f, -0.5f},
         { 0.5f,  0.5f,  0.5f},
         {-0.5f,  0.5f, -0.5f},
         {-0.5f,  0.5f,  0.5f}
      };

      Pikzel::BufferLayout layout = {
         {"inPos", Pikzel::DataType::Vec3}
      };
      m_VertexBuffer = Pikzel::RenderCore::CreateVertexBuffer(layout, sizeof(vertices), vertices);
   }


   void CreateUniformBuffers() {
      m_BufferDirectionalLight = Pikzel::RenderCore::CreateUniformBuffer(sizeof(Pikzel::DirectionalLight) * m_DirectionalLights.size());
      m_BufferPointLights = Pikzel::RenderCore::CreateUniformBuffer(sizeof(Pikzel::PointLight) * m_PointLights.size());
   }


   void CreatePipelines() {
      m_PipelineLight = GetWindow().GetGraphicsContext().CreatePipeline({
         .shaders = {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/Light.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/Light.frag.spv" }
         },
         .bufferLayout = m_VertexBuffer->GetLayout()
      });

      m_PipelineScene = GetWindow().GetGraphicsContext().CreatePipeline({
         .shaders = {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/Lighting.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/Lighting.frag.spv" }
         },
         .bufferLayout = {
            { "inPos",    Pikzel::DataType::Vec3 },
            { "inNormal", Pikzel::DataType::Vec3 },
            { "inUV",     Pikzel::DataType::Vec2 }
         }
      });
   }


   static void ImGuiDrawPointLight(const char* label, Pikzel::PointLight& pointLight) {
      ImGui::PushID(label);
      if (ImGui::TreeNode(label)) {
         Pikzel::ImGuiEx::EditVec3("Position", &pointLight.position);
         Pikzel::ImGuiEx::EditVec3Color("Color", &pointLight.color); 
         Pikzel::ImGuiEx::EditFloat("Size", &pointLight.size);
         Pikzel::ImGuiEx::EditFloat("Power", &pointLight.power);
         ImGui::TreePop();
      }
      ImGui::PopID();
   }


   virtual void OnWindowResize(const Pikzel::WindowResizeEvent& event) override {
      super::OnWindowResize(event);
      m_Camera.projection = glm::perspective(m_Camera.fovRadians, static_cast<float>(GetWindow().GetWidth()) / static_cast<float>(GetWindow().GetHeight()), nearPlane, farPlane);
   }


private:

   Pikzel::Input m_Input;

   Camera m_Camera = {
      .position = {-900.0f, 100.0f, 0.0f},
      .direction = glm::normalize(glm::vec3{900.0f, -100.0f, 0.0f}),
      .upVector = {0.0f, 1.0f, 0.0f},
      .fovRadians = glm::radians(45.f),
      .moveSpeed = 200.0f,
      .rotateSpeed = 20.0f
   };

   // note: currently shader expects exactly 1 directional light
   std::vector<Pikzel::DirectionalLight> m_DirectionalLights = {
      {
         .direction = {250.0f, -1750.0f, 250.0f},
         .color = Pikzel::sRGB{0.5f, 0.5f, 0.5f},
         .ambient = Pikzel::sRGB{0.1f, 0.1f, 0.1f},
         .size = 0.002f
      }
   };

   glm::mat4 m_LightSpace;

   // note: currently shader expects exactly 4 point lights
   // You can turn them off by setting power to 0
   std::vector<Pikzel::PointLight> m_PointLights = {
      {
         .position = {-619.3f, 130.3f, -219.5f},
         .color = Pikzel::sRGB{1.0f, 1.0f, 1.0f},
         .size = 1.0f,
         .power = 30000.0f
      },
      {
         .position = {487.3f, 130.3f, -219.5f},
         .color = Pikzel::sRGB{1.0f, 1.0f, 1.0f},
         .size = 1.0f,
         .power = 30000.0f
      },
      {
         .position = {487.3f, 130.3f, 141.1f},
         .color = Pikzel::sRGB{1.0f, 1.0f, 1.0f},
         .size = 1.0f,
         .power = 30000.0f
      },
      {
         .position = {-619.3f, 130.3f, 141.1f},
         .color = Pikzel::sRGB{1.0f, 1.0f, 1.0f},
         .size = 1.0f,
         .power = 30000.0f
      }
   };

   std::unique_ptr<ModelAndMeshDemo::Model> m_Model;
   std::shared_ptr<Pikzel::VertexBuffer> m_VertexBuffer;
   std::unique_ptr<Pikzel::UniformBuffer> m_BufferDirectionalLight;
   std::unique_ptr<Pikzel::UniformBuffer> m_BufferPointLights;
   std::unique_ptr<Pikzel::Pipeline> m_PipelineLight;
   std::unique_ptr<Pikzel::Pipeline> m_PipelineScene;

   Pikzel::DeltaTime m_DeltaTime;
};


std::unique_ptr<Pikzel::Application> CreateApplication(int argc, const char* argv[]) {
   return std::make_unique<ModelAndMeshApp>();
}
