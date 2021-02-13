#include "ModelSerializer.h"

#include "Pikzel/Pikzel.h"
#include "Pikzel/Core/EntryPoint.h"

// note: Pikzel uses reverse-Z so near and far planes are swapped
const float nearPlane = 10000.0f;
const float farPlane = 1.f;

class SponzaShadowsApp final : public Pikzel::Application {
public:
   SponzaShadowsApp()
   : Pikzel::Application {{.title = APP_DESCRIPTION, .clearColor = Pikzel::sRGB{0.1f, 0.1f, 0.2f}, .isVSync = true}}
   , m_Input {GetWindow()}
   {

      m_Camera.projection = glm::perspective(m_Camera.fovRadians, static_cast<float>(GetWindow().GetWidth()) / static_cast<float>(GetWindow().GetHeight()), nearPlane, farPlane);

      CreateVertexBuffer();  // for rendering point lights as cubes
      CreateUniformBuffers();
      CreateFramebuffers();
      CreatePipelines();

      // POI: We use the ModelSerializer to load the model from an asset file.  In this case "sponza.gltf"
      m_Model = SponzaShadows::ModelSerializer::Import("Assets/Models/Sponza/sponza.gltf");

      Pikzel::ImGuiEx::Init(GetWindow());
   }


   ~SponzaShadowsApp() {
      SponzaShadows::ModelSerializer::ClearTextureCache();
   }


   virtual void Update(const Pikzel::DeltaTime deltaTime) override {
      PKZL_PROFILE_FUNCTION();
      m_DeltaTime = deltaTime;
      if (m_Input.IsKeyPressed(Pikzel::KeyCode::Escape)) {
         Exit();
      }
      m_Camera.Update(m_Input, deltaTime);
   }


   virtual void RenderBegin() override {}


