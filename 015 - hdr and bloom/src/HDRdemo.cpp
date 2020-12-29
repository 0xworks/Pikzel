#include "Pikzel/Pikzel.h"
#include "Pikzel/Core/EntryPoint.h"

constexpr float nearPlane = 0.1f;
constexpr float farPlane = 50.0f;

class HDRdemo final : public Pikzel::Application {
public:
   HDRdemo()
   : Pikzel::Application {{.Title = APP_DESCRIPTION, .ClearColor = Pikzel::sRGB{0.01f, 0.01f, 0.01f}, .IsVSync = true}}
   , m_Input {GetWindow()}
   {
      CreateVertexBuffers();
      CreateUniformBuffers();
      CreateTextures();
      CreateFramebuffers();
      CreatePipelines();

      m_Camera.Projection = glm::perspective(m_Camera.FoVRadians, static_cast<float>(GetWindow().GetWidth()) / static_cast<float>(GetWindow().GetHeight()), nearPlane, farPlane);

      Pikzel::ImGuiEx::Init(GetWindow());
   }


protected:

   virtual void Update(const Pikzel::DeltaTime deltaTime) override {
      PKZL_PROFILE_FUNCTION();
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

      auto start = std::chrono::steady_clock::now();

      // update buffers
      Matrices matrices;
      matrices.viewProjection = m_Camera.Projection * glm::lookAt(m_Camera.Position, m_Camera.Position + m_Camera.Direction, m_Camera.UpVector);
      matrices.lightSpace = m_LightSpace;
      matrices.eyePosition = m_Camera.Position;
      m_BufferMatrices->CopyFromHost(0, sizeof(Matrices), &matrices);
      m_BufferPointLights->CopyFromHost(0, sizeof(Pikzel::PointLight) * m_PointLights.size(), m_PointLights.data());

      // render to directional light shadow map
      if(m_ShowDirectionalLight) {
         Pikzel::GraphicsContext& gc = m_FramebufferDirShadow->GetGraphicsContext();
         gc.BeginFrame();
         gc.Bind(*m_PipelineDirShadow);

         // floor
         glm::mat4 model = glm::identity<glm::mat4>();
         gc.PushConstant("constants.mvp"_hs, m_LightSpace * model);
         gc.DrawTriangles(*m_VertexBuffer, 6, 36);

         // cubes
         for (int i = 0; i < m_CubePositions.size(); ++i) {
            glm::mat4 model = glm::rotate(glm::translate(glm::identity<glm::mat4>(), m_CubePositions[i]), glm::radians(20.0f * i), glm::vec3 {1.0f, 0.3f, 0.5f});
            gc.PushConstant("constants.mvp"_hs, m_LightSpace * model);
            gc.DrawTriangles(*m_VertexBuffer, 36);
         }

         gc.EndFrame();
         gc.SwapBuffers();
      }

      // render to point light shadow map
      if(m_ShowPointLights) {
         Pikzel::GraphicsContext& gc = m_FramebufferPtShadow->GetGraphicsContext();

         for (int i = 0; i < m_PointLights.size(); ++i) {
            auto& light = m_PointLights[i];

            std::array<glm::mat4, 6> lightViews = {
               lightProjection * glm::lookAt(light.Position, light.Position + glm::vec3 {1.0f,  0.0f,  0.0f}, glm::vec3 {0.0f, -1.0f,  0.0f}),
               lightProjection * glm::lookAt(light.Position, light.Position + glm::vec3 {-1.0f,  0.0f,  0.0f}, glm::vec3 {0.0f, -1.0f,  0.0f}),
               lightProjection * glm::lookAt(light.Position, light.Position + glm::vec3 {0.0f,  1.0f,  0.0f}, glm::vec3 {0.0f,  0.0f,  1.0f}),
               lightProjection * glm::lookAt(light.Position, light.Position + glm::vec3 {0.0f, -1.0f,  0.0f}, glm::vec3 {0.0f,  0.0f, -1.0f}),
               lightProjection * glm::lookAt(light.Position, light.Position + glm::vec3 {0.0f,  0.0f,  1.0f}, glm::vec3 {0.0f, -1.0f,  0.0f}),
               lightProjection * glm::lookAt(light.Position, light.Position + glm::vec3 {0.0f,  0.0f, -1.0f}, glm::vec3 {0.0f, -1.0f,  0.0f}),
            };
            m_BufferLightViews->CopyFromHost(0, sizeof(glm::mat4) * lightViews.size(), lightViews.data());

            gc.BeginFrame(i == 0? Pikzel::BeginFrameOp::ClearAll : Pikzel::BeginFrameOp::ClearNone);
            gc.Bind(*m_PipelinePtShadow);
            gc.PushConstant("constants.lightIndex"_hs, i);
            gc.PushConstant("constants.lightRadius"_hs, lightRadius);
            gc.Bind(*m_BufferLightViews, "UBOLightViews"_hs);
            gc.Bind(*m_BufferPointLights, "UBOPointLights"_hs);

            // floor
            glm::mat4 model = glm::identity<glm::mat4>();
            gc.PushConstant("constants.model"_hs, model);
            gc.DrawTriangles(*m_VertexBuffer, 6, 36);

            // cubes
            for (int i = 0; i < m_CubePositions.size(); ++i) {
               glm::mat4 model = glm::rotate(glm::translate(glm::identity<glm::mat4>(), m_CubePositions[i]), glm::radians(20.0f * i), glm::vec3 {1.0f, 0.3f, 0.5f});
               gc.PushConstant("constants.model"_hs, model);
               gc.DrawTriangles(*m_VertexBuffer, 36);
            }

            gc.EndFrame();
            gc.SwapBuffers();
         }
      }

      // render scene
      {

         Pikzel::GraphicsContext& gc = m_FramebufferScene->GetGraphicsContext();
         gc.BeginFrame();

         if (m_ShowPointLights) {
            gc.Bind(*m_PipelineColoredModel);
            for (const auto& pointLight : m_PointLights) {
               glm::mat4 model = glm::scale(glm::translate(glm::identity<glm::mat4>(), pointLight.Position), {pointLight.Size,pointLight.Size,pointLight.Size});
               gc.PushConstant("constants.mvp"_hs, matrices.viewProjection * model);
               gc.PushConstant("constants.color"_hs, pointLight.Color * pointLight.Power);
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
         gc.Bind(*m_BufferMatrices, "UBOMatrices"_hs);
         gc.Bind(*m_BufferDirectionalLight, "UBODirectionalLight"_hs);
         gc.Bind(*m_BufferPointLights, "UBOPointLights"_hs);
         gc.Bind(m_FramebufferDirShadow->GetDepthTexture(), "dirShadowMap"_hs);
         gc.Bind(m_FramebufferPtShadow->GetDepthTexture(), "ptShadowMap"_hs);

         // floor
         glm::mat4 model = glm::identity<glm::mat4>();
         gc.Bind(*m_TextureFloor, "diffuseMap"_hs);
         gc.Bind(*m_TextureFloorSpecular, "specularMap"_hs);
         gc.Bind(*m_TextureFloorNormal, "normalMap"_hs);
         gc.Bind(*m_TextureFloorDisplacement, "displacementMap"_hs);
         gc.PushConstant("constants.model"_hs, model);
         gc.DrawTriangles(*m_VertexBuffer, 6, 36);

         // cubes
         gc.Bind(*m_TextureContainer, "diffuseMap"_hs);
         gc.Bind(*m_TextureContainerSpecular, "specularMap"_hs);
         gc.Bind(*m_TextureContainerNormal, "normalMap"_hs);
         gc.Bind(*m_TextureContainerDisplacement, "displacementMap"_hs);
         for (int i = 0; i < m_CubePositions.size(); ++i) {
            glm::mat4 model = glm::rotate(glm::translate(glm::identity<glm::mat4>(), m_CubePositions[i]), glm::radians(20.0f * i), glm::vec3 {1.0f, 0.3f, 0.5f});
            gc.PushConstant("constants.model"_hs, model);
            gc.DrawTriangles(*m_VertexBuffer, 36);
         }

         gc.EndFrame();
         gc.SwapBuffers();
      }

      // bloom post processing
      bool horizontal = true;
      if(m_Bloom) {
         bool firstIteration = true;
         uint32_t blurIterations = 10;

         for (uint32_t i = 0; i < blurIterations; ++i) {
            Pikzel::GraphicsContext& gc = m_FramebufferBlur[horizontal]->GetGraphicsContext();
            gc.BeginFrame();

            gc.Bind(*m_PipelineBlur);

            gc.PushConstant("constants.horizontal"_hs, horizontal ? 1u : 0u);
            if (firstIteration) {
               gc.Bind(m_FramebufferScene->GetColorTexture(1), "uTexture"_hs);
               firstIteration = false;
            } else {
               gc.Bind(m_FramebufferBlur[!horizontal]->GetColorTexture(0), "uTexture"_hs);
            }
            gc.DrawTriangles(*m_QuadVertexBuffer, 6);

            gc.EndFrame();
            gc.SwapBuffers();
            horizontal = !horizontal;
         }
      }

      GetWindow().BeginFrame();
      Pikzel::GraphicsContext& gc = GetWindow().GetGraphicsContext();
      gc.Bind(*m_PipelineScreenQuad);
      gc.PushConstant("constants.bloom"_hs, m_Bloom ? 1 : 0);
      gc.PushConstant("constants.tonemap"_hs, m_ToneMap);
      gc.PushConstant("constants.exposure"_hs, m_Exposure);
      gc.Bind(m_FramebufferScene->GetColorTexture(0), "uTexture"_hs);
      gc.Bind(m_FramebufferBlur[!horizontal]->GetColorTexture(0), "uBloom"_hs);
      gc.DrawTriangles(*m_QuadVertexBuffer, 6);

      auto end = std::chrono::steady_clock::now();
      std::chrono::duration<double, std::milli> ms = end - start;

      GetWindow().BeginImGuiFrame();
      {
         ImGui::Begin("Lighting");
         ImGui::Checkbox("Bloom", &m_Bloom);
         ImGui::Text("Tone mapping:");
         ImGui::RadioButton("None", &m_ToneMap, 0);
         ImGui::RadioButton("Reinhard", &m_ToneMap, 1);
         ImGui::RadioButton("Exposure", &m_ToneMap, 2);
         ImGui::SameLine();
         ImGui::DragFloat("", &m_Exposure, 0.1f, 0, 10);
         ImGui::Checkbox("Normal mapping", &m_UseNormalMaps);
         ImGui::Checkbox("Displacement mapping", &m_UseDisplacementMaps);
         ImGui::Checkbox("Directional Light", &m_ShowDirectionalLight);
         ImGui::Checkbox("Point Lights", &m_ShowPointLights);
         for (size_t i = 0; i < m_PointLights.size(); ++i) {
            ImGuiDrawPointLight(fmt::format("light {0}", i).c_str(), m_PointLights[i]);
         }
         ImGui::Text("Frame time: %.3fms (%.0f FPS)", ms.count(), 1000.0 / ms.count());
         //ImGui::Text("Brightness color buffer:");
         //ImVec2 size = ImGui::GetContentRegionAvail();
         //ImGui::Image(m_FramebufferScene->GetImGuiColorTextureId(1), size, ImVec2 {0, 1}, ImVec2 {1, 0});
         ImGui::End();
      }
      GetWindow().EndImGuiFrame();
      GetWindow().EndFrame();
   }


   virtual void RenderEnd() override {}


   virtual void OnWindowResize(const Pikzel::WindowResizeEvent& event) override {
      __super::OnWindowResize(event);
      m_Camera.Projection = glm::perspective(m_Camera.FoVRadians, static_cast<float>(GetWindow().GetWidth()) / static_cast<float>(GetWindow().GetHeight()), nearPlane, farPlane);

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

   struct QuadVertex {
      glm::vec3 Pos;
      glm::vec2 TexCoord;
   };

   void CreateVertexBuffers() {
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
      };

      QuadVertex quadVertices[] = {
         // Fullscreen  quad
         {.Pos {-1.0f,   1.0f,   0.0f}, .TexCoord {0.0f,  1.0f}},
         {.Pos{ -1.0f,  -1.0f,   0.0f}, .TexCoord{ 0.0f,  0.0f}},
         {.Pos{  1.0f,  -1.0f,   0.0f}, .TexCoord{ 1.0f,  0.0f}},

         {.Pos{ -1.0f,   1.0f,   0.0f}, .TexCoord{ 0.0f,  1.0f}},
         {.Pos{  1.0f,  -1.0f,   0.0f}, .TexCoord{ 1.0f,  0.0f}},
         {.Pos{  1.0f,   1.0f,   0.0f}, .TexCoord{ 1.0f,  1.0f}},
      };

      m_VertexBuffer = Pikzel::RenderCore::CreateVertexBuffer(sizeof(vertices), vertices);
      m_VertexBuffer->SetLayout({
         { "inPos",      Pikzel::DataType::Vec3 },
         { "inNormal",   Pikzel::DataType::Vec3 },
         { "inTangent",  Pikzel::DataType::Vec3 },
         { "inTexCoord", Pikzel::DataType::Vec2 }
      });

      m_QuadVertexBuffer = Pikzel::RenderCore::CreateVertexBuffer(sizeof(quadVertices), quadVertices);
      m_QuadVertexBuffer->SetLayout({
         { "inPos",      Pikzel::DataType::Vec3 },
         { "inTexCoord", Pikzel::DataType::Vec2 }
      });

   }


   struct Matrices {
      glm::mat4 viewProjection;
      glm::mat4 lightSpace;
      glm::vec3 eyePosition;
   };

   void CreateUniformBuffers() {

      glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, -5.0f, 10.0f);  // TODO: how does one determine the parameters here?
      glm::mat4 lightView = glm::lookAt(-m_DirectionalLights[0].Direction, glm::vec3 {0.0f, 0.0f, 0.0f}, glm::vec3 {0.0f, 1.0f, 0.0f});
      m_LightSpace = lightProjection * lightView;

      m_BufferMatrices = Pikzel::RenderCore::CreateUniformBuffer(sizeof(Matrices));
      m_BufferLightViews = Pikzel::RenderCore::CreateUniformBuffer(sizeof(glm::mat4) * m_PointLights.size() * 6);
      m_BufferDirectionalLight = Pikzel::RenderCore::CreateUniformBuffer(sizeof(m_DirectionalLights), m_DirectionalLights);
      m_BufferPointLights = Pikzel::RenderCore::CreateUniformBuffer(m_PointLights.size() * sizeof(Pikzel::PointLight), m_PointLights.data());
   }


   void CreateTextures() {
      m_TextureContainer = Pikzel::RenderCore::CreateTexture2D("Assets/" APP_NAME "/Textures/Container.png");
      m_TextureContainerSpecular = Pikzel::RenderCore::CreateTexture2D("Assets/" APP_NAME "/Textures/ContainerSpecular.png", false);
      m_TextureContainerNormal = Pikzel::RenderCore::CreateTexture2D("Assets/" APP_NAME "/Textures/ContainerNormal.png", false);
      m_TextureContainerDisplacement = Pikzel::RenderCore::CreateTexture2D("Assets/" APP_NAME "/Textures/ContainerDisplacement.png", false);

      m_TextureFloor = Pikzel::RenderCore::CreateTexture2D("Assets/" APP_NAME "/Textures/Floor.jpg");
      m_TextureFloorSpecular = Pikzel::RenderCore::CreateTexture2D("Assets/" APP_NAME "/Textures/FloorSpecular.jpg", false);
      m_TextureFloorNormal = Pikzel::RenderCore::CreateTexture2D("Assets/" APP_NAME "/Textures/FloorNormal.jpg", false);
      m_TextureFloorDisplacement = Pikzel::RenderCore::CreateTexture2D("Assets/" APP_NAME "/Textures/FloorDisplacement.jpg", false);
   }


   void CreateFramebuffers() {
      const uint32_t shadowMapWidth = 2048;
      const uint32_t shadowMapHeight = 2048;

      m_FramebufferScene = Pikzel::RenderCore::CreateFramebuffer({
         .Width = GetWindow().GetWidth(),
         .Height = GetWindow().GetHeight(),
         .MSAANumSamples = 4,
         .ClearColor = GetWindow().GetClearColor(),
         .Attachments = {
            {Pikzel::AttachmentType::Color, Pikzel::TextureFormat::RGBA16F},
            {Pikzel::AttachmentType::Color, Pikzel::TextureFormat::RGBA16F},
            {Pikzel::AttachmentType::Depth, Pikzel::TextureFormat::D32F}
         }
      });


      m_FramebufferBlur[0] = Pikzel::RenderCore::CreateFramebuffer({
         .Width = GetWindow().GetWidth(),
         .Height = GetWindow().GetHeight(),
         .ClearColor = GetWindow().GetClearColor(),
         .Attachments = {
            {Pikzel::AttachmentType::Color, Pikzel::TextureFormat::RGBA16F},
         }
      });

      m_FramebufferBlur[1] = Pikzel::RenderCore::CreateFramebuffer({
         .Width = GetWindow().GetWidth(),
         .Height = GetWindow().GetHeight(),
         .ClearColor = GetWindow().GetClearColor(),
         .Attachments = {
            {Pikzel::AttachmentType::Color, Pikzel::TextureFormat::RGBA16F},
         }
      });

      if (!m_FramebufferDirShadow) {
         m_FramebufferDirShadow = Pikzel::RenderCore::CreateFramebuffer({
            .Width = shadowMapWidth,
            .Height = shadowMapHeight,
            .Attachments = {{Pikzel::AttachmentType::Depth, Pikzel::TextureFormat::D32F}}
         });
      }

      if (!m_FramebufferPtShadow) {
         m_FramebufferPtShadow = Pikzel::RenderCore::CreateFramebuffer({
            .Width = shadowMapWidth,
            .Height = shadowMapHeight,
            .Layers = static_cast<uint32_t>(m_PointLights.size()),
            .Attachments = {{Pikzel::AttachmentType::Depth, Pikzel::TextureFormat::D32F, Pikzel::TextureType::TextureCubeArray}}
         });
      }
   }


   void CreatePipelines() {
      m_PipelineDirShadow = m_FramebufferDirShadow->GetGraphicsContext().CreatePipeline({
         m_VertexBuffer->GetLayout(),
         {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/Depth.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/Depth.frag.spv" }
         }
      });
      m_PipelinePtShadow = m_FramebufferPtShadow->GetGraphicsContext().CreatePipeline({
         m_VertexBuffer->GetLayout(),
         {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/DepthCube.vert.spv" },
            { Pikzel::ShaderType::Geometry, "Assets/" APP_NAME "/Shaders/DepthCube.geom.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/DepthCube.frag.spv" }
         }
      });
      m_PipelineColoredModel = m_FramebufferScene->GetGraphicsContext().CreatePipeline({
         m_VertexBuffer->GetLayout(),
         {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/ColoredModel.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/ColoredModel.frag.spv" }
         }
      });
      m_PipelineLitModel = m_FramebufferScene->GetGraphicsContext().CreatePipeline({
         m_VertexBuffer->GetLayout(),
         {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/LitModel.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/LitModel.frag.spv" }
         }
      });
      m_PipelineBlur = m_FramebufferBlur[0]->GetGraphicsContext().CreatePipeline({
         m_QuadVertexBuffer->GetLayout(),
         {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/Quad.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/QuadGaussianBlur.frag.spv" }
         }
      });
      m_PipelineScreenQuad = GetWindow().GetGraphicsContext().CreatePipeline({
         m_QuadVertexBuffer->GetLayout(),
         {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/Quad.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/QuadCombine.frag.spv" }
         }
      });
   }


private:
   static void ImGuiDrawPointLight(const char* label, Pikzel::PointLight& pointLight) {
      ImGui::PushID(label);
      if (ImGui::TreeNode(label)) {
         Pikzel::ImGuiEx::EditVec3("Position", &pointLight.Position);
         Pikzel::ImGuiEx::EditVec3Color("Color", &pointLight.Color);
         Pikzel::ImGuiEx::EditFloat("Size", &pointLight.Size);
         Pikzel::ImGuiEx::EditFloat("Power", &pointLight.Power);
         ImGui::TreePop();
      }
      ImGui::PopID();
   }


private:
   Pikzel::Input m_Input;

   Camera m_Camera = {
      .Position = {-10.0f, 5.0f, 0.0f},
      .Direction = glm::normalize(glm::vec3{1.0f, -0.5f, 0.0f}),
      .UpVector = {0.0f, 1.0f, 0.0f},
      .FoVRadians = glm::radians(45.f),
      .MoveSpeed = 2.0f,
      .RotateSpeed = 10.0f
   };

   // note: shader expects exactly 1
   Pikzel::DirectionalLight m_DirectionalLights[1] = {
      {
         .Direction = { -2.0f, -4.0f, 2.0f},
         .Color = Pikzel::sRGB{0.0f, 0.0f, 0.0f},
         .Ambient = Pikzel::sRGB{0.1f, 0.1f, 0.1f},
         .Size = 0.02
      }
   };

   // note: shader expects 1 to 16
   std::vector<Pikzel::PointLight> m_PointLights = {
      {
         .Position = {-2.8f, 2.8f, -1.7f},
         .Color = Pikzel::sRGB{1.0f, 1.0f, 1.0f},
         .Size = 0.02,
         .Power = 20.0f
      }
      ,{
         .Position = {2.3f, 3.3f, -4.0f},
         .Color = Pikzel::sRGB{0.0f, 1.0f, 0.0f},
         .Size = 0.02,
         .Power = 20.0f
      }
   };
//       {
//          .Position = {-4.0f, 2.0f, -12.0f},
//          .Color = Pikzel::sRGB{1.0f, 0.0f, 0.0f},
//          .Constant = 1.0f,
//          .Linear = 0.09f,
//          .Quadratic = 0.032f
//       },
//       {
//          .Position = {0.0f, 0.0f, -3.0f},
//          .Color = Pikzel::sRGB{1.0f, 1.0f, 0.0f},
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
   std::unique_ptr<Pikzel::VertexBuffer> m_QuadVertexBuffer;
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
   std::unique_ptr<Pikzel::Framebuffer> m_FramebufferDirShadow;
   std::unique_ptr<Pikzel::Framebuffer> m_FramebufferPtShadow;
   std::array<std::unique_ptr<Pikzel::Framebuffer>, 2> m_FramebufferBlur;
   std::unique_ptr<Pikzel::Framebuffer> m_FramebufferScene;
   std::unique_ptr<Pikzel::Pipeline> m_PipelineDirShadow;
   std::unique_ptr<Pikzel::Pipeline> m_PipelinePtShadow;
   std::unique_ptr<Pikzel::Pipeline> m_PipelineColoredModel;
   std::unique_ptr<Pikzel::Pipeline> m_PipelineLitModel;
   std::unique_ptr<Pikzel::Pipeline> m_PipelineScreenQuad;
   std::unique_ptr<Pikzel::Pipeline> m_PipelineBlur;

   float m_Exposure = 1.0;
   int m_ToneMap = 0;
   bool m_Bloom = true;
   bool m_ShowDirectionalLight = true;
   bool m_ShowPointLights = true;
   bool m_UseNormalMaps = true;
   bool m_UseDisplacementMaps = true;
};


std::unique_ptr<Pikzel::Application> CreateApplication(int argc, const char* argv[]) {
   return std::make_unique<HDRdemo>();
}