#include "ModelSerializer.h"

#include "Pikzel/Pikzel.h"
#include "Pikzel/Core/EntryPoint.h"

// note: Pikzel uses reverse-Z so near and far planes are swapped
constexpr float nearPlane = 10000.0f;
constexpr float farPlane = 1.f;

class SponzaPBRApp final : public Pikzel::Application {
public:
   SponzaPBRApp()
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


   ~SponzaPBRApp() {
      SponzaPBR::ModelSerializer::ClearTextureCache();
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
      static float lightRadius = 1000.0f; // TODO: set light radius appropriately
      static glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), 1.0f, lightRadius, 1.0f);  // note: Pikzel uses reverse-Z so near and far planes are swapped
      static int skyboxLod = 1;

      PKZL_PROFILE_FUNCTION();

      // update buffers
      glm::mat4 view = glm::lookAt(m_Camera.Position, m_Camera.Position + m_Camera.Direction, m_Camera.UpVector);

      Matrices matrices;
      matrices.viewProjection = m_Camera.Projection * view;
      matrices.lightSpace = m_LightSpace;
      matrices.eyePosition = m_Camera.Position;
      m_BufferMatrices->CopyFromHost(0, sizeof(Matrices), &matrices);
      m_BufferDirectionalLight->CopyFromHost(0, sizeof(Pikzel::DirectionalLight) * m_DirectionalLights.size(), m_DirectionalLights.data());
      m_BufferPointLights->CopyFromHost(0, sizeof(Pikzel::PointLight) * m_PointLights.size(), m_PointLights.data());

      // render to directional light shadow map
      {
         Pikzel::GraphicsContext& gc = m_FramebufferDirShadow->GetGraphicsContext();
         gc.BeginFrame();
         gc.Bind(*m_PipelineDirShadow);

         // Model
         glm::mat4 transform = glm::identity<glm::mat4>();
         gc.PushConstant("constants.mvp"_hs, m_LightSpace * transform);
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
               lightProjection * glm::lookAt(light.Position, light.Position + glm::vec3 { 1.0f,  0.0f,  0.0f}, glm::vec3 {0.0f, -1.0f,  0.0f}),
               lightProjection * glm::lookAt(light.Position, light.Position + glm::vec3 {-1.0f,  0.0f,  0.0f}, glm::vec3 {0.0f, -1.0f,  0.0f}),
               lightProjection * glm::lookAt(light.Position, light.Position + glm::vec3 { 0.0f,  1.0f,  0.0f}, glm::vec3 {0.0f,  0.0f,  1.0f}),
               lightProjection * glm::lookAt(light.Position, light.Position + glm::vec3 { 0.0f, -1.0f,  0.0f}, glm::vec3 {0.0f,  0.0f, -1.0f}),
               lightProjection * glm::lookAt(light.Position, light.Position + glm::vec3 { 0.0f,  0.0f,  1.0f}, glm::vec3 {0.0f, -1.0f,  0.0f}),
               lightProjection * glm::lookAt(light.Position, light.Position + glm::vec3 { 0.0f,  0.0f, -1.0f}, glm::vec3 {0.0f, -1.0f,  0.0f}),
            };
            m_BufferLightViews->CopyFromHost(0, sizeof(glm::mat4) * lightViews.size(), lightViews.data());

            gcPtShadows.BeginFrame(i == 0 ? Pikzel::BeginFrameOp::ClearAll : Pikzel::BeginFrameOp::ClearNone);
            gcPtShadows.Bind(*m_PipelinePtShadow);
            gcPtShadows.PushConstant("constants.lightIndex"_hs, i);
            gcPtShadows.PushConstant("constants.lightRadius"_hs, lightRadius);
            gcPtShadows.Bind("UBOLightViews"_hs, *m_BufferLightViews);
            gcPtShadows.Bind("UBOPointLights"_hs, *m_BufferPointLights);

            glm::mat4 transform = glm::identity<glm::mat4>();
            gcPtShadows.PushConstant("constants.model"_hs, transform);
            for (const auto& mesh : m_Model->Meshes) {
               gcPtShadows.DrawIndexed(*mesh.VertexBuffer, *mesh.IndexBuffer);
            }

            gcPtShadows.EndFrame();
            gcPtShadows.SwapBuffers();
         }
      }

      // render scene
      {

         Pikzel::GraphicsContext& gc = m_FramebufferScene->GetGraphicsContext();
         gc.BeginFrame();

         gc.Bind(*m_PipelinePBR);
         gc.PushConstant("constants.textureRepeat"_hs, glm::vec2{ 1.0, 1.0 });
         gc.PushConstant("constants.heightScale"_hs, 0.05f);
         gc.PushConstant("constants.lightRadius"_hs, lightRadius);
         gc.PushConstant("constants.numPointLights"_hs, static_cast<uint32_t>(m_PointLights.size()));
         gc.Bind("UBOMatrices"_hs, *m_BufferMatrices);
         gc.Bind("UBODirectionalLight"_hs, *m_BufferDirectionalLight);
         gc.Bind("UBOPointLights"_hs, *m_BufferPointLights);
         gc.Bind("uDirShadowMap"_hs, m_FramebufferDirShadow->GetDepthTexture());
         gc.Bind("uDirShadowMap"_hs, m_FramebufferDirShadow->GetDepthTexture());
         gc.Bind("uPtShadowMap"_hs, m_FramebufferPtShadow->GetDepthTexture());
         gc.Bind("uIrradiance"_hs, *m_Irradiance);
         gc.Bind("uSpecularIrradiance"_hs, *m_SpecularIrradiance);
         gc.Bind("uSpecularBRDF_LUT"_hs, *m_SpecularBRDF_LUT);

         glm::mat4 transform = glm::identity<glm::mat4>();
         gc.PushConstant("constants.model"_hs, transform);
         for (const auto& mesh : m_Model->Meshes) {
            gc.Bind("uAlbedo"_hs, *mesh.AlbedoTexture);
            gc.Bind("uMetallicRoughness"_hs, *mesh.MetallicRoughnessTexture);
            gc.Bind("uNormals"_hs, *mesh.NormalTexture);
            gc.Bind("uAmbientOcclusion"_hs, *mesh.AmbientOcclusionTexture);
            gc.Bind("uHeightMap"_hs, *mesh.HeightTexture);
            gc.DrawIndexed(*mesh.VertexBuffer, *mesh.IndexBuffer);
         }

         // render point lights as little cubes
         gc.Bind(*m_PipelineLight);
         for (const auto& pointLight : m_PointLights) {
            glm::mat4 model = glm::translate(glm::identity<glm::mat4>(), pointLight.Position);
            gc.PushConstant("constants.mvp"_hs, matrices.viewProjection * model);
            gc.PushConstant("constants.lightColor"_hs, pointLight.Color);
            gc.DrawTriangles(*m_VertexBufferCube, 36);
         }

         // Skybox
         view = glm::mat3(view);
         gc.Bind(*m_PipelineSkybox);
         gc.Bind("uSkybox"_hs, *m_Skybox);
         gc.PushConstant("constants.vp"_hs, m_Camera.Projection * view);
         gc.PushConstant("constants.lod"_hs, skyboxLod);
         gc.DrawTriangles(*m_VertexBuffer, 36, 6);

         gc.EndFrame();
         gc.SwapBuffers();
      }

      // post processing
      GetWindow().BeginFrame();
      Pikzel::GraphicsContext& gc = GetWindow().GetGraphicsContext();
      gc.Bind(*m_PipelinePostProcess);
      gc.PushConstant("constants.tonemap"_hs, m_ToneMap);
      gc.PushConstant("constants.exposure"_hs, m_Exposure);
      gc.Bind("uTexture"_hs, m_FramebufferScene->GetColorTexture(0));
      gc.DrawTriangles(*m_VertexBuffer, 6);

      GetWindow().BeginImGuiFrame();
      {
         ImGui::Begin("Lighting");
         for (size_t i = 0; i < m_PointLights.size(); ++i) {
            ImGuiDrawPointLight(fmt::format("light {0}", i).c_str(), m_PointLights[i]);
         }
         ImGui::Text("Frame time: %.3fms (%.0f FPS)", m_DeltaTime.count() * 1000.0f, 1.0f / m_DeltaTime.count());
         ImGui::Text("Tone mapping:");
         ImGui::RadioButton("None", &m_ToneMap, 0);
         ImGui::RadioButton("Reinhard", &m_ToneMap, 1);
         ImGui::RadioButton("Exposure", &m_ToneMap, 2);
         ImGui::SameLine();
         ImGui::DragFloat("", &m_Exposure, 0.1f, 0, 10);
         ImGui::InputInt("Lod", &skyboxLod, 1.0f);

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
      m_Camera.Projection = glm::perspective(m_Camera.FoVRadians, static_cast<float>(GetWindow().GetWidth()) / static_cast<float>(GetWindow().GetHeight()), nearPlane, farPlane);

      // recreate framebuffer with new size
      CreateFramebuffers();
   }


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

   struct Vertex {
      glm::vec3 Pos;
      glm::vec2 TexCoords;
   };

   void CreateVertexBuffers() {
      Vertex vertices[] = {
         // Fullscreen  quad
         {.Pos{-1.0f,  1.0f,  0.0f}, .TexCoords{0.0f, 1.0f}},
         {.Pos{-1.0f, -1.0f,  0.0f}, .TexCoords{0.0f, 0.0f}},
         {.Pos{ 1.0f, -1.0f,  0.0f}, .TexCoords{1.0f, 0.0f}},
         {.Pos{-1.0f,  1.0f,  0.0f}, .TexCoords{0.0f, 1.0f}},
         {.Pos{ 1.0f, -1.0f,  0.0f}, .TexCoords{1.0f, 0.0f}},
         {.Pos{ 1.0f,  1.0f,  0.0f}, .TexCoords{1.0f, 1.0f}},

         // Skybox
         {.Pos{-1.0f,  1.0f, -1.0f}, .TexCoords{0.0f, 0.0f}},
         {.Pos{-1.0f, -1.0f, -1.0f}, .TexCoords{1.0f, 1.0f}},
         {.Pos{ 1.0f, -1.0f, -1.0f}, .TexCoords{1.0f, 0.0f}},
         {.Pos{ 1.0f, -1.0f, -1.0f}, .TexCoords{1.0f, 1.0f}},
         {.Pos{ 1.0f,  1.0f, -1.0f}, .TexCoords{0.0f, 0.0f}},
         {.Pos{-1.0f,  1.0f, -1.0f}, .TexCoords{0.0f, 1.0f}},

         {.Pos{-1.0f, -1.0f,  1.0f}, .TexCoords{0.0f, 0.0f}},
         {.Pos{-1.0f, -1.0f, -1.0f}, .TexCoords{1.0f, 0.0f}},
         {.Pos{-1.0f,  1.0f, -1.0f}, .TexCoords{1.0f, 1.0f}},
         {.Pos{-1.0f,  1.0f, -1.0f}, .TexCoords{1.0f, 1.0f}},
         {.Pos{-1.0f,  1.0f,  1.0f}, .TexCoords{0.0f, 1.0f}},
         {.Pos{-1.0f, -1.0f,  1.0f}, .TexCoords{0.0f, 0.0f}},

         {.Pos{ 1.0f, -1.0f, -1.0f}, .TexCoords{1.0f, 1.0f}},
         {.Pos{ 1.0f, -1.0f,  1.0f}, .TexCoords{0.0f, 1.0f}},
         {.Pos{ 1.0f,  1.0f,  1.0f}, .TexCoords{0.0f, 0.0f}},
         {.Pos{ 1.0f,  1.0f,  1.0f}, .TexCoords{0.0f, 0.0f}},
         {.Pos{ 1.0f,  1.0f, -1.0f}, .TexCoords{1.0f, 0.0f}},
         {.Pos{ 1.0f, -1.0f, -1.0f}, .TexCoords{1.0f, 1.0f}},

         {.Pos{-1.0f, -1.0f,  1.0f}, .TexCoords{1.0f, 1.0f}},
         {.Pos{-1.0f,  1.0f,  1.0f}, .TexCoords{0.0f, 0.0f}},
         {.Pos{ 1.0f,  1.0f,  1.0f}, .TexCoords{0.0f, 1.0f}},
         {.Pos{ 1.0f,  1.0f,  1.0f}, .TexCoords{0.0f, 0.0f}},
         {.Pos{ 1.0f, -1.0f,  1.0f}, .TexCoords{1.0f, 1.0f}},
         {.Pos{-1.0f, -1.0f,  1.0f}, .TexCoords{1.0f, 0.0f}},

         {.Pos{-1.0f,  1.0f, -1.0f}, .TexCoords{0.0f, 0.0f}},
         {.Pos{ 1.0f,  1.0f, -1.0f}, .TexCoords{1.0f, 0.0f}},
         {.Pos{ 1.0f,  1.0f,  1.0f}, .TexCoords{1.0f, 1.0f}},
         {.Pos{ 1.0f,  1.0f,  1.0f}, .TexCoords{1.0f, 1.0f}},
         {.Pos{-1.0f,  1.0f,  1.0f}, .TexCoords{0.0f, 1.0f}},
         {.Pos{-1.0f,  1.0f, -1.0f}, .TexCoords{0.0f, 0.0f}},

         {.Pos{-1.0f, -1.0f, -1.0f}, .TexCoords{0.0f, 0.0f}},
         {.Pos{-1.0f, -1.0f,  1.0f}, .TexCoords{1.0f, 1.0f}},
         {.Pos{ 1.0f, -1.0f, -1.0f}, .TexCoords{1.0f, 0.0f}},
         {.Pos{ 1.0f, -1.0f, -1.0f}, .TexCoords{1.0f, 1.0f}},
         {.Pos{-1.0f, -1.0f,  1.0f}, .TexCoords{0.0f, 0.0f}},
         {.Pos{ 1.0f, -1.0f,  1.0f}, .TexCoords{0.0f, 1.0f}},
      };

      m_VertexBuffer = Pikzel::RenderCore::CreateVertexBuffer(
         {
            {"inPos",       Pikzel::DataType::Vec3},
            {"inTexCoords", Pikzel::DataType::Vec2},
         },
         sizeof(vertices),
         vertices
      );

      glm::vec3 cubeVertices[] = {
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
      m_VertexBufferCube = Pikzel::RenderCore::CreateVertexBuffer(
         {
            {"inPos", Pikzel::DataType::Vec3},
         },
         sizeof(cubeVertices),
         cubeVertices
      );


      // POI: load model
      m_Model = SponzaPBR::ModelSerializer::Import("Assets/Models/Sponza/sponza.gltf");
   }


   struct Matrices {
      glm::mat4 viewProjection;
      glm::mat4 lightSpace;
      glm::vec3 eyePosition;
   };

   void CreateUniformBuffers() {
      glm::mat4 lightProjection = glm::ortho(-2100.0f, 2100.0f, -2000.0f, 2000.0f, 2000.0f, 50.0f);  // TODO: need to automatically determine correct parameters here (+ cascades...)
      glm::mat4 lightView = glm::lookAt(-m_DirectionalLights[0].Direction, glm::vec3 {0.0f, 0.0f, 0.0f}, glm::vec3 {0.0f, 1.0f, 0.0f});
      m_LightSpace = lightProjection * lightView;

      m_BufferMatrices = Pikzel::RenderCore::CreateUniformBuffer(sizeof(Matrices));
      m_BufferDirectionalLight = Pikzel::RenderCore::CreateUniformBuffer(sizeof(Pikzel::DirectionalLight) * m_DirectionalLights.size());
      m_BufferLightViews = Pikzel::RenderCore::CreateUniformBuffer(sizeof(glm::mat4) * m_PointLights.size() * 6);
      m_BufferPointLights = Pikzel::RenderCore::CreateUniformBuffer(sizeof(Pikzel::PointLight) * m_PointLights.size());
   }


   void CreateTextures() {
      m_Skybox = Pikzel::RenderCore::CreateTexture({.Type = Pikzel::TextureType::TextureCube, .Path = "Assets/Skyboxes/dikhololo_night_4k.hdr"});

      // POI: We use compute shaders to bake lighting from the skybox image into diffuse and specular "Irradiance maps"

      // diffuse irradiance
      m_Irradiance = Pikzel::RenderCore::CreateTexture({
         .Type = Pikzel::TextureType::TextureCube,
         .Width = 32,                                // POI: The diffuse irradiance map does not need to be very big
         .Height = 32,
         .Format = Pikzel::TextureFormat::RGBA16F,
         .ImageStorage = true                        // POI: Textures that will be written to in a shader need this flag set (because Vulkan), and also need to be Commit()'d before you use them in shader
      });
      std::unique_ptr<Pikzel::ComputeContext> compute = Pikzel::RenderCore::CreateComputeContext();
      std::unique_ptr<Pikzel::Pipeline> pipelineIrradiance = compute->CreatePipeline({
         .Shaders = {
            { Pikzel::ShaderType::Compute, "Renderer/EnvironmentIrradiance.comp.spv"}
         }
      });
 
      compute->Begin();
      compute->Bind(*pipelineIrradiance);
      compute->Bind("inputTexture"_hs, *m_Skybox);
      compute->Bind("outputTexture"_hs, *m_Irradiance);
      compute->Dispatch(m_Irradiance->GetWidth() / 32, m_Irradiance->GetHeight() / 32, 6);  // POI: width and height divided by 32 because the shader works in 32x32 blocks.  z is 6 for the six faces of the cube
      compute->End();
      m_Irradiance->Commit();

      // specular irradiance
      m_SpecularIrradiance = Pikzel::RenderCore::CreateTexture({
         .Type = Pikzel::TextureType::TextureCube,
         .Width = m_Skybox->GetWidth(),                              // POI: The specular irradiance map cannot be as small as the diffuse one, otherwise details will be lost in reflections off highly reflective materials.
         .Height = m_Skybox->GetHeight(),
         .Format = Pikzel::TextureFormat::RGBA16F,
         .ImageStorage = true
      });

      // POI: specular irradiance mip level 0 is a straight copy of the skybox.
      //      this level is used for perfectly reflective materials
      m_SpecularIrradiance->CopyFrom(*m_Skybox);

      // POI: specular irradiance mip levels 1..N are computed by pre-filtering the skybox
      //      per Epic Games split sum approximation.
      std::unique_ptr<Pikzel::Pipeline> pipelinePrefilter = compute->CreatePipeline({
         .Shaders = {
            { Pikzel::ShaderType::Compute, "Renderer/EnvironmentPrefilter.comp.spv"}
         }
      });
      const float deltaRoughness = 0.5f / static_cast<float>(m_SpecularIrradiance->GetMIPLevels() - 1);
      for (uint32_t level = 1; level < m_SpecularIrradiance->GetMIPLevels(); ++level) {
         compute->Begin();
         compute->Bind(*pipelinePrefilter);
         compute->PushConstant("constants.roughness"_hs, level * deltaRoughness);
         compute->Bind("inputTexture"_hs, *m_Skybox);
         compute->Bind("outputTexture"_hs, *m_SpecularIrradiance, level);
         compute->Dispatch(
            std::max(1u, m_SpecularIrradiance->GetWidth() / (1 << level) / 32),
            std::max(1u, m_SpecularIrradiance->GetHeight() / (1 << level) / 32),
            6
         );
         compute->End();
      }
      m_SpecularIrradiance->Commit(/*generateMipmap = */false);

      // POI: We also use a compute shader to pre-calculate the specular BRDF values and store them in a lookup table (packed into a texture)
      //      These values do not depend on the skybox and could just be loaded from a texture stored on disk.
      //      However, its just as fast to compute them as it is to load a texture from disk, and precomputing has a couple of advantages:
      //      * don't need to distribute yet another texture with the compiled binaries
      //      * can easily vary the parameters (in particular the size of the texture) here
      m_SpecularBRDF_LUT = Pikzel::RenderCore::CreateTexture({
         .Width = 512,
         .Height = 512,
         .Format = Pikzel::TextureFormat::RG16F,      // POI: The lookup table only needs two dimensions. Which we store as 16-bit floats in the Red and Green channels.
         .WrapU = Pikzel::TextureWrap::ClampToEdge,   // POI: The lookup table must clamp to edge, otherwise there will be artefacts when looking up the table for values near the edges
         .WrapV = Pikzel::TextureWrap::ClampToEdge,
         .MIPLevels = 1,
         .ImageStorage = true
      });
      std::unique_ptr<Pikzel::Pipeline> pipelineSpecularBRDF = compute->CreatePipeline({
         .Shaders = {
            { Pikzel::ShaderType::Compute, "Renderer/EnvironmentSpecularBRDF.comp.spv"}
         }
      });
      compute->Begin();
      compute->Bind(*pipelineSpecularBRDF);
      compute->Bind("LUT"_hs, *m_SpecularBRDF_LUT);
      compute->Dispatch(m_SpecularBRDF_LUT->GetWidth() / 32, m_SpecularBRDF_LUT->GetHeight() / 32, 1);
      compute->End();
      m_SpecularBRDF_LUT->Commit();
   }


   void CreateFramebuffers() {
      const uint32_t shadowMapWidth = 4096;
      const uint32_t shadowMapHeight = 4096;

      m_FramebufferScene = Pikzel::RenderCore::CreateFramebuffer({
         .Width = GetWindow().GetWidth(),
         .Height = GetWindow().GetHeight(),
         .MSAANumSamples = 4,
         .ClearColorValue = GetWindow().GetClearColor(),
         .Attachments = {
            {Pikzel::AttachmentType::Color, Pikzel::TextureFormat::RGBA16F},
            {Pikzel::AttachmentType::Depth, Pikzel::TextureFormat::D32F}
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
            .Width = shadowMapWidth / 2,
            .Height = shadowMapHeight / 2,
            .Layers = 4,
            .Attachments = {{Pikzel::AttachmentType::Depth, Pikzel::TextureFormat::D32F, Pikzel::TextureType::TextureCubeArray}}
         });
      }
   }


   void CreatePipelines() {
      m_PipelineSkybox = m_FramebufferScene->GetGraphicsContext().CreatePipeline({
         .Shaders = {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/Skybox.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/Skybox.frag.spv" }
         },
         .BufferLayout = m_VertexBuffer->GetLayout()
      });

      m_PipelineLight = m_FramebufferScene->GetGraphicsContext().CreatePipeline({
         .Shaders = {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/Light.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/Light.frag.spv" }
         },
         .BufferLayout = m_VertexBufferCube->GetLayout(),
      });

      Pikzel::BufferLayout layout = {
         { "inPos",     Pikzel::DataType::Vec3 },
         { "inNormal",  Pikzel::DataType::Vec3 },
         { "inTangent", Pikzel::DataType::Vec3 },
         { "inUV",      Pikzel::DataType::Vec2 },
      };

      m_PipelineDirShadow = m_FramebufferDirShadow->GetGraphicsContext().CreatePipeline({
         .Shaders = {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/Depth.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/Depth.frag.spv" }
         },
         .BufferLayout = layout
      });

      m_PipelinePtShadow = m_FramebufferPtShadow->GetGraphicsContext().CreatePipeline({
         .Shaders = {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/DepthCube.vert.spv" },
            { Pikzel::ShaderType::Geometry, "Assets/" APP_NAME "/Shaders/DepthCube.geom.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/DepthCube.frag.spv" }
         },
         .BufferLayout = layout,
      });

      // POI: here is the PBR pipeline
      m_PipelinePBR = m_FramebufferScene->GetGraphicsContext().CreatePipeline({
         .Shaders = {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/PBR.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/PBR.frag.spv" }
         },
         .BufferLayout = layout
      });

      m_PipelinePostProcess = GetWindow().GetGraphicsContext().CreatePipeline({
         .Shaders = {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/PostProcess.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/PostProcess.frag.spv" }
         },
         .BufferLayout = m_VertexBuffer->GetLayout(),
      });
   }


private:
   Pikzel::Input m_Input;

   Camera m_Camera = {
      .Position = {-900.0f, 100.0f, 0.0f},
      .Direction = glm::normalize(glm::vec3{900.0f, -100.0f, 0.0f}),
      .UpVector = {0.0f, 1.0f, 0.0f},
      .FoVRadians = glm::radians(45.f),
      .MoveSpeed = 200.0f,
      .RotateSpeed = 20.0f
   };

   // note: currently shader expects exactly 1 directional light
   std::vector<Pikzel::DirectionalLight> m_DirectionalLights = {
      {
         .Direction = {250.0f, -1750.0f, 250.0f},
         .Color = {0.0f, 0.0f, 0.0f},           // POI: The PBR pipeline is HDR, so there is no reason why we need to limit ourselves to light intensities in range 0 to 1
         .Ambient = {1.0, 1.0, 1.0},            // POI: The ambient light (from environment map) is multiplied by this amount.  This allows us to tone down what might otherwise be too bright environments
         .Size = 0.002f
      }
   };

   glm::mat4 m_LightSpace;

   // note: currently shader supports between 1 and 32 point lights
   // for zero points lights, pass in one, with power set to 0
   std::vector<Pikzel::PointLight> m_PointLights = {
      {
         .Position = {-619.3f, 130.3f, -219.5f},
         .Color = Pikzel::sRGB{1.0f, 1.0f, 1.0f},
         .Size = 1.0f,
         .Power = 30000.0f
      },
      {
         .Position = {487.3f, 130.3f, -219.5f},
         .Color = Pikzel::sRGB{1.0f, 1.0f, 1.0f},
         .Size = 1.0f,
         .Power = 30000.0f
      },
      {
         .Position = {487.3f, 130.3f, 141.1f},
         .Color = Pikzel::sRGB{1.0f, 1.0f, 1.0f},
         .Size = 1.0f,
         .Power = 30000.0f
      },
      {
         .Position = {-619.3f, 130.3f, 141.1f},
         .Color = Pikzel::sRGB{1.0f, 1.0f, 1.0f},
         .Size = 1.0f,
         .Power = 30000.0f
      }
   };


   std::unique_ptr<SponzaPBR::Model> m_Model;
   std::unique_ptr<Pikzel::VertexBuffer> m_VertexBuffer;
   std::unique_ptr<Pikzel::VertexBuffer> m_VertexBufferCube;
   std::unique_ptr<Pikzel::UniformBuffer> m_BufferMatrices;
   std::unique_ptr<Pikzel::UniformBuffer> m_BufferDirectionalLight;
   std::unique_ptr<Pikzel::UniformBuffer> m_BufferLightViews;
   std::unique_ptr<Pikzel::UniformBuffer> m_BufferPointLights;
   std::vector<std::unique_ptr<Pikzel::Texture>> m_Textures;
   std::unique_ptr<Pikzel::Texture> m_Skybox;
   std::unique_ptr<Pikzel::Texture> m_Irradiance;
   std::unique_ptr<Pikzel::Texture> m_SpecularIrradiance;
   std::unique_ptr<Pikzel::Texture> m_SpecularBRDF_LUT;
   std::unique_ptr<Pikzel::Framebuffer> m_FramebufferDirShadow;
   std::unique_ptr<Pikzel::Framebuffer> m_FramebufferPtShadow;
   std::unique_ptr<Pikzel::Framebuffer> m_FramebufferScene;

   std::unique_ptr<Pikzel::Pipeline> m_PipelineLight;
   std::unique_ptr<Pikzel::Pipeline> m_PipelineSkybox;
   std::unique_ptr<Pikzel::Pipeline> m_PipelineDirShadow;
   std::unique_ptr<Pikzel::Pipeline> m_PipelinePtShadow;
   std::unique_ptr<Pikzel::Pipeline> m_PipelinePBR;
   std::unique_ptr<Pikzel::Pipeline> m_PipelinePostProcess;

   Pikzel::DeltaTime m_DeltaTime = {};
   float m_Exposure = 1.0;
   int m_ToneMap = 2;
};


std::unique_ptr<Pikzel::Application> CreateApplication(int argc, const char* argv[]) {
   return std::make_unique<SponzaPBRApp>();
}