   virtual void Render() override {
      static float lightRadius = 1000.0f; // TODO: set light radius appropriately
      static glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), 1.0f, lightRadius, 1.0f);  // note: Pikzel uses reverse-Z so near and far planes are swapped

      PKZL_PROFILE_FUNCTION();

      // update buffers
      glm::mat4 view = glm::lookAt(m_Camera.position, m_Camera.position + m_Camera.direction, m_Camera.upVector);

      Matrices matrices;
      matrices.viewProjection = m_Camera.projection * view;
      matrices.lightSpace = m_LightSpace;
      matrices.eyePosition = m_Camera.position;
      m_BufferMatrices->CopyFromHost(0, sizeof(Matrices), &matrices);
      m_BufferDirectionalLight->CopyFromHost(0, sizeof(Pikzel::DirectionalLight) * m_DirectionalLights.size(), m_DirectionalLights.data());
      m_BufferPointLights->CopyFromHost(0, sizeof(Pikzel::PointLight) * m_PointLights.size(), m_PointLights.data());


      // render to directional light shadow map
      glm::mat4 transform = glm::identity<glm::mat4>();
      {
         Pikzel::GraphicsContext& gc = m_FramebufferDirShadow->GetGraphicsContext();
         gc.BeginFrame();
         gc.Bind(*m_PipelineDirShadowMap);

         gc.PushConstant("constants.mvp"_hs, m_LightSpace * transform);

         // POI: To render, we can iterate over the model's meshes, and draw each one
         for (const auto& mesh : m_Model->Meshes) {
            gc.DrawIndexed(*mesh.VertexBuffer, *mesh.IndexBuffer);
         }

         gc.EndFrame();
         gc.SwapBuffers();
      }

      // render to point light shadow map
      {
         Pikzel::GraphicsContext& gcPtShadows = m_FramebufferPtShadow->GetGraphicsContext();
         for (int i = 0; i < m_PointLights.size(); ++i) {
            auto& light = m_PointLights[i];
            std::array<glm::mat4, 6> lightViews = {
               lightProjection * glm::lookAt(light.position, light.position + glm::vec3 { 1.0f,  0.0f,  0.0f}, glm::vec3 {0.0f, -1.0f,  0.0f}),
               lightProjection * glm::lookAt(light.position, light.position + glm::vec3 {-1.0f,  0.0f,  0.0f}, glm::vec3 {0.0f, -1.0f,  0.0f}),
               lightProjection * glm::lookAt(light.position, light.position + glm::vec3 { 0.0f,  1.0f,  0.0f}, glm::vec3 {0.0f,  0.0f,  1.0f}),
               lightProjection * glm::lookAt(light.position, light.position + glm::vec3 { 0.0f, -1.0f,  0.0f}, glm::vec3 {0.0f,  0.0f, -1.0f}),
               lightProjection * glm::lookAt(light.position, light.position + glm::vec3 { 0.0f,  0.0f,  1.0f}, glm::vec3 {0.0f, -1.0f,  0.0f}),
               lightProjection * glm::lookAt(light.position, light.position + glm::vec3 { 0.0f,  0.0f, -1.0f}, glm::vec3 {0.0f, -1.0f,  0.0f}),
            };
            m_BufferLightViews->CopyFromHost(0, sizeof(glm::mat4) * lightViews.size(), lightViews.data());
      
            gcPtShadows.BeginFrame(i == 0 ? Pikzel::BeginFrameOp::ClearAll : Pikzel::BeginFrameOp::ClearNone);
            gcPtShadows.Bind(*m_PipelinePtShadow);
            gcPtShadows.PushConstant("constants.lightIndex"_hs, i);
            gcPtShadows.PushConstant("constants.lightRadius"_hs, lightRadius);
            gcPtShadows.Bind("UBOLightViews"_hs, *m_BufferLightViews);
            gcPtShadows.Bind("UBOPointLights"_hs, *m_BufferPointLights);
            gcPtShadows.PushConstant("constants.model"_hs, transform);

            for (const auto& mesh : m_Model->Meshes) {
               gcPtShadows.DrawIndexed(*mesh.VertexBuffer, *mesh.IndexBuffer);
            }

            gcPtShadows.EndFrame();
            gcPtShadows.SwapBuffers();
         }
      }

      // Render scene
      GetWindow().BeginFrame();
      Pikzel::GraphicsContext& gc = GetWindow().GetGraphicsContext();
      {
         PKZL_PROFILE_SCOPE("Render scene");
         gc.Bind(*m_PipelineScene);
         gc.Bind("UBOMatrices"_hs, *m_BufferMatrices);
         gc.Bind("UBODirectionalLight"_hs, *m_BufferDirectionalLight);
         gc.Bind("UBOPointLights"_hs, *m_BufferPointLights);
         gc.Bind("dirShadowMap"_hs, m_FramebufferDirShadow->GetDepthTexture());
         gc.Bind("ptShadowMap"_hs, m_FramebufferPtShadow->GetDepthTexture());

         glm::mat4 transform = glm::identity<glm::mat4>();
         gc.PushConstant("constants.model"_hs, transform);
         gc.PushConstant("constants.lightRadius"_hs, lightRadius);
         gc.PushConstant("constants.numPointLights"_hs, static_cast<uint32_t>(m_PointLights.size()));

         // POI: To render, we can iterate over the model's meshes, and draw each one
         //      Eventually, we will have a more sophisticated material system rather than the
         //      mesh having a hard-coded set of textures (and a fixed shader to match)
         for (const auto& mesh : m_Model->Meshes) {
            gc.Bind("diffuseMap"_hs, *mesh.DiffuseTexture);
            gc.Bind("specularMap"_hs, *mesh.SpecularTexture);
            gc.Bind("normalMap"_hs, *mesh.NormalTexture);
            gc.DrawIndexed(*mesh.VertexBuffer, *mesh.IndexBuffer);
         }
      }

      // Render lights as little cubes
      {
         PKZL_PROFILE_SCOPE("Render light cubes");
         glm::mat4 projView = m_Camera.projection * view;
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
               ImGuiDrawPointLight(fmt::format("light {0}", i).c_str(), m_PointLights[i]);
            }
            ImGui::Text("Frame time: %.3fms (%.0f FPS)", m_DeltaTime.count() * 1000.0f, 1.0f / m_DeltaTime.count());
            ImGui::Text("Depth buffer:");
            ImVec2 size = ImGui::GetContentRegionAvail();
            ImGui::Image(m_FramebufferDirShadow->GetImGuiDepthTextureId(), size, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
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
         {"inPos", Pikzel::DataType::Vec3},
      };
      m_VertexBuffer = Pikzel::RenderCore::CreateVertexBuffer(layout, sizeof(vertices), vertices);
   }


   struct Matrices {
      glm::mat4 viewProjection;
      glm::mat4 lightSpace;
      glm::vec3 eyePosition;
   };

   void CreateUniformBuffers() {

      glm::mat4 lightProjection = glm::ortho(-2100.0f, 2100.0f, -2000.0f, 2000.0f, 2000.0f, 50.0f);  // TODO: need to automatically determine correct parameters here (+ cascades...)
      glm::mat4 lightView = glm::lookAt(-m_DirectionalLights[0].direction, glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f });

      m_LightSpace = lightProjection * lightView;

      m_BufferMatrices = Pikzel::RenderCore::CreateUniformBuffer(sizeof(Matrices));
      m_BufferDirectionalLight = Pikzel::RenderCore::CreateUniformBuffer(sizeof(Pikzel::DirectionalLight) * m_DirectionalLights.size());
      m_BufferLightViews = Pikzel::RenderCore::CreateUniformBuffer(sizeof(glm::mat4) * m_PointLights.size() * 6);
      m_BufferPointLights = Pikzel::RenderCore::CreateUniformBuffer(sizeof(Pikzel::PointLight) * m_PointLights.size());
   }


   void CreateFramebuffers() {
      const uint32_t shadowMapWidth = 4096;
      const uint32_t shadowMapHeight = 4096;

      if (!m_FramebufferDirShadow) {
         m_FramebufferDirShadow = Pikzel::RenderCore::CreateFramebuffer({
            .width = shadowMapWidth,
            .height = shadowMapHeight,
            .attachments = {{Pikzel::AttachmentType::Depth, Pikzel::TextureFormat::D32F}}
         });
      }

      if (!m_FramebufferPtShadow) {
         m_FramebufferPtShadow = Pikzel::RenderCore::CreateFramebuffer({
            .width = shadowMapWidth / 2,
            .height = shadowMapHeight / 2,
            .layers = 4,
            .attachments = {{Pikzel::AttachmentType::Depth, Pikzel::TextureFormat::D32F, Pikzel::TextureType::TextureCubeArray}}
         });
      }
   }


   void CreatePipelines() {
      m_PipelineLight = GetWindow().GetGraphicsContext().CreatePipeline({
         .shaders = {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/Light.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/Light.frag.spv" }
         },
         .bufferLayout = m_VertexBuffer->GetLayout(),
      });

      Pikzel::BufferLayout layout{
         { "inPos",    Pikzel::DataType::Vec3 },
         { "inNormal", Pikzel::DataType::Vec3 },
         { "inTangent", Pikzel::DataType::Vec3 },
         { "inUV",     Pikzel::DataType::Vec2 },
      };

      m_PipelineDirShadowMap = m_FramebufferDirShadow->GetGraphicsContext().CreatePipeline({
         .shaders = {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/Depth.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/Depth.frag.spv" }
         },
         .bufferLayout = layout
      });

      m_PipelinePtShadow = m_FramebufferPtShadow->GetGraphicsContext().CreatePipeline({
         .shaders = {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/DepthCube.vert.spv" },
            { Pikzel::ShaderType::Geometry, "Assets/" APP_NAME "/Shaders/DepthCube.geom.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/DepthCube.frag.spv" }
         },
         .bufferLayout = layout,
      });

      m_PipelineScene = GetWindow().GetGraphicsContext().CreatePipeline({
         .shaders = {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/LitModel.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/LitModel.frag.spv" }
         },
         .bufferLayout = layout
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
      __super::OnWindowResize(event);
      m_Camera.projection = glm::perspective(m_Camera.fovRadians, static_cast<float>(GetWindow().GetWidth()) / static_cast<float>(GetWindow().GetHeight()), nearPlane, farPlane);

      // recreate framebuffer with new size
      CreateFramebuffers();
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

   // note: currently shader supports between 1 and 32 point lights
   // for zero points lights, pass in one, with power set to 0
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

   std::unique_ptr<SponzaShadows::Model> m_Model;
   std::shared_ptr<Pikzel::VertexBuffer> m_VertexBuffer;
   std::unique_ptr<Pikzel::UniformBuffer> m_BufferMatrices;
   std::unique_ptr<Pikzel::UniformBuffer> m_BufferDirectionalLight;
   std::unique_ptr<Pikzel::UniformBuffer> m_BufferLightViews;
   std::unique_ptr<Pikzel::UniformBuffer> m_BufferPointLights;
   std::unique_ptr<Pikzel::Framebuffer> m_FramebufferDirShadow;
   std::unique_ptr<Pikzel::Framebuffer> m_FramebufferPtShadow;
   std::unique_ptr<Pikzel::Framebuffer> m_FramebufferScene;
   std::unique_ptr<Pikzel::Pipeline> m_PipelineLight;
   std::unique_ptr<Pikzel::Pipeline> m_PipelineDirShadowMap;
   std::unique_ptr<Pikzel::Pipeline> m_PipelinePtShadow;
   std::unique_ptr<Pikzel::Pipeline> m_PipelineScene;

   Pikzel::DeltaTime m_DeltaTime;
};


std::unique_ptr<Pikzel::Application> CreateApplication(int argc, const char* argv[]) {
   return std::make_unique<SponzaShadowsApp>();
}
