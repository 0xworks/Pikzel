#include "Pikzel/Pikzel.h"
#include "Pikzel/Core/EntryPoint.h"

#include <format>
#include <memory>
#include <vector>

// note: Pikzel uses reverse-Z so near and far planes are swapped
const float nearPlane = 50.0f;
const float farPlane = 0.1f;

class NormalMaps final : public Pikzel::Application {
using super = Pikzel::Application;
public:
   NormalMaps()
   : Pikzel::Application {{.title = APP_DESCRIPTION, .clearColor = Pikzel::sRGB{0.01f, 0.01f, 0.01f}, .isVSync = true}}
   , m_Input {GetWindow()}
   {
      CreateVertexBuffer();
      CreateUniformBuffers();
      CreateTextures();
      CreateFramebuffers();
      CreatePipelines();

      m_Camera.projection = glm::perspective(m_Camera.fovRadians, static_cast<float>(GetWindow().GetWidth()) / static_cast<float>(GetWindow().GetHeight()), nearPlane, farPlane);

      Pikzel::ImGuiEx::Init(GetWindow());
   }


protected:

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
      PKZL_PROFILE_FUNCTION();

      static float lightRadius = 25.0f;
      static glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, lightRadius);

      // update buffers
      Matrices matrices;
      matrices.viewProjection = m_Camera.projection * glm::lookAt(m_Camera.position, m_Camera.position + m_Camera.direction, m_Camera.upVector);
      matrices.lightSpace = m_LightSpace;
      matrices.eyePosition = m_Camera.position;
      m_BufferMatrices->CopyFromHost(0, sizeof(Matrices), &matrices);
      m_BufferPointLights->CopyFromHost(0, sizeof(Pikzel::PointLight) * m_PointLights.size(), m_PointLights.data());

      // render scene
      {

         Pikzel::GraphicsContext& gc = m_FramebufferScene->GetGraphicsContext();
         gc.BeginFrame();

         if (m_ShowPointLights) {
            gc.Bind(*m_PipelineColoredModel);
            for (const auto& pointLight : m_PointLights) {
               glm::mat4 model = glm::scale(glm::translate(glm::identity<glm::mat4>(), pointLight.position), {0.05f, 0.05f, 0.05f});
               gc.PushConstant("constants.mvp"_hs, matrices.viewProjection * model);
               gc.PushConstant("constants.color"_hs, pointLight.color);
               gc.DrawTriangles(*m_VertexBuffer, 36);
            }
         }

         gc.Bind(*m_PipelineLitModel);
         gc.PushConstant("constants.lightRadius"_hs, lightRadius);
         gc.PushConstant("constants.numPointLights"_hs, static_cast<uint32_t>(m_PointLights.size()));
         gc.PushConstant("constants.showDirectionalLight"_hs, m_ShowDirectionalLight? 1u : 0u);
         gc.PushConstant("constants.showPointLights"_hs, m_ShowPointLights ? 1u : 0u);
         gc.PushConstant("constants.useNormalMaps"_hs, m_UseNormalMaps ? 1u : 0u);
         gc.PushConstant("constants.useDisplacementMaps"_hs, m_UseDisplacementMaps ? 1u : 0u);
         gc.Bind("UBOMatrices"_hs, *m_BufferMatrices);
         gc.Bind("UBODirectionalLight"_hs, *m_BufferDirectionalLight);
         gc.Bind("UBOPointLights"_hs, *m_BufferPointLights);

         // floor
         glm::mat4 model = glm::identity<glm::mat4>();
         gc.PushConstant("constants.model"_hs, model);
         gc.Bind("diffuseMap"_hs, *m_TextureFloor);
         gc.Bind("specularMap"_hs, *m_TextureFloorSpecular);
         gc.Bind("normalMap"_hs, *m_TextureFloorNormal);
         gc.Bind("displacementMap"_hs, *m_TextureFloorDisplacement);
         gc.DrawTriangles(*m_VertexBuffer, 6, 36);

         // cubes
         gc.Bind("diffuseMap"_hs, *m_TextureContainer);
         gc.Bind("specularMap"_hs, *m_TextureContainerSpecular);
         gc.Bind("normalMap"_hs, *m_TextureContainerNormal);
         gc.Bind("displacementMap"_hs, *m_TextureContainerDisplacement);
         for (int i = 0; i < m_CubePositions.size(); ++i) {
            glm::mat4 model = glm::rotate(glm::translate(glm::identity<glm::mat4>(), m_CubePositions[i]), glm::radians(20.0f * i), glm::vec3 {1.0f, 0.3f, 0.5f});
            gc.PushConstant("constants.model"_hs, model);
            gc.DrawTriangles(*m_VertexBuffer, 36);
         }

         gc.EndFrame();
         gc.SwapBuffers();
      }

      GetWindow().BeginFrame();
      Pikzel::GraphicsContext& gc = GetWindow().GetGraphicsContext();
      gc.Bind(*m_PipelineFullScreenQuad);

      gc.Bind("uTexture"_hs, m_FramebufferScene->GetColorTexture(0));
      gc.DrawTriangles(*m_VertexBuffer, 6, 42);

      GetWindow().BeginImGuiFrame();
      {
         ImGui::Begin("Lighting");
         ImGui::Text("Frame time: %.3fms (%.0f FPS)", m_DeltaTime.count() * 1000.0f, 1.0f / m_DeltaTime.count());
         ImGui::Checkbox("Normal mapping", &m_UseNormalMaps);
         ImGui::Checkbox("Displacement mapping", &m_UseDisplacementMaps);
         ImGui::Checkbox("Directional Light", &m_ShowDirectionalLight);
         ImGui::Checkbox("Point Lights", &m_ShowPointLights);
         for (size_t i = 0; i < m_PointLights.size(); ++i) {
            ImGuiDrawPointLight(std::format("light {}", i).c_str(), m_PointLights[i]);
         }
         //ImGui::Text("Depth buffer:");
         //ImVec2 size = ImGui::GetContentRegionAvail();
         //ImGui::Image(m_FramebufferDepth->GetImGuiDepthTextureId(), size, ImVec2 {0, 1}, ImVec2 {1, 0});
         ImGui::End();
      }
      GetWindow().EndImGuiFrame();
      GetWindow().EndFrame();
   }


   virtual void RenderEnd() override {}


   virtual void OnWindowResize(const Pikzel::WindowResizeEvent& event) override {
      super::OnWindowResize(event);
      m_Camera.projection = glm::perspective(m_Camera.fovRadians, static_cast<float>(GetWindow().GetWidth()) / static_cast<float>(GetWindow().GetHeight()), nearPlane, farPlane);

      // recreate framebuffer with new size
      CreateFramebuffers();
   }


