#include "ModelSerializer.h"

#include "Pikzel/Pikzel.h"
#include "Pikzel/Core/EntryPoint.h"

#include <array>
#include <format>
#include <memory>
#include <utility>
#include <vector>

// note: Pikzel uses reverse-Z so near and far planes are swapped
constexpr float nearPlane = 100.0f;
constexpr float farPlane = 0.01f;

class SponzaPBRApp final : public Pikzel::Application {
using super = Pikzel::Application;
public:
   SponzaPBRApp()
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
      static float lightRadius = 10.0f; // TODO: set light radius appropriately
      static glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), 1.0f, lightRadius, 0.01f);  // note: Pikzel uses reverse-Z so near and far planes are swapped
      static int skyboxLod = 1;

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
      {
         Pikzel::GraphicsContext& gc = m_FramebufferDirShadow->GetGraphicsContext();
         gc.BeginFrame();
         gc.Bind(*m_PipelineDirShadow);

         // Model
         glm::mat4 transform = glm::identity<glm::mat4>();
         gc.PushConstant("constants.mvp"_hs, m_LightSpace * transform);
         for (const auto& mesh : m_Model->Meshes) {
            gc.PushConstant("constants.mvp"_hs, m_LightSpace * transform * mesh.Transform);
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

            glm::mat4 transform = glm::identity<glm::mat4>();
            gcPtShadows.PushConstant("constants.model"_hs, transform);
            for (const auto& mesh : m_Model->Meshes) {
               gcPtShadows.PushConstant("constants.model"_hs, transform * mesh.Transform);
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
            gc.PushConstant("constants.model"_hs, transform * mesh.Transform);
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
            glm::mat4 model = glm::scale(glm::translate(glm::identity<glm::mat4>(), pointLight.position), glm::vec3{ pointLight.size });
            gc.PushConstant("constants.mvp"_hs, matrices.viewProjection * model);
            gc.PushConstant("constants.lightColor"_hs, pointLight.color);
            gc.DrawTriangles(*m_VertexBufferCube, 36);
         }

         // Skybox
         view = glm::mat3(view);
         gc.Bind(*m_PipelineSkybox);
         gc.Bind("uSkybox"_hs, *m_Skybox);
         gc.PushConstant("constants.vp"_hs, m_Camera.projection * view);
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
            ImGuiDrawPointLight(std::format("light {}", i).c_str(), m_PointLights[i]);
         }
         ImGui::Text("Frame time: %.3fms (%.0f FPS)", m_DeltaTime.count() * 1000.0f, 1.0f / m_DeltaTime.count());
         ImGui::Text("Tone mapping:");
         ImGui::RadioButton("None", &m_ToneMap, 0);
         ImGui::RadioButton("Reinhard", &m_ToneMap, 1);
         ImGui::RadioButton("Exposure", &m_ToneMap, 2);
         ImGui::SameLine();
         ImGui::DragFloat("##", &m_Exposure, 0.1f, 0, 10);
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
      super::OnWindowResize(event);
      m_Camera.projection = glm::perspective(m_Camera.fovRadians, static_cast<float>(GetWindow().GetWidth()) / static_cast<float>(GetWindow().GetHeight()), nearPlane, farPlane);

      // recreate framebuffer with new size
      CreateFramebuffers();
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
      m_Model = SponzaPBR::ModelSerializer::Import("Assets/Models/Sponza/Sponza.gltf");
   }


   struct Matrices {
      glm::mat4 viewProjection;
      glm::mat4 lightSpace;
      glm::vec3 eyePosition;
   };

   void CreateUniformBuffers() {
      glm::mat4 lightProjection = glm::ortho(-21.0f, 21.0f, -20.0f, 20.0f, 20.0f, -10.0f);  // TODO: need to automatically determine correct parameters here (+ cascades...)
      glm::mat4 lightView = glm::lookAt(-m_DirectionalLights[0].direction, glm::vec3 {0.0f, 0.0f, 0.0f}, glm::vec3 {0.0f, 1.0f, 0.0f});
      m_LightSpace = lightProjection * lightView;

      m_BufferMatrices = Pikzel::RenderCore::CreateUniformBuffer(sizeof(Matrices));
      m_BufferDirectionalLight = Pikzel::RenderCore::CreateUniformBuffer(sizeof(Pikzel::DirectionalLight) * m_DirectionalLights.size());
      m_BufferLightViews = Pikzel::RenderCore::CreateUniformBuffer(sizeof(glm::mat4) * m_PointLights.size() * 6);
      m_BufferPointLights = Pikzel::RenderCore::CreateUniformBuffer(sizeof(Pikzel::PointLight) * m_PointLights.size());
   }


   void CreateTextures() {
      m_Skybox = Pikzel::RenderCore::CreateTexture({.textureType = Pikzel::TextureType::TextureCube, .path = "Assets/Skyboxes/dikhololo_night_4k.hdr"});

      // POI: We use compute shaders to bake lighting from the skybox image into diffuse and specular "Irradiance maps"

      // diffuse irradiance
      m_Irradiance = Pikzel::RenderCore::CreateTexture({
         .textureType = Pikzel::TextureType::TextureCube,
         .width = 32,                                // POI: The diffuse irradiance map does not need to be very big
         .height = 32,
         .format = Pikzel::TextureFormat::RGBA16F,
         .imageStorage = true                        // POI: Textures that will be written to in a shader need this flag set (because Vulkan), and also need to be Commit()'d before you use them in shader
      });
      std::unique_ptr<Pikzel::ComputeContext> compute = Pikzel::RenderCore::CreateComputeContext();
      std::unique_ptr<Pikzel::Pipeline> pipelineIrradiance = compute->CreatePipeline({
         .shaders = {
            { Pikzel::ShaderType::Compute, "Renderer/EnvironmentIrradiance.comp.spv"}
         }
      });
 
      compute->Begin();
      compute->Bind(*pipelineIrradiance);
      compute->Bind("inputTexture"_hs, *m_Skybox);
      compute->Bind("outputTexture"_hs, *m_Irradiance);
      compute->Dispatch(m_Irradiance->GetWidth() / 32, m_Irradiance->GetHeight() / 32, 6);  // POI: width and height divided by 32 because the shader works in 32x32 blocks.  z is 6 for the six faces of the cube
      compute->End();
      m_Irradiance->Commit(0);

      // specular irradiance
      m_SpecularIrradiance = Pikzel::RenderCore::CreateTexture({
         .textureType = Pikzel::TextureType::TextureCube,
         .width = m_Skybox->GetWidth(),                              // POI: The specular irradiance map cannot be as small as the diffuse one, otherwise details will be lost in reflections off highly reflective materials.
         .height = m_Skybox->GetHeight(),
         .format = Pikzel::TextureFormat::RGBA16F,
         .imageStorage = true
      });

      // POI: specular irradiance mip level 0 is a straight copy of the skybox.
      //      this level is used for perfectly reflective materials
      m_SpecularIrradiance->CopyFrom(*m_Skybox);

      // POI: specular irradiance mip levels 1..N are computed by pre-filtering the skybox
      //      per Epic Games split sum approximation.
      std::unique_ptr<Pikzel::Pipeline> pipelinePrefilter = compute->CreatePipeline({
         .shaders = {
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
         .width = 512,
         .height = 512,
         .format = Pikzel::TextureFormat::RG16F,      // POI: The lookup table only needs two dimensions. Which we store as 16-bit floats in the Red and Green channels.
         .wrapU = Pikzel::TextureWrap::ClampToEdge,   // POI: The lookup table must clamp to edge, otherwise there will be artefacts when looking up the table for values near the edges
         .wrapV = Pikzel::TextureWrap::ClampToEdge,
         .mipLevels = 1,
         .imageStorage = true
      });
      std::unique_ptr<Pikzel::Pipeline> pipelineSpecularBRDF = compute->CreatePipeline({
         .shaders = {
            { Pikzel::ShaderType::Compute, "Renderer/EnvironmentSpecularBRDF.comp.spv"}
         }
      });
      compute->Begin();
      compute->Bind(*pipelineSpecularBRDF);
      compute->Bind("LUT"_hs, *m_SpecularBRDF_LUT);
      compute->Dispatch(m_SpecularBRDF_LUT->GetWidth() / 32, m_SpecularBRDF_LUT->GetHeight() / 32, 1);
      compute->End();
      m_SpecularBRDF_LUT->Commit(0);
   }


   void CreateFramebuffers() {
      const uint32_t shadowMapWidth = 4096;
      const uint32_t shadowMapHeight = 4096;

      m_FramebufferScene = Pikzel::RenderCore::CreateFramebuffer({
         .width = GetWindow().GetWidth(),
         .height = GetWindow().GetHeight(),
         .msaaNumSamples = 4,
         .clearColorValue = GetWindow().GetClearColor(),
         .attachments = {
            {Pikzel::AttachmentType::Color, Pikzel::TextureFormat::RGBA16F},
            {Pikzel::AttachmentType::Depth, Pikzel::TextureFormat::D32F}
         }
      });

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
      m_PipelineSkybox = m_FramebufferScene->GetGraphicsContext().CreatePipeline({
         .shaders = {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/Skybox.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/Skybox.frag.spv" }
         },
         .bufferLayout = m_VertexBuffer->GetLayout()
      });

      m_PipelineLight = m_FramebufferScene->GetGraphicsContext().CreatePipeline({
         .shaders = {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/Light.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/Light.frag.spv" }
         },
         .bufferLayout = m_VertexBufferCube->GetLayout(),
      });

      Pikzel::BufferLayout layout = {
         { "inPos",     Pikzel::DataType::Vec3 },
         { "inNormal",  Pikzel::DataType::Vec3 },
         { "inTangent", Pikzel::DataType::Vec3 },
         { "inUV",      Pikzel::DataType::Vec2 },
      };

      m_PipelineDirShadow = m_FramebufferDirShadow->GetGraphicsContext().CreatePipeline({
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

      // POI: here is the PBR pipeline
      m_PipelinePBR = m_FramebufferScene->GetGraphicsContext().CreatePipeline({
         .shaders = {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/PBR.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/PBR.frag.spv" }
         },
         .bufferLayout = layout
      });

      m_PipelinePostProcess = GetWindow().GetGraphicsContext().CreatePipeline({
         .shaders = {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/PostProcess.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/PostProcess.frag.spv" }
         },
         .bufferLayout = m_VertexBuffer->GetLayout(),
      });
   }


private:
   Pikzel::Input m_Input;

   Camera m_Camera = {
      .position = {-9.0f, 1.0f, 0.0f},
      .direction = glm::normalize(glm::vec3{9.0f, -1.0f, 0.0f}),
      .upVector = {0.0f, 1.0f, 0.0f},
      .fovRadians = glm::radians(45.f),
      .moveSpeed = 2.0f,
      .rotateSpeed = 20.0f
   };

   // note: currently shader expects exactly 1 directional light
   std::vector<Pikzel::DirectionalLight> m_DirectionalLights = {
      {
         .direction = {5.00f, -13.5f, 7.00f},
         .color = {1.0f, 1.0f, 1.0f},           // POI: The PBR pipeline is HDR, so there is no reason why we need to limit ourselves to light intensities in range 0 to 1
         .ambient = {1.0, 1.0, 1.0},            // POI: The ambient light (from environment map) is multiplied by this amount.  This allows us to tone down what might otherwise be too bright environments
         .size = 0.002f
      }
   };

   glm::mat4 m_LightSpace;

   // note: currently shader supports between 1 and 32 point lights
   // for zero points lights, pass in one, with power set to 0
   std::vector<Pikzel::PointLight> m_PointLights = {
      {
         .position = {-4.96f, 1.1f, -1.76f},
         .color = Pikzel::sRGB{1.0f, 1.0f, 1.0f},
         .size = 0.01f,
         .power = 1.0f
      },
      {
         .position = {3.9f, 1.1f, -1.76f},
         .color = Pikzel::sRGB{1.0f, 1.0f, 1.0f},
         .size = 0.01f,
         .power = 1.0f
      },
      {
         .position = {3.9f, 1.1f, 1.15f},
         .color = Pikzel::sRGB{1.0f, 1.0f, 1.0f},
         .size = 0.01f,
         .power = 1.0f
      },
      {
         .position = {-4.96f, 1.1f, 1.15f},
         .color = Pikzel::sRGB{1.0f, 1.0f, 1.0f},
         .size = 0.01f,
         .power = 1.0f
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
