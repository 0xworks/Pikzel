#include "Pikzel/Pikzel.h"
#include "Pikzel/Core/EntryPoint.h"

// Deferred rendering demonstration
// The scene is first rendered to an off-screen "G-buffer" consisting of:
// - position information
// - normals
// - diffuse color (rgb) and specular amount (a) (packed into one vec4)
//
// Then later (i.e deferred), a full screen quad is rendered to the screen and this one does the (expensive) lighting calculations
// using the information from G-buffer.
//
// I have not put too much effort into this demo as ultimately I want to go with a clustered forward renderer rather than deferred.
// This demo is just a quick one to see how deferred works.
//
// Points of interest in the code are marked with "POI" in the code comments.

// note: Pikzel uses reverse-Z so near and far planes are swapped
constexpr float nearPlane = 30.0f;
constexpr float farPlane = 0.1f;

class DeferredRendering final : public Pikzel::Application {
public:
   DeferredRendering()
   : Pikzel::Application {{.title = APP_DESCRIPTION, .clearColor = Pikzel::sRGB{0.01f, 0.01f, 0.01f}, .isVSync = true}}
   , m_Input {GetWindow()}
   {
      CreateVertexBuffers();
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

      // POI: Geometry pass, render to G-buffer
      {
         Pikzel::GraphicsContext& gc = m_GBuffer->GetGraphicsContext();
         gc.BeginFrame();
         gc.Bind(*m_PipelineGeometry);


         gc.Bind("UBOMatrices"_hs, *m_BufferMatrices);

         // floor
         glm::mat4 model = glm::identity<glm::mat4>();
         gc.PushConstant("constants.model"_hs, model);
         gc.Bind("diffuseMap"_hs, *m_TextureFloor);
         gc.Bind("specularMap"_hs, *m_TextureFloorSpecular);
         gc.DrawTriangles(*m_VertexBuffer, 6, 36);

         // cubes
         gc.Bind("diffuseMap"_hs, *m_TextureContainer);
         gc.Bind("specularMap"_hs, *m_TextureContainerSpecular);
         for (int i = 0; i < m_CubePositions.size(); ++i) {
            glm::mat4 model = glm::rotate(glm::translate(glm::identity<glm::mat4>(), m_CubePositions[i]), glm::radians(20.0f * i), glm::vec3 {1.0f, 0.3f, 0.5f});
            gc.PushConstant("constants.model"_hs, model);
            gc.DrawTriangles(*m_VertexBuffer, 36);
         }

         gc.EndFrame();
         gc.SwapBuffers();
      }

      // POI: Lighting pass
      GetWindow().BeginFrame();
      Pikzel::GraphicsContext& gc = GetWindow().GetGraphicsContext();
      gc.Bind(*m_PipelineLighting);
      gc.PushConstant("constants.numPointLights"_hs, static_cast<uint32_t>(m_PointLights.size()));
      gc.PushConstant("constants.showDirectionalLight"_hs, m_ShowDirectionalLight ? 1u : 0u);
      gc.PushConstant("constants.showPointLights"_hs, m_ShowPointLights ? 1u : 0u);
      gc.Bind("UBOMatrices"_hs, *m_BufferMatrices);
      gc.Bind("UBODirectionalLight"_hs, *m_BufferDirectionalLight);
      gc.Bind("UBOPointLights"_hs, *m_BufferPointLights);
      gc.Bind("uPosition"_hs, m_GBuffer->GetColorTexture(0));
      gc.Bind("uNormal"_hs, m_GBuffer->GetColorTexture(1));
      gc.Bind("uDiffuseSpecular"_hs, m_GBuffer->GetColorTexture(2));
      gc.DrawTriangles(*m_QuadVertexBuffer, 6);

      GetWindow().BeginImGuiFrame();
      {
         ImGui::Begin("Lighting");
         ImGui::Checkbox("Directional Light", &m_ShowDirectionalLight);
         ImGui::Checkbox("Point Lights", &m_ShowPointLights);
         for (size_t i = 0; i < m_PointLights.size(); ++i) {
            ImGuiDrawPointLight(fmt::format("light {0}", i).c_str(), m_PointLights[i]);
         }
         ImGui::Text("Frame time: %.3fms (%.0f FPS)", m_DeltaTime.count() * 1000.0f, 1.0f / m_DeltaTime.count());

         // POI: We can also visualize what's in the G-buffer by drawing the attached textures in ImGui image.
         //      Note that some of the images won't look quite right (as we have numbers outside of the 0-1 range in the G-buffer,
         //      and in the case of the DiffuseSpecular texture, we're using the alpha channel for specular amount, not transparency!)
         //      but this visualization gives you the general idea of what's going on.
         ImGui::Text("G-Buffer:");
         ImVec2 size = ImGui::GetContentRegionAvail();
         size.x /= 2.0f;
         size.y /= 2.0f;
         ImGui::Image(m_GBuffer->GetImGuiColorTextureId(0), size, ImVec2 {0, 1}, ImVec2 {1, 0});
         ImGui::SameLine();
         ImGui::Image(m_GBuffer->GetImGuiColorTextureId(1), size, ImVec2 {0, 1}, ImVec2 {1, 0});
         ImGui::Image(m_GBuffer->GetImGuiColorTextureId(2), size, ImVec2 {0, 1}, ImVec2 {1, 0});
         ImGui::SameLine();
         ImGui::Image(m_GBuffer->GetImGuiDepthTextureId(), size, ImVec2 {0, 1}, ImVec2 {1, 0});

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

      Pikzel::BufferLayout layout = {
         {"inPos",       Pikzel::DataType::Vec3},
         {"inNormal",    Pikzel::DataType::Vec3},
         {"inTangent",   Pikzel::DataType::Vec3},
         {"inTexCoords", Pikzel::DataType::Vec2},
      };
      m_VertexBuffer = Pikzel::RenderCore::CreateVertexBuffer(layout, sizeof(vertices), vertices);

      layout = {
         {"inPos",       Pikzel::DataType::Vec3},
         {"inTexCoords", Pikzel::DataType::Vec2},
      };
      m_QuadVertexBuffer = Pikzel::RenderCore::CreateVertexBuffer(layout, sizeof(quadVertices), quadVertices);
   }


   struct Matrices {
      glm::mat4 viewProjection;
      glm::mat4 lightSpace;
      glm::vec3 eyePosition;
   };

   void CreateUniformBuffers() {
      m_BufferMatrices = Pikzel::RenderCore::CreateUniformBuffer(sizeof(Matrices));
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
      m_GBuffer = Pikzel::RenderCore::CreateFramebuffer({
         .width = GetWindow().GetWidth(),
         .height = GetWindow().GetHeight(),
         .msaaNumSamples = 1,                                                              // POI: no MSAA with deferred rendering
         .clearColorValue = GetWindow().GetClearColor(),
         .attachments = {
            {Pikzel::AttachmentType::Color, Pikzel::TextureFormat::RGBA16F},               // POI: position color buffer. 16 bit floating point
            {Pikzel::AttachmentType::Color, Pikzel::TextureFormat::RGBA16F},               // POI: normals color buffer. 16 bit floating point
            {Pikzel::AttachmentType::Color, Pikzel::TextureFormat::RGBA8},                 // POI: diffuse color + specular.  This can go in an RGBA 8-bit buffer
            {Pikzel::AttachmentType::Depth, Pikzel::TextureFormat::D32F}
         }
      });


   }


   void CreatePipelines() {
      // POI: We have a pipeline for the "geometry pass".  This renders the vertices into the G-buffer
      m_PipelineGeometry = m_GBuffer->GetGraphicsContext().CreatePipeline({
         .enableBlend = false,  // POI: blending must be disabled for the G-Buffer to work properly
         .shaders = {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/GeometryPass.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/GeometryPass.frag.spv" }
         },
         .bufferLayout = m_VertexBuffer->GetLayout()
      });

      // POI: and another pipeline for the "lighting pass". This pipeline is used to render a full screen quad,
      // and the fragment shader does the (expensive) lighting calculations for each screen pixel using
      // information taken from the G-buffer
      m_PipelineLighting = GetWindow().GetGraphicsContext().CreatePipeline({
         .shaders = {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/LightingPass.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/LightingPass.frag.spv" }
         },
         .bufferLayout = m_QuadVertexBuffer->GetLayout(),
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
         .color = Pikzel::sRGB{1.0f, 1.0f, 1.0f},
         .ambient = Pikzel::sRGB{0.1f, 0.1f, 0.1f},
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
   std::unique_ptr<Pikzel::VertexBuffer> m_QuadVertexBuffer;
   std::unique_ptr<Pikzel::UniformBuffer> m_BufferMatrices;
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
   std::unique_ptr<Pikzel::Framebuffer> m_GBuffer;
   std::unique_ptr<Pikzel::Pipeline> m_PipelineGeometry;
   std::unique_ptr<Pikzel::Pipeline> m_PipelineLighting;

   Pikzel::DeltaTime m_DeltaTime = {};
   bool m_ShowDirectionalLight = true;
   bool m_ShowPointLights = true;
};


std::unique_ptr<Pikzel::Application> CreateApplication(int argc, const char* argv[]) {
   return std::make_unique<DeferredRendering>();
}