private:

   struct Vertex {
      glm::vec3 Pos;
      glm::vec3 Normal;
      glm::vec3 Tangent;
      glm::vec2 TexCoord;
   };

   void CreateVertexBuffer() {
      Vertex vertices[] = {
         // Cube
         {.Pos{-0.5f, -0.5f, -0.5f}, .Normal{ 0.0f,  0.0f, -1.0f}, .Tangent{-1.0f,  0.0f,  0.0f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{ 0.5f,  0.5f, -0.5f}, .Normal{ 0.0f,  0.0f, -1.0f}, .Tangent{-1.0f,  0.0f,  0.0f}, .TexCoord{0.0f, 1.0f}},
         {.Pos{ 0.5f, -0.5f, -0.5f}, .Normal{ 0.0f,  0.0f, -1.0f}, .Tangent{-1.0f,  0.0f,  0.0f}, .TexCoord{0.0f, 0.0f}},
         {.Pos{ 0.5f,  0.5f, -0.5f}, .Normal{ 0.0f,  0.0f, -1.0f}, .Tangent{-1.0f,  0.0f,  0.0f}, .TexCoord{0.0f, 1.0f}},
         {.Pos{-0.5f, -0.5f, -0.5f}, .Normal{ 0.0f,  0.0f, -1.0f}, .Tangent{-1.0f,  0.0f,  0.0f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{-0.5f,  0.5f, -0.5f}, .Normal{ 0.0f,  0.0f, -1.0f}, .Tangent{-1.0f,  0.0f,  0.0f}, .TexCoord{1.0f, 1.0f}},

         {.Pos{-0.5f, -0.5f,  0.5f}, .Normal{ 0.0f,  0.0f,  1.0f}, .Tangent{ 1.0f,  0.0f,  0.0f}, .TexCoord{0.0f, 0.0f}},
         {.Pos{ 0.5f, -0.5f,  0.5f}, .Normal{ 0.0f,  0.0f,  1.0f}, .Tangent{ 1.0f,  0.0f,  0.0f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{ 0.5f,  0.5f,  0.5f}, .Normal{ 0.0f,  0.0f,  1.0f}, .Tangent{ 1.0f,  0.0f,  0.0f}, .TexCoord{1.0f, 1.0f}},
         {.Pos{ 0.5f,  0.5f,  0.5f}, .Normal{ 0.0f,  0.0f,  1.0f}, .Tangent{ 1.0f,  0.0f,  0.0f}, .TexCoord{1.0f, 1.0f}},
         {.Pos{-0.5f,  0.5f,  0.5f}, .Normal{ 0.0f,  0.0f,  1.0f}, .Tangent{ 1.0f,  0.0f,  0.0f}, .TexCoord{0.0f, 1.0f}},
         {.Pos{-0.5f, -0.5f,  0.5f}, .Normal{ 0.0f,  0.0f,  1.0f}, .Tangent{ 1.0f,  0.0f,  0.0f}, .TexCoord{0.0f, 0.0f}},

         {.Pos{-0.5f, -0.5f,  0.5f}, .Normal{-1.0f,  0.0f,  0.0f}, .Tangent{ 0.0f,  0.0f,  1.0f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{-0.5f,  0.5f, -0.5f}, .Normal{-1.0f,  0.0f,  0.0f}, .Tangent{ 0.0f,  0.0f,  1.0f}, .TexCoord{0.0f, 1.0f}},
         {.Pos{-0.5f, -0.5f, -0.5f}, .Normal{-1.0f,  0.0f,  0.0f}, .Tangent{ 0.0f,  0.0f,  1.0f}, .TexCoord{0.0f, 0.0f}},
         {.Pos{-0.5f,  0.5f, -0.5f}, .Normal{-1.0f,  0.0f,  0.0f}, .Tangent{ 0.0f,  0.0f,  1.0f}, .TexCoord{0.0f, 1.0f}},
         {.Pos{-0.5f, -0.5f,  0.5f}, .Normal{-1.0f,  0.0f,  0.0f}, .Tangent{ 0.0f,  0.0f,  1.0f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{-0.5f,  0.5f,  0.5f}, .Normal{-1.0f,  0.0f,  0.0f}, .Tangent{ 0.0f,  0.0f,  1.0f}, .TexCoord{1.0f, 1.0f}},

         {.Pos{ 0.5f,  0.5f,  0.5f}, .Normal{ 1.0f,  0.0f,  0.0f}, .Tangent{ 0.0f,  0.0f, -1.0f}, .TexCoord{0.0f, 1.0f}},
         {.Pos{ 0.5f, -0.5f, -0.5f}, .Normal{ 1.0f,  0.0f,  0.0f}, .Tangent{ 0.0f,  0.0f, -1.0f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{ 0.5f,  0.5f, -0.5f}, .Normal{ 1.0f,  0.0f,  0.0f}, .Tangent{ 0.0f,  0.0f, -1.0f}, .TexCoord{1.0f, 1.0f}},
         {.Pos{ 0.5f, -0.5f, -0.5f}, .Normal{ 1.0f,  0.0f,  0.0f}, .Tangent{ 0.0f,  0.0f, -1.0f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{ 0.5f,  0.5f,  0.5f}, .Normal{ 1.0f,  0.0f,  0.0f}, .Tangent{ 0.0f,  0.0f, -1.0f}, .TexCoord{0.0f, 1.0f}},
         {.Pos{ 0.5f, -0.5f,  0.5f}, .Normal{ 1.0f,  0.0f,  0.0f}, .Tangent{ 0.0f,  0.0f, -1.0f}, .TexCoord{0.0f, 0.0f}},

         {.Pos{-0.5f, -0.5f, -0.5f}, .Normal{ 0.0f, -1.0f,  0.0f}, .Tangent{ 1.0f,  0.0f,  0.0f}, .TexCoord{0.0f, 1.0f}},
         {.Pos{ 0.5f, -0.5f, -0.5f}, .Normal{ 0.0f, -1.0f,  0.0f}, .Tangent{ 1.0f,  0.0f,  0.0f}, .TexCoord{1.0f, 1.0f}},
         {.Pos{ 0.5f, -0.5f,  0.5f}, .Normal{ 0.0f, -1.0f,  0.0f}, .Tangent{ 1.0f,  0.0f,  0.0f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{ 0.5f, -0.5f,  0.5f}, .Normal{ 0.0f, -1.0f,  0.0f}, .Tangent{ 1.0f,  0.0f,  0.0f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{-0.5f, -0.5f,  0.5f}, .Normal{ 0.0f, -1.0f,  0.0f}, .Tangent{ 1.0f,  0.0f,  0.0f}, .TexCoord{0.0f, 0.0f}},
         {.Pos{-0.5f, -0.5f, -0.5f}, .Normal{ 0.0f, -1.0f,  0.0f}, .Tangent{ 1.0f,  0.0f,  0.0f}, .TexCoord{0.0f, 1.0f}},

         {.Pos{-0.5f,  0.5f, -0.5f}, .Normal{ 0.0f,  1.0f,  0.0f}, .Tangent{-1.0f,  0.0f,  0.0f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{ 0.5f,  0.5f,  0.5f}, .Normal{ 0.0f,  1.0f,  0.0f}, .Tangent{-1.0f,  0.0f,  0.0f}, .TexCoord{0.0f, 1.0f}},
         {.Pos{ 0.5f,  0.5f, -0.5f}, .Normal{ 0.0f,  1.0f,  0.0f}, .Tangent{-1.0f,  0.0f,  0.0f}, .TexCoord{0.0f, 0.0f}},
         {.Pos{ 0.5f,  0.5f,  0.5f}, .Normal{ 0.0f,  1.0f,  0.0f}, .Tangent{-1.0f,  0.0f,  0.0f}, .TexCoord{0.0f, 1.0f}},
         {.Pos{-0.5f,  0.5f, -0.5f}, .Normal{ 0.0f,  1.0f,  0.0f}, .Tangent{-1.0f,  0.0f,  0.0f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{-0.5f,  0.5f,  0.5f}, .Normal{ 0.0f,  1.0f,  0.0f}, .Tangent{-1.0f,  0.0f,  0.0f}, .TexCoord{1.0f, 1.0f}},

         // Plane
         {.Pos{-10.0f,  0.0f, -10.0f}, .Normal{ 0.0f,  1.0f,  0.0f}, .Tangent{-1.0f, 0.0f, 0.0f}, .TexCoord{10.0f,  0.0f}},
         {.Pos{ 10.0f,  0.0f,  10.0f}, .Normal{ 0.0f,  1.0f,  0.0f}, .Tangent{-1.0f, 0.0f, 0.0f}, .TexCoord{ 0.0f, 10.0f}},
         {.Pos{ 10.0f,  0.0f, -10.0f}, .Normal{ 0.0f,  1.0f,  0.0f}, .Tangent{-1.0f, 0.0f, 0.0f}, .TexCoord{ 0.0f,  0.0f}},

         {.Pos{ 10.0f,  0.0f,  10.0f}, .Normal{ 0.0f,  1.0f,  0.0f}, .Tangent{-1.0f, 0.0f, 0.0f}, .TexCoord{ 0.0f, 10.0f}},
         {.Pos{-10.0f,  0.0f, -10.0f}, .Normal{ 0.0f,  1.0f,  0.0f}, .Tangent{-1.0f, 0.0f, 0.0f}, .TexCoord{10.0f,  0.0f}},
         {.Pos{-10.0f,  0.0f,  10.0f}, .Normal{ 0.0f,  1.0f,  0.0f}, .Tangent{-1.0f, 0.0f, 0.0f}, .TexCoord{10.0f, 10.0f}},


         // Fullscreen  quad
         {.Pos{ -1.0f,   1.0f,   0.0f}, .Normal{ 0.0f,  1.0f,  0.0f}, .Tangent{1.0f, 0.0f, 0.0f}, .TexCoord{ 0.0f,  1.0f}},
         {.Pos{ -1.0f,  -1.0f,   0.0f}, .Normal{ 0.0f,  1.0f,  0.0f}, .Tangent{1.0f, 0.0f, 0.0f}, .TexCoord{ 0.0f,  0.0f}},
         {.Pos{  1.0f,  -1.0f,   0.0f}, .Normal{ 0.0f,  1.0f,  0.0f}, .Tangent{1.0f, 0.0f, 0.0f}, .TexCoord{ 1.0f,  0.0f}},

         {.Pos{ -1.0f,   1.0f,   0.0f}, .Normal{ 0.0f,  1.0f,  0.0f}, .Tangent{1.0f, 0.0f, 0.0f}, .TexCoord{ 0.0f,  1.0f}},
         {.Pos{  1.0f,  -1.0f,   0.0f}, .Normal{ 0.0f,  1.0f,  0.0f}, .Tangent{1.0f, 0.0f, 0.0f}, .TexCoord{ 1.0f,  0.0f}},
         {.Pos{  1.0f,   1.0f,   0.0f}, .Normal{ 0.0f,  1.0f,  0.0f}, .Tangent{1.0f, 0.0f, 0.0f}, .TexCoord{ 1.0f,  1.0f}},
      };

      Pikzel::BufferLayout layout = {
         {"inPos",       Pikzel::DataType::Vec3},
         {"inNormal",    Pikzel::DataType::Vec3},
         {"inTangent",   Pikzel::DataType::Vec3},
         {"inTexCoords", Pikzel::DataType::Vec2},
      };
      m_VertexBuffer = Pikzel::RenderCore::CreateVertexBuffer(layout, sizeof(vertices), vertices);
   }


   struct Matrices {
      glm::mat4 viewProjection;
      glm::mat4 lightSpace;
      glm::vec3 eyePosition;
   };

   void CreateUniformBuffers() {

      glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, -5.0f, 10.0f);  // TODO: how does one determine the parameters here?
      glm::mat4 lightView = glm::lookAt(-m_DirectionalLights[0].direction, glm::vec3 {0.0f, 0.0f, 0.0f}, glm::vec3 {0.0f, 1.0f, 0.0f});
      m_LightSpace = lightProjection * lightView;

      m_BufferMatrices = Pikzel::RenderCore::CreateUniformBuffer(sizeof(Matrices));
      m_BufferLightViews = Pikzel::RenderCore::CreateUniformBuffer(sizeof(glm::mat4) * m_PointLights.size() * 6);
      m_BufferDirectionalLight = Pikzel::RenderCore::CreateUniformBuffer(sizeof(m_DirectionalLights), m_DirectionalLights);
      m_BufferPointLights = Pikzel::RenderCore::CreateUniformBuffer(m_PointLights.size() * sizeof(Pikzel::PointLight), m_PointLights.data());
   }


   void CreateTextures() {
      m_TextureContainer = Pikzel::RenderCore::CreateTexture({.path = "Assets/" APP_NAME "/Textures/Container.png"});
      m_TextureContainerSpecular = Pikzel::RenderCore::CreateTexture({.path = "Assets/" APP_NAME "/Textures/ContainerSpecular.png", .format = Pikzel::TextureFormat::RGBA8});
      m_TextureContainerNormal = Pikzel::RenderCore::CreateTexture({.path = "Assets/" APP_NAME "/Textures/ContainerNormal.png", .format = Pikzel::TextureFormat::RGBA8});
      m_TextureContainerDisplacement = Pikzel::RenderCore::CreateTexture({.path = "Assets/" APP_NAME "/Textures/ContainerDisplacement.png", .format = Pikzel::TextureFormat::RGBA8});

      m_TextureFloor = Pikzel::RenderCore::CreateTexture({.path = "Assets/" APP_NAME "/Textures/Floor.jpg"});
      m_TextureFloorSpecular = Pikzel::RenderCore::CreateTexture({.path = "Assets/" APP_NAME "/Textures/FloorSpecular.jpg", .format = Pikzel::TextureFormat::RGBA8});
      m_TextureFloorNormal = Pikzel::RenderCore::CreateTexture({.path = "Assets/" APP_NAME "/Textures/FloorNormal.jpg", .format = Pikzel::TextureFormat::RGBA8});
      m_TextureFloorDisplacement = Pikzel::RenderCore::CreateTexture({.path = "Assets/" APP_NAME "/Textures/FloorDisplacement.jpg", .format = Pikzel::TextureFormat::RGBA8});
   }


   void CreateFramebuffers() {
      m_FramebufferScene = Pikzel::RenderCore::CreateFramebuffer({.width = GetWindow().GetWidth(), .height = GetWindow().GetHeight(), .msaaNumSamples = 4, .clearColorValue = GetWindow().GetClearColor()});
   }


   void CreatePipelines() {
      m_PipelineColoredModel = m_FramebufferScene->GetGraphicsContext().CreatePipeline({
         .shaders = {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/ColoredModel.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/ColoredModel.frag.spv" }
         },
         .bufferLayout = m_VertexBuffer->GetLayout()
      });
      m_PipelineLitModel = m_FramebufferScene->GetGraphicsContext().CreatePipeline({
         .shaders = {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/LitModel.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/LitModel.frag.spv" }
         },
         .bufferLayout = m_VertexBuffer->GetLayout()
      });
      m_PipelineFullScreenQuad = GetWindow().GetGraphicsContext().CreatePipeline({
         .shaders = {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/FullScreenQuad.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/FullScreenQuad.frag.spv" }
         },
         .bufferLayout = m_VertexBuffer->GetLayout()
      });
   }


private:
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


private:
   Pikzel::Input m_Input;

   Camera m_Camera = {
      .position = {-10.0f, 5.0f, 0.0f},
      .direction = glm::normalize(glm::vec3{1.0f, -0.5f, 0.0f}),
      .upVector = {0.0f, 1.0f, 0.0f},
      .fovRadians = glm::radians(45.f),
      .moveSpeed = 2.0f,
      .rotateSpeed = 10.0f
   };

   // note: shader expects exactly 1
   Pikzel::DirectionalLight m_DirectionalLights[1] = {
      {
         .direction = { -2.0f, -4.0f, 2.0f},
         .color = Pikzel::sRGB{0.5f, 0.5f, 0.5f},
         .ambient = Pikzel::sRGB{0.01f, 0.01f, 0.01f},
         .size = 0.02
      }
   };

   // note: shader expects 1 to 16
   std::vector<Pikzel::PointLight> m_PointLights = {
      {
         .position = {-2.8f, 2.8f, -1.7f},
         .color = Pikzel::sRGB{1.0f, 1.0f, 1.0f},
         .size = 0.02,
         .power = 20.0f
      }
//      ,{
//         .position = {2.3f, 3.3f, -4.0f},
//         .color = Pikzel::sRGB{0.0f, 1.0f, 0.0f},
//         .Size = 0.02,
//         .Power = 20.0f
//      }
   };
//       {
//          .position = {-4.0f, 2.0f, -12.0f},
//          .color = Pikzel::sRGB{1.0f, 0.0f, 0.0f},
//          .Constant = 1.0f,
//          .Linear = 0.09f,
//          .Quadratic = 0.032f
//       },
//       {
//          .position = {0.0f, 0.0f, -3.0f},
//          .color = Pikzel::sRGB{1.0f, 1.0f, 0.0f},
//          .Constant = 1.0f,
//          .Linear = 0.09f,
//          .Quadratic = 0.032f
//       }
//    };

   std::vector<glm::vec3> m_CubePositions = {
       glm::vec3( 0.0f, 0.5f, 0.0f),
       glm::vec3(2.0f,  1.5f, -1.5f),
   };

   glm::mat4 m_LightSpace;

   std::unique_ptr<Pikzel::VertexBuffer> m_VertexBuffer;
   std::unique_ptr<Pikzel::UniformBuffer> m_BufferMatrices;
   std::unique_ptr<Pikzel::UniformBuffer> m_BufferLightViews;
   std::unique_ptr<Pikzel::UniformBuffer> m_BufferDirectionalLight;
   std::unique_ptr<Pikzel::UniformBuffer> m_BufferPointLights;
   std::unique_ptr<Pikzel::Texture> m_TextureContainer;
   std::unique_ptr<Pikzel::Texture> m_TextureContainerSpecular;
   std::unique_ptr<Pikzel::Texture> m_TextureContainerNormal;
   std::unique_ptr<Pikzel::Texture> m_TextureContainerDisplacement;
   std::unique_ptr<Pikzel::Texture> m_TextureFloor;
   std::unique_ptr<Pikzel::Texture> m_TextureFloorSpecular;
   std::unique_ptr<Pikzel::Texture> m_TextureFloorNormal;
   std::unique_ptr<Pikzel::Texture> m_TextureFloorDisplacement;
   std::unique_ptr<Pikzel::Framebuffer> m_FramebufferScene;
   std::unique_ptr<Pikzel::Pipeline> m_PipelineColoredModel;
   std::unique_ptr<Pikzel::Pipeline> m_PipelineLitModel;
   std::unique_ptr<Pikzel::Pipeline> m_PipelineFullScreenQuad;

   Pikzel::DeltaTime m_DeltaTime = {};
   bool m_ShowDirectionalLight = true;
   bool m_ShowPointLights = true;
   bool m_UseNormalMaps = true;
   bool m_UseDisplacementMaps = true;
};


std::unique_ptr<Pikzel::Application> CreateApplication(int argc, const char* argv[]) {
   return std::make_unique<NormalMaps>();
}
