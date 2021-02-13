#include "Pikzel/Pikzel.h"
#include "Pikzel/Core/EntryPoint.h"

// note: Pikzel uses reverse-Z so near and far planes are swapped
const float nearPlane = 50.0f;
const float farPlane = 0.1f;

class Shadows final : public Pikzel::Application {
public:
   Shadows()
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
      static glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), 1.0f, lightRadius, 0.1f); // note: Pikzel uses reverse-Z so near and far planes are swapped


      // Note: This example illustrates the basic idea of shadow maps, namely that you first render the scene to
      //       an offscreen depth buffer, and then render the scene for real, sampling from the depth buffer to figure
      //       out which fragments are in shadow.
      //
      //       The actual implementation here is not how you'd really do things though, especially not in Vulkan.
      //       Instead of these completely separate "framebuffers", better would be one render pass (using subpasses for the depth)
      //       Secondly, for the point light shadows, it might be better to use multi-views rather than the geometry shader
      //       used here.
      //
      //       When/if I get around to writing some higher level "scene renderer" classes, I will deal with these issues then.

      // update buffers
      Matrices matrices;
      matrices.viewProjection = m_Camera.projection * glm::lookAt(m_Camera.position, m_Camera.position + m_Camera.direction, m_Camera.upVector);
      matrices.lightSpace = m_LightSpace;
      matrices.eyePosition = m_Camera.position;
      m_BufferMatrices->CopyFromHost(0, sizeof(Matrices), &matrices);
      m_BufferPointLights->CopyFromHost(0, sizeof(Pikzel::PointLight) * m_PointLights.size(), m_PointLights.data());

      // render to directional light shadow map
      if (m_ShowDirectionalLight) {
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
      if (m_ShowPointLights) {
         Pikzel::GraphicsContext& gc = m_FramebufferPtShadow->GetGraphicsContext();

         for (int i = 0; i < m_PointLights.size(); ++i) {
            auto& light = m_PointLights[i];

            std::array<glm::mat4, 6> lightViews = {
               lightProjection * glm::lookAt(light.position, light.position + glm::vec3 {1.0f,  0.0f,  0.0f}, glm::vec3 {0.0f, -1.0f,  0.0f}),
               lightProjection * glm::lookAt(light.position, light.position + glm::vec3 {-1.0f,  0.0f,  0.0f}, glm::vec3 {0.0f, -1.0f,  0.0f}),
               lightProjection * glm::lookAt(light.position, light.position + glm::vec3 {0.0f,  1.0f,  0.0f}, glm::vec3 {0.0f,  0.0f,  1.0f}),
               lightProjection * glm::lookAt(light.position, light.position + glm::vec3 {0.0f, -1.0f,  0.0f}, glm::vec3 {0.0f,  0.0f, -1.0f}),
               lightProjection * glm::lookAt(light.position, light.position + glm::vec3 {0.0f,  0.0f,  1.0f}, glm::vec3 {0.0f, -1.0f,  0.0f}),
               lightProjection * glm::lookAt(light.position, light.position + glm::vec3 {0.0f,  0.0f, -1.0f}, glm::vec3 {0.0f, -1.0f,  0.0f}),
            };
            m_BufferLightViews->CopyFromHost(0, sizeof(glm::mat4) * lightViews.size(), lightViews.data());

            gc.BeginFrame(i == 0? Pikzel::BeginFrameOp::ClearAll : Pikzel::BeginFrameOp::ClearNone);
            gc.Bind(*m_PipelinePtShadow);
            gc.PushConstant("constants.lightIndex"_hs, i);
            gc.PushConstant("constants.lightRadius"_hs, lightRadius);
            gc.Bind("UBOLightViews"_hs, *m_BufferLightViews);
            gc.Bind("UBOPointLights"_hs, *m_BufferPointLights);

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

         gc.Bind(*m_PipelineColoredModel);
         for (const auto& pointLight : m_PointLights) {
            glm::mat4 model = glm::scale(glm::translate(glm::identity<glm::mat4>(), pointLight.position), {0.05f, 0.05f, 0.05f});
            gc.PushConstant("constants.mvp"_hs, matrices.viewProjection * model);
            gc.PushConstant("constants.color"_hs, pointLight.color);
            gc.DrawTriangles(*m_VertexBuffer, 36);
         }

         gc.Bind(*m_PipelineLitModel);
         gc.PushConstant("constants.lightRadius"_hs, lightRadius);
         gc.PushConstant("constants.numPointLights"_hs, static_cast<uint32_t>(m_PointLights.size()));
         gc.PushConstant("constants.showDirectionalLight"_hs, m_ShowDirectionalLight ? 1u : 0u);
         gc.PushConstant("constants.showPointLights"_hs, m_ShowPointLights ? 1u : 0u);
         gc.PushConstant("constants.usePCSS"_hs, m_UsePCSS? 1u : 0u);
         gc.Bind("UBOMatrices"_hs, *m_BufferMatrices);
         gc.Bind("UBODirectionalLight"_hs, *m_BufferDirectionalLight);
         gc.Bind("UBOPointLights"_hs, *m_BufferPointLights);
         gc.Bind("dirShadowMap"_hs, m_FramebufferDirShadow->GetDepthTexture());
         gc.Bind("ptShadowMap"_hs, m_FramebufferPtShadow->GetDepthTexture());

         // floor
         glm::mat4 model = glm::identity<glm::mat4>();
         glm::mat4 modelInvTrans = glm::mat4(glm::transpose(glm::inverse(glm::mat3(model))));
         gc.PushConstant("constants.model"_hs, model);
         gc.PushConstant("constants.modelInvTrans"_hs, modelInvTrans);
         gc.Bind("diffuseMap"_hs, *m_TextureFloor);
         gc.Bind("specularMap"_hs, *m_TextureFloorSpecular);
         gc.DrawTriangles(*m_VertexBuffer, 6, 36);

         // cubes
         gc.Bind("diffuseMap"_hs, *m_TextureContainer);
         gc.Bind("specularMap"_hs, *m_TextureContainerSpecular);
         for (int i = 0; i < m_CubePositions.size(); ++i) {
            glm::mat4 model = glm::rotate(glm::translate(glm::identity<glm::mat4>(), m_CubePositions[i]), glm::radians(20.0f * i), glm::vec3 {1.0f, 0.3f, 0.5f});
            glm::mat4 modelInvTrans = glm::mat4(glm::transpose(glm::inverse(glm::mat3(model))));
            gc.PushConstant("constants.model"_hs, model);
            gc.PushConstant("constants.modelInvTrans"_hs, modelInvTrans);
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
         ImGui::Begin("Lighting:");
         ImGui::Text("Frame time: %.3fms (%.0f FPS)", m_DeltaTime.count() * 1000.0f, 1.0f / m_DeltaTime.count());
         ImGui::Checkbox("Directional Light", &m_ShowDirectionalLight);
         ImGui::Checkbox("Point Lights", &m_ShowPointLights);
         ImGui::Checkbox("Soft Shadows", &m_UsePCSS);
         for (size_t i = 0; i < m_PointLights.size(); ++i) {
            ImGuiDrawPointLight(fmt::format("light {0}", i).c_str(), m_PointLights[i]);
         }
         ImGui::Text("Depth buffer:");
         ImVec2 size = ImGui::GetContentRegionAvail();
         ImGui::Image(m_FramebufferDirShadow->GetImGuiDepthTextureId(), size, ImVec2 {0, 1}, ImVec2 {1, 0});
         ImGui::End();
      }
      GetWindow().EndImGuiFrame();
      GetWindow().EndFrame();
   }


   virtual void RenderEnd() override {}


   virtual void OnWindowResize(const Pikzel::WindowResizeEvent& event) override {
      __super::OnWindowResize(event);
      m_Camera.projection = glm::perspective(m_Camera.fovRadians, static_cast<float>(GetWindow().GetWidth()) / static_cast<float>(GetWindow().GetHeight()), nearPlane, farPlane);

      // recreate framebuffer with new size
      CreateFramebuffers();
   }


private:

   struct Vertex {
      glm::vec3 Pos;
      glm::vec3 Normal;
      glm::vec2 TexCoord;
   };

   void CreateVertexBuffer() {
      Vertex vertices[] = {
         // Cube
         {.Pos{ -0.5f,  -0.5f,  -0.5f}, .Normal{ 0.0f,  0.0f, -1.0f}, .TexCoord{ 0.0f,  0.0f}},
         {.Pos{  0.5f,   0.5f,  -0.5f}, .Normal{ 0.0f,  0.0f, -1.0f}, .TexCoord{ 1.0f,  1.0f}},
         {.Pos{  0.5f,  -0.5f,  -0.5f}, .Normal{ 0.0f,  0.0f, -1.0f}, .TexCoord{ 1.0f,  0.0f}},
         {.Pos{  0.5f,   0.5f,  -0.5f}, .Normal{ 0.0f,  0.0f, -1.0f}, .TexCoord{ 1.0f,  1.0f}},
         {.Pos{ -0.5f,  -0.5f,  -0.5f}, .Normal{ 0.0f,  0.0f, -1.0f}, .TexCoord{ 0.0f,  0.0f}},
         {.Pos{ -0.5f,   0.5f,  -0.5f}, .Normal{ 0.0f,  0.0f, -1.0f}, .TexCoord{ 0.0f,  1.0f}},

         {.Pos{ -0.5f,  -0.5f,   0.5f}, .Normal{ 0.0f,  0.0f,  1.0f}, .TexCoord{ 0.0f,  0.0f}},
         {.Pos{  0.5f,  -0.5f,   0.5f}, .Normal{ 0.0f,  0.0f,  1.0f}, .TexCoord{ 1.0f,  0.0f}},
         {.Pos{  0.5f,   0.5f,   0.5f}, .Normal{ 0.0f,  0.0f,  1.0f}, .TexCoord{ 1.0f,  1.0f}},
         {.Pos{  0.5f,   0.5f,   0.5f}, .Normal{ 0.0f,  0.0f,  1.0f}, .TexCoord{ 1.0f,  1.0f}},
         {.Pos{ -0.5f,   0.5f,   0.5f}, .Normal{ 0.0f,  0.0f,  1.0f}, .TexCoord{ 0.0f,  1.0f}},
         {.Pos{ -0.5f,  -0.5f,   0.5f}, .Normal{ 0.0f,  0.0f,  1.0f}, .TexCoord{ 0.0f,  0.0f}},

         {.Pos{ -0.5f,   0.5f,   0.5f}, .Normal{-1.0f,  0.0f,  0.0f}, .TexCoord{ 1.0f,  0.0f}},
         {.Pos{ -0.5f,   0.5f,  -0.5f}, .Normal{-1.0f,  0.0f,  0.0f}, .TexCoord{ 1.0f,  1.0f}},
         {.Pos{ -0.5f,  -0.5f,  -0.5f}, .Normal{-1.0f,  0.0f,  0.0f}, .TexCoord{ 0.0f,  1.0f}},
         {.Pos{ -0.5f,  -0.5f,  -0.5f}, .Normal{-1.0f,  0.0f,  0.0f}, .TexCoord{ 0.0f,  1.0f}},
         {.Pos{ -0.5f,  -0.5f,   0.5f}, .Normal{-1.0f,  0.0f,  0.0f}, .TexCoord{ 0.0f,  0.0f}},
         {.Pos{ -0.5f,   0.5f,   0.5f}, .Normal{-1.0f,  0.0f,  0.0f}, .TexCoord{ 1.0f,  0.0f}},

         {.Pos{  0.5f,   0.5f,   0.5f}, .Normal{ 1.0f,  0.0f,  0.0f}, .TexCoord{ 1.0f,  0.0f}},
         {.Pos{  0.5f,  -0.5f,  -0.5f}, .Normal{ 1.0f,  0.0f,  0.0f}, .TexCoord{ 0.0f,  1.0f}},
         {.Pos{  0.5f,   0.5f,  -0.5f}, .Normal{ 1.0f,  0.0f,  0.0f}, .TexCoord{ 1.0f,  1.0f}},
         {.Pos{  0.5f,  -0.5f,  -0.5f}, .Normal{ 1.0f,  0.0f,  0.0f}, .TexCoord{ 0.0f,  1.0f}},
         {.Pos{  0.5f,   0.5f,   0.5f}, .Normal{ 1.0f,  0.0f,  0.0f}, .TexCoord{ 1.0f,  0.0f}},
         {.Pos{  0.5f,  -0.5f,   0.5f}, .Normal{ 1.0f,  0.0f,  0.0f}, .TexCoord{ 0.0f,  0.0f}},

         {.Pos{ -0.5f,  -0.5f,  -0.5f}, .Normal{ 0.0f, -1.0f,  0.0f}, .TexCoord{ 0.0f,  1.0f}},
         {.Pos{  0.5f,  -0.5f,  -0.5f}, .Normal{ 0.0f, -1.0f,  0.0f}, .TexCoord{ 1.0f,  1.0f}},
         {.Pos{  0.5f,  -0.5f,   0.5f}, .Normal{ 0.0f, -1.0f,  0.0f}, .TexCoord{ 1.0f,  0.0f}},
         {.Pos{  0.5f,  -0.5f,   0.5f}, .Normal{ 0.0f, -1.0f,  0.0f}, .TexCoord{ 1.0f,  0.0f}},
         {.Pos{ -0.5f,  -0.5f,   0.5f}, .Normal{ 0.0f, -1.0f,  0.0f}, .TexCoord{ 0.0f,  0.0f}},
         {.Pos{ -0.5f,  -0.5f,  -0.5f}, .Normal{ 0.0f, -1.0f,  0.0f}, .TexCoord{ 0.0f,  1.0f}},

         {.Pos{ -0.5f,   0.5f,  -0.5f}, .Normal{ 0.0f,  1.0f,  0.0f}, .TexCoord{ 0.0f,  1.0f}},
         {.Pos{  0.5f,   0.5f,   0.5f}, .Normal{ 0.0f,  1.0f,  0.0f}, .TexCoord{ 1.0f,  0.0f}},
         {.Pos{  0.5f,   0.5f,  -0.5f}, .Normal{ 0.0f,  1.0f,  0.0f}, .TexCoord{ 1.0f,  1.0f}},
         {.Pos{  0.5f,   0.5f,   0.5f}, .Normal{ 0.0f,  1.0f,  0.0f}, .TexCoord{ 1.0f,  0.0f}},
         {.Pos{ -0.5f,   0.5f,  -0.5f}, .Normal{ 0.0f,  1.0f,  0.0f}, .TexCoord{ 0.0f,  1.0f}},
         {.Pos{ -0.5f,   0.5f,   0.5f}, .Normal{ 0.0f,  1.0f,  0.0f}, .TexCoord{ 0.0f,  0.0f}},

         // Plane
         {.Pos{ 10.0f,   0.0f,  10.0f}, .Normal{ 0.0f,  1.0f,  0.0f}, .TexCoord{10.0f,  0.0f}},
         {.Pos{-10.0f,   0.0f, -10.0f}, .Normal{ 0.0f,  1.0f,  0.0f}, .TexCoord{ 0.0f, 10.0f}},
         {.Pos{-10.0f,   0.0f,  10.0f}, .Normal{ 0.0f,  1.0f,  0.0f}, .TexCoord{ 0.0f,  0.0f}},

         {.Pos{ 10.0f,   0.0f,  10.0f}, .Normal{ 0.0f,  1.0f,  0.0f}, .TexCoord{10.0f,  0.0f}},
         {.Pos{ 10.0f,   0.0f, -10.0f}, .Normal{ 0.0f,  1.0f,  0.0f}, .TexCoord{10.0f, 10.0f}},
         {.Pos{-10.0f,   0.0f, -10.0f}, .Normal{ 0.0f,  1.0f,  0.0f}, .TexCoord{ 0.0f, 10.0f}},

         // Fullscreen  quad
         {.Pos{ -1.0f,   1.0f,   0.0f}, .Normal{ 0.0f,  1.0f,  0.0f}, .TexCoord{ 0.0f,  1.0f}},
         {.Pos{ -1.0f,  -1.0f,   0.0f}, .Normal{ 0.0f,  1.0f,  0.0f}, .TexCoord{ 0.0f,  0.0f}},
         {.Pos{  1.0f,  -1.0f,   0.0f}, .Normal{ 0.0f,  1.0f,  0.0f}, .TexCoord{ 1.0f,  0.0f}},

         {.Pos{ -1.0f,   1.0f,   0.0f}, .Normal{ 0.0f,  1.0f,  0.0f}, .TexCoord{ 0.0f,  1.0f}},
         {.Pos{  1.0f,  -1.0f,   0.0f}, .Normal{ 0.0f,  1.0f,  0.0f}, .TexCoord{ 1.0f,  0.0f}},
         {.Pos{  1.0f,   1.0f,   0.0f}, .Normal{ 0.0f,  1.0f,  0.0f}, .TexCoord{ 1.0f,  1.0f}},
      };

      Pikzel::BufferLayout layout = {
         {"inPos",       Pikzel::DataType::Vec3},
         {"inNormal",    Pikzel::DataType::Vec3},
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

      glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 15.0f, -5.0f);  // TODO: how does one determine the parameters here?
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
      m_TextureFloor = Pikzel::RenderCore::CreateTexture({.path = "Assets/" APP_NAME "/Textures/Floor.png"});
      m_TextureFloorSpecular = Pikzel::RenderCore::CreateTexture({.path = "Assets/" APP_NAME "/Textures/FloorSpecular.png", .format = Pikzel::TextureFormat::RGBA8});
   }


   void CreateFramebuffers() {
      uint32_t width = 4096;
      uint32_t height = 4096;
      m_FramebufferScene = Pikzel::RenderCore::CreateFramebuffer({.width = GetWindow().GetWidth(), .height = GetWindow().GetHeight(), .msaaNumSamples = 4, .clearColorValue = GetWindow().GetClearColor()});
      m_FramebufferDirShadow = Pikzel::RenderCore::CreateFramebuffer({.width = width, .height = height, .attachments = {{Pikzel::AttachmentType::Depth, Pikzel::TextureFormat::D32F}}});
      m_FramebufferPtShadow = Pikzel::RenderCore::CreateFramebuffer({
         .width = width,
         .height = height,
         .layers = static_cast<uint32_t>(m_PointLights.size()),
         .attachments = {{Pikzel::AttachmentType::Depth, Pikzel::TextureFormat::D32F, Pikzel::TextureType::TextureCubeArray}}
      });
   }


   void CreatePipelines() {
      m_PipelineDirShadow = m_FramebufferDirShadow->GetGraphicsContext().CreatePipeline({
         .shaders = {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/Depth.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/Depth.frag.spv" }
         },
         .bufferLayout = m_VertexBuffer->GetLayout()
      });
      m_PipelinePtShadow = m_FramebufferPtShadow->GetGraphicsContext().CreatePipeline({
         .shaders = {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/DepthCube.vert.spv" },
            { Pikzel::ShaderType::Geometry, "Assets/" APP_NAME "/Shaders/DepthCube.geom.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/DepthCube.frag.spv" }
         },
         .bufferLayout = m_VertexBuffer->GetLayout(),
      });
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
         .ambient = Pikzel::sRGB{0.1f, 0.1f, 0.1f},
         .size = 0.02
      }
   };

   // note: shader expects 1 to 32
   std::vector<Pikzel::PointLight> m_PointLights = {
      {
         .position = {-2.8f, 2.8f, -1.7f},
         .color = Pikzel::sRGB{1.0f, 1.0f, 1.0f},
         .size = 0.02,
         .power = 20.0f
      }
      ,{
         .position = {2.3f, 3.3f, -4.0f},
         .color = Pikzel::sRGB{0.0f, 1.0f, 0.0f},
         .size = 0.02,
         .power = 20.0f
      }
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
   std::unique_ptr<Pikzel::Texture> m_TextureFloor;
   std::unique_ptr<Pikzel::Texture> m_TextureFloorSpecular;
   std::unique_ptr<Pikzel::Framebuffer> m_FramebufferDirShadow;
   std::unique_ptr<Pikzel::Framebuffer> m_FramebufferPtShadow;
   std::unique_ptr<Pikzel::Framebuffer> m_FramebufferScene;
   std::unique_ptr<Pikzel::Pipeline> m_PipelineDirShadow;
   std::unique_ptr<Pikzel::Pipeline> m_PipelinePtShadow;
   std::unique_ptr<Pikzel::Pipeline> m_PipelineColoredModel;
   std::unique_ptr<Pikzel::Pipeline> m_PipelineLitModel;
   std::unique_ptr<Pikzel::Pipeline> m_PipelineFullScreenQuad;

   Pikzel::DeltaTime m_DeltaTime = {};
   bool m_ShowDirectionalLight = true;
   bool m_ShowPointLights = true;
   bool m_UsePCSS = true;
};


std::unique_ptr<Pikzel::Application> CreateApplication(int argc, const char* argv[]) {
   return std::make_unique<Shadows>();
}
