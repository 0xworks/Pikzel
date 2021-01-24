#include "Pikzel/Pikzel.h"
#include "Pikzel/Core/EntryPoint.h"

// note: Pikzel uses reverse-Z so near and far planes are swapped
constexpr float nearPlane = 50.0f;
constexpr float farPlane = 0.1f;

class PBRdemo final : public Pikzel::Application {
public:
   PBRdemo()
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
      m_DeltaTime = deltaTime;
      if (m_Input.IsKeyPressed(Pikzel::KeyCode::Escape)) {
         Exit();
      }
      m_Camera.Update(m_Input, deltaTime);
   }


   virtual void RenderBegin() override {}


   virtual void Render() override {
      PKZL_PROFILE_FUNCTION();

      static int skyboxLod = 1;

      // update buffers
      glm::mat4 view = glm::lookAt(m_Camera.Position, m_Camera.Position + m_Camera.Direction, m_Camera.UpVector);

      Matrices matrices;
      matrices.viewProjection = m_Camera.Projection * view;
      matrices.lightSpace = m_LightSpace;
      matrices.eyePosition = m_Camera.Position;
      m_BufferMatrices->CopyFromHost(0, sizeof(Matrices), &matrices);

      // render to directional light shadow map
      {
         Pikzel::GraphicsContext& gc = m_FramebufferDirShadow->GetGraphicsContext();
         gc.BeginFrame();
         gc.Bind(*m_PipelineDirShadow);

         // Backdrop
         glm::mat4 transform = glm::identity<glm::mat4>();
         gc.PushConstant("constants.mvp"_hs, m_LightSpace * transform);
         for (const auto& mesh : m_ModelBackdrop->Meshes) {
            gc.DrawIndexed(*mesh.VertexBuffer, *mesh.IndexBuffer);
         }

         // Shader balls
         for (size_t ball = 0; ball < m_ShaderBallPositions.size(); ++ball) {
            transform = glm::translate(glm::identity<glm::mat4>(), m_ShaderBallPositions[ball]);
            gc.PushConstant("constants.mvp"_hs, m_LightSpace * transform);
            for (const auto& mesh : m_ModelShaderBall->Meshes) {
               gc.DrawIndexed(*mesh.VertexBuffer, *mesh.IndexBuffer);
            }
         }

         gc.EndFrame();
         gc.SwapBuffers();
      }

      // render scene
      {

         Pikzel::GraphicsContext& gc = m_FramebufferScene->GetGraphicsContext();
         gc.BeginFrame();

         gc.Bind(*m_PipelinePBR);
         gc.Bind(*m_BufferMatrices, "UBOMatrices"_hs);
         gc.Bind(*m_BufferDirectionalLight, "UBODirectionalLight"_hs);
         gc.Bind(m_FramebufferDirShadow->GetDepthTexture(), "uDirShadowMap"_hs);
         gc.Bind(*m_Irradiance, "uIrradiance"_hs);
         gc.Bind(*m_SpecularIrradiance, "uSpecularIrradiance"_hs);
         gc.Bind(*m_SpecularBRDF_LUT, "uSpecularBRDF_LUT"_hs);

         // Backdrop
         glm::mat4 transform = glm::identity<glm::mat4>();
         gc.PushConstant("constants.model"_hs, transform);
         gc.PushConstant("constants.textureRepeat"_hs, m_MaterialBackdrop.TextureRepeat);
         gc.PushConstant("constants.heightScale"_hs, m_MaterialBackdrop.HeightScale);
         gc.Bind(*m_Textures[(int)m_MaterialBackdrop.BaseColor], "uAlbedo"_hs);
         gc.Bind(*m_Textures[(int)m_MaterialBackdrop.MetallicRoughness], "uMetallicRoughness"_hs);
         gc.Bind(*m_Textures[(int)m_MaterialBackdrop.Normals], "uNormals"_hs);
         gc.Bind(*m_Textures[(int)m_MaterialBackdrop.AmbientOcclusion], "uAmbientOcclusion"_hs);
         gc.Bind(*m_Textures[(int)m_MaterialBackdrop.HeightMap], "uHeightMap"_hs);
         for (const auto& mesh : m_ModelBackdrop->Meshes) {
            gc.DrawIndexed(*mesh.VertexBuffer, *mesh.IndexBuffer);
         }

         // Shader balls
         for (size_t ball = 0; ball < m_ShaderBallPositions.size(); ++ball) {
            transform = glm::translate(glm::identity<glm::mat4>(), m_ShaderBallPositions[ball]);
            gc.PushConstant("constants.model"_hs, transform);
            for (size_t mesh = 0; mesh < m_ModelShaderBall->Meshes.size(); ++mesh) {
               gc.PushConstant("constants.textureRepeat"_hs, m_MaterialShaderBall[ball][mesh].TextureRepeat);
               gc.PushConstant("constants.heightScale"_hs, m_MaterialBackdrop.HeightScale);
               gc.Bind(*m_Textures[(int)m_MaterialShaderBall[ball][mesh].BaseColor], "uAlbedo"_hs);
               gc.Bind(*m_Textures[(int)m_MaterialShaderBall[ball][mesh].MetallicRoughness], "uMetallicRoughness"_hs);
               gc.Bind(*m_Textures[(int)m_MaterialShaderBall[ball][mesh].Normals], "uNormals"_hs);
               gc.Bind(*m_Textures[(int)m_MaterialShaderBall[ball][mesh].AmbientOcclusion], "uAmbientOcclusion"_hs);
               gc.Bind(*m_Textures[(int)m_MaterialShaderBall[ball][mesh].HeightMap], "uHeightMap"_hs);
               gc.DrawIndexed(*m_ModelShaderBall->Meshes[mesh].VertexBuffer, *m_ModelShaderBall->Meshes[mesh].IndexBuffer);
            }
         }

         // Skybox
         view = glm::mat3(view);
         gc.Bind(*m_PipelineSkybox);
         gc.Bind(*m_Skybox, "uSkybox"_hs);
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
      gc.Bind(m_FramebufferScene->GetColorTexture(0), "uTexture"_hs);
      gc.DrawTriangles(*m_VertexBuffer, 6);

      GetWindow().BeginImGuiFrame();
      {
         ImGui::Begin("Renderer");
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

      Pikzel::BufferLayout layout = {
         {"inPos",       Pikzel::DataType::Vec3},
         {"inTexCoords", Pikzel::DataType::Vec2},
      };
      m_VertexBuffer = Pikzel::RenderCore::CreateVertexBuffer(layout, sizeof(vertices), vertices);

      // POI: we load the backdrop and shader ball from .obj files that were exported from Blender
      m_ModelBackdrop = Pikzel::ModelSerializer::Import("Assets/" APP_NAME "/Models/Backdrop.obj");
      m_ModelShaderBall = Pikzel::ModelSerializer::Import("Assets/" APP_NAME "/Models/ShaderBall.obj");
   }


   struct Matrices {
      glm::mat4 viewProjection;
      glm::mat4 lightSpace;
      glm::vec3 eyePosition;
   };

   void CreateUniformBuffers() {

      glm::mat4 lightProjection = glm::ortho(-15.0f, 15.0f, -10.0f, 10.0f, 20.0f, 0.0f);  // TODO: need to automatically determine correct parameters here (+ cascades...)
      glm::mat4 lightView = glm::lookAt(-m_DirectionalLights[0].Direction, glm::vec3 {0.0f, 0.0f, 0.0f}, glm::vec3 {0.0f, 1.0f, 0.0f});
      m_LightSpace = lightProjection * lightView;

      m_BufferMatrices = Pikzel::RenderCore::CreateUniformBuffer(sizeof(Matrices));
      m_BufferDirectionalLight = Pikzel::RenderCore::CreateUniformBuffer(sizeof(m_DirectionalLights), m_DirectionalLights);
   }


   void CreateTextures() {
      for (const auto [textureId, textureSettings] : m_TextureLibrary) {
         m_Textures.emplace_back(Pikzel::RenderCore::CreateTexture(textureSettings));
      }
      m_Skybox = Pikzel::RenderCore::CreateTexture({.Type = Pikzel::TextureType::TextureCube, .Path = "Assets/Skyboxes/birchwood_4k.hdr"});

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

   }


   void CreatePipelines() {
      m_PipelineSkybox = m_FramebufferScene->GetGraphicsContext().CreatePipeline({
         .Shaders = {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/Skybox.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/Skybox.frag.spv" }
         },
         .BufferLayout = m_VertexBuffer->GetLayout()
      });

      m_PipelineDirShadow = m_FramebufferDirShadow->GetGraphicsContext().CreatePipeline({
         .Shaders = {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/Depth.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/Depth.frag.spv" }
         },
         .BufferLayout = {
            { "inPos",     Pikzel::DataType::Vec3 },
            { "inNormal",  Pikzel::DataType::Vec3 },
            { "inTangent", Pikzel::DataType::Vec3 },
            { "inUV",      Pikzel::DataType::Vec2 },
         }
      });

      // POI: here is the PBR pipeline
      m_PipelinePBR= m_FramebufferScene->GetGraphicsContext().CreatePipeline({
         .Shaders = {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/PBR.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/PBR.frag.spv" }
         },
         .BufferLayout = {
            { "inPos",     Pikzel::DataType::Vec3 },
            { "inNormal",  Pikzel::DataType::Vec3 },
            { "inTangent", Pikzel::DataType::Vec3 },
            { "inUV",      Pikzel::DataType::Vec2 },
         }
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

   // POI: TextureId here is just a convienient way to refer to textures later on.
   enum class TextureId {
      DefaultColor = 0,
      DefaultMR,
      DefaultNormals,
      DefaultAO,
      DefaultHeight,
      BackdropColor,
      Gold,
      Red,
      Bamboo,
      BambooMR,
      BambooNormals,
      BambooAO,
      ButtonedSquareLeatherOxblood,
      ButtonedSquareLeatherMR,
      ButtonedSquareLeatherNormals,
      ButtonedSquareLeatherAO,
      ButtonedSquareLeatherHeight,
      Concrete,
      ConcreteMR,
      ConcreteNormals,
      ConcreteAO,
      ConcreteHeight,
      ScuffedPlasticMR,
      ScuffedPlasticNormals,
      SmoothMetalMR,
      SmoothMetalNormals,
      WoodVeneer,
      WoodVeneerMR,
      WoodVeneerNormals,
      WoodVeneerAO,
      WoodVeneerHeight,
   };

   // POI: And here is the mapping from TextureId to the actual asset.
   inline static const std::map<TextureId, Pikzel::TextureSettings> m_TextureLibrary = {
      { TextureId::DefaultColor,                   {.Path = "Assets/" APP_NAME "/Textures/DefaultColor.png",                            .Format = Pikzel::TextureFormat::SRGBA8}},
      { TextureId::DefaultMR,                      {.Path = "Assets/" APP_NAME "/Textures/DefaultMetallicRoughness.png",                .Format = Pikzel::TextureFormat::RGBA8}},
      { TextureId::DefaultNormals,                 {.Path = "Assets/" APP_NAME "/Textures/DefaultNormals.png",                          .Format = Pikzel::TextureFormat::RGBA8}},
      { TextureId::DefaultAO,                      {.Path = "Assets/" APP_NAME "/Textures/DefaultAmbientOcclusion.png",                 .Format = Pikzel::TextureFormat::RGBA8}},
      { TextureId::DefaultHeight,                  {.Path = "Assets/" APP_NAME "/Textures/DefaultHeight.png",                           .Format = Pikzel::TextureFormat::RGBA8}},
      { TextureId::BackdropColor,                  {.Path = "Assets/" APP_NAME "/Textures/BackdropColor.png",                           .Format = Pikzel::TextureFormat::SRGBA8}},
      { TextureId::Gold,                           {.Path = "Assets/" APP_NAME "/Textures/Gold.png",                                    .Format = Pikzel::TextureFormat::SRGBA8}},
      { TextureId::Red,                            {.Path = "Assets/" APP_NAME "/Textures/Red.png",                                     .Format = Pikzel::TextureFormat::SRGBA8}},
      { TextureId::Bamboo,                         {.Path = "Assets/" APP_NAME "/Textures/Bamboo.png",                                  .Format = Pikzel::TextureFormat::SRGBA8}},
      { TextureId::BambooMR,                       {.Path = "Assets/" APP_NAME "/Textures/BambooMetallicRoughness.png",                 .Format = Pikzel::TextureFormat::RGBA8}},
      { TextureId::BambooNormals,                  {.Path = "Assets/" APP_NAME "/Textures/BambooNormals.png",                           .Format = Pikzel::TextureFormat::RGBA8}},
      { TextureId::BambooAO,                       {.Path = "Assets/" APP_NAME "/Textures/BambooAmbientOcclusion.png",                  .Format = Pikzel::TextureFormat::RGBA8}},
      { TextureId::ButtonedSquareLeatherOxblood,   {.Path = "Assets/" APP_NAME "/Textures/ButtonedSquareLeatherOxblood.png",            .Format = Pikzel::TextureFormat::SRGBA8}},
      { TextureId::ButtonedSquareLeatherMR,        {.Path = "Assets/" APP_NAME "/Textures/ButtonedSquareLeatherMetallicRoughness.png",  .Format = Pikzel::TextureFormat::RGBA8}},
      { TextureId::ButtonedSquareLeatherNormals,   {.Path = "Assets/" APP_NAME "/Textures/ButtonedSquareLeatherNormals.png",            .Format = Pikzel::TextureFormat::RGBA8}},
      { TextureId::ButtonedSquareLeatherAO,        {.Path = "Assets/" APP_NAME "/Textures/ButtonedSquareLeatherAmbientOcclusion.png",   .Format = Pikzel::TextureFormat::RGBA8}},
      { TextureId::ButtonedSquareLeatherHeight,    {.Path = "Assets/" APP_NAME "/Textures/ButtonedSquareLeatherHeightMap.png",          .Format = Pikzel::TextureFormat::RGBA8}},
      { TextureId::Concrete,                       {.Path = "Assets/" APP_NAME "/Textures/Concrete.png",                                .Format = Pikzel::TextureFormat::SRGBA8}},
      { TextureId::ConcreteMR,                     {.Path = "Assets/" APP_NAME "/Textures/ConcreteMetallicRoughness.png",               .Format = Pikzel::TextureFormat::RGBA8}},
      { TextureId::ConcreteNormals,                {.Path = "Assets/" APP_NAME "/Textures/ConcreteNormals.png",                         .Format = Pikzel::TextureFormat::RGBA8}},
      { TextureId::ConcreteAO,                     {.Path = "Assets/" APP_NAME "/Textures/ConcreteAmbientOcclusion.png",                .Format = Pikzel::TextureFormat::RGBA8}},
      { TextureId::ConcreteHeight,                 {.Path = "Assets/" APP_NAME "/Textures/ConcreteHeight.png",                          .Format = Pikzel::TextureFormat::RGBA8}},
      { TextureId::ScuffedPlasticMR,               {.Path = "Assets/" APP_NAME "/Textures/ScuffedPlasticMetallicRoughness.png",         .Format = Pikzel::TextureFormat::RGBA8}},
      { TextureId::ScuffedPlasticNormals,          {.Path = "Assets/" APP_NAME "/Textures/ScuffedPlasticNormals.png",                   .Format = Pikzel::TextureFormat::RGBA8}},
      { TextureId::SmoothMetalMR,                  {.Path = "Assets/" APP_NAME "/Textures/SmoothMetalMetallicRoughness.png",            .Format = Pikzel::TextureFormat::RGBA8}},
      { TextureId::SmoothMetalNormals,             {.Path = "Assets/" APP_NAME "/Textures/SmoothMetalNormals.png",                      .Format = Pikzel::TextureFormat::RGBA8}},
      { TextureId::WoodVeneer,                     {.Path = "Assets/" APP_NAME "/Textures/WoodVeneer.png",                              .Format = Pikzel::TextureFormat::SRGBA8}},
      { TextureId::WoodVeneerMR,                   {.Path = "Assets/" APP_NAME "/Textures/WoodVeneerMetallicRoughness.png",             .Format = Pikzel::TextureFormat::RGBA8}},
      { TextureId::WoodVeneerNormals,              {.Path = "Assets/" APP_NAME "/Textures/WoodVeneerNormals.png",                       .Format = Pikzel::TextureFormat::RGBA8}},
      { TextureId::WoodVeneerAO,                   {.Path = "Assets/" APP_NAME "/Textures/WoodVeneerAmbientOcclusion.png",              .Format = Pikzel::TextureFormat::RGBA8}},
      { TextureId::WoodVeneerHeight,               {.Path = "Assets/" APP_NAME "/Textures/WoodVeneerHeight.png",                        .Format = Pikzel::TextureFormat::RGBA8}},
   };

   // POI: Our PBR "material" consists of four textures, as follows.
   //      BaseColor         = The basic color of the material (sometimes called "Albedo")
   //      MetallicRoughness = The metalness and roughness packed into one texture.
   //                          Here using the GLTF convention where metalness is in the blue channel, and Rroughness is in the green channel.
   //      AmbientOcclusion  = How much is ambient light occluded (0 = fully occluded, 1 = not occluded).  The shader only reads from the red channel. 
   //      Normals           = Normal map.
   //
   // The default constructed material is 100% white, totally non metallic with neutral roughness, no ambient occlusion, and perfectly flat normals.
   struct Material {
      TextureId BaseColor = TextureId::DefaultColor;
      TextureId MetallicRoughness = TextureId::DefaultMR;
      TextureId Normals = TextureId::DefaultNormals;
      TextureId AmbientOcclusion = TextureId::DefaultAO;
      TextureId HeightMap = TextureId::DefaultHeight;
      glm::vec2 TextureRepeat = {1.0f, 1.0f};
      float HeightScale = 0.05;
   };

private:
   Pikzel::Input m_Input;

   Camera m_Camera = {
      .Position = {0.0f, 5.0f, 10.0f},
      .Direction = glm::normalize(glm::vec3{0.0f, -0.5f, -10.0f}),
      .UpVector = {0.0f, 1.0f, 0.0f},
      .FoVRadians = glm::radians(45.f),
      .MoveSpeed = 2.0f,
      .RotateSpeed = 10.0f
   };

   // note: shader expects exactly 1
   Pikzel::DirectionalLight m_DirectionalLights[1] = {
      {
         .Direction = { -4.0f, -10.0f, -5.0f},
         .Color = {4.0f, 4.0f, 4.0f},           // POI: The PBR pipeline is HDR, so there is no reason why we need to limit ourselves to light intensities in range 0 to 1
         .Ambient = {1.0, 1.0, 1.0},            // POI: The ambient light (from environment map) is multiplied by this amount.  This allows us to tone down what might otherwise be too bright environments
         .Size = 0.02
      }
   };
   glm::mat4 m_LightSpace;

   std::unique_ptr<Pikzel::Model> m_ModelBackdrop;
   std::unique_ptr<Pikzel::Model> m_ModelShaderBall;
   std::unique_ptr<Pikzel::VertexBuffer> m_VertexBuffer;
   std::unique_ptr<Pikzel::UniformBuffer> m_BufferMatrices;
   std::unique_ptr<Pikzel::UniformBuffer> m_BufferDirectionalLight;
   std::vector<std::unique_ptr<Pikzel::Texture>> m_Textures;
   std::unique_ptr<Pikzel::Texture> m_Skybox;
   std::unique_ptr<Pikzel::Texture> m_Irradiance;
   std::unique_ptr<Pikzel::Texture> m_SpecularIrradiance;
   std::unique_ptr<Pikzel::Texture> m_SpecularBRDF_LUT;
   std::unique_ptr<Pikzel::Framebuffer> m_FramebufferDirShadow;
   std::unique_ptr<Pikzel::Framebuffer> m_FramebufferScene;
   std::unique_ptr<Pikzel::Pipeline> m_PipelineSkybox;
   std::unique_ptr<Pikzel::Pipeline> m_PipelineDirShadow;
   std::unique_ptr<Pikzel::Pipeline> m_PipelinePBR;
   std::unique_ptr<Pikzel::Pipeline> m_PipelinePostProcess;

   Material m_MaterialBackdrop = {.BaseColor = TextureId::BackdropColor, .TextureRepeat = {10.0f, 7.5f}};

   std::vector<glm::vec3> m_ShaderBallPositions = {
      {0.0f, 0.0f, 0.0f},
      {2.0f, 0.0f, 0.0f},
      {4.0f, 0.0f, 0.0f},
      {-2.0f, 0.0f, 0.0f},
      {-4.0f, 0.0f, 0.0f},
   };

   Material neutral = {};
   Material goldMetal = {
      .BaseColor = TextureId::Gold,
      .MetallicRoughness = TextureId::SmoothMetalMR,
      .Normals = TextureId::SmoothMetalNormals
   };
   Material redPlastic = {
      .BaseColor = TextureId::Red,
      .MetallicRoughness = TextureId::ScuffedPlasticMR,
      .Normals = TextureId::ScuffedPlasticNormals
   };
   Material buttonedLeather = {
      .BaseColor = TextureId::ButtonedSquareLeatherOxblood,
      .MetallicRoughness = TextureId::ButtonedSquareLeatherMR,
      .Normals = TextureId::ButtonedSquareLeatherNormals,
      .AmbientOcclusion = TextureId::ButtonedSquareLeatherAO,
      .HeightMap = TextureId::ButtonedSquareLeatherHeight,
      .TextureRepeat = {3.0f, 3.0f}
   };
   Material bamboo = {
      .BaseColor = TextureId::Bamboo,
      .MetallicRoughness = TextureId::BambooMR,
      .Normals = TextureId::BambooNormals,
      .AmbientOcclusion = TextureId::BambooAO,
      .TextureRepeat = {3.0f,3.0f}
   };
   Material concrete = {
      .BaseColor = TextureId::Concrete,
      .MetallicRoughness = TextureId::ConcreteMR,
      .Normals = TextureId::ConcreteNormals,
      .AmbientOcclusion = TextureId::ConcreteAO,
      .HeightMap = TextureId::ConcreteHeight,
   };
   Material woodVeneer = {
      .BaseColor = TextureId::WoodVeneer,
      .MetallicRoughness = TextureId::WoodVeneerMR,
      .Normals = TextureId::WoodVeneerNormals,
      .AmbientOcclusion = TextureId::WoodVeneerAO,
      .HeightMap = TextureId::WoodVeneerHeight,
   };


   // POI: Here we define the Materials to use for each of the five shader balls in the scene.
   //      Note that each ball consists of five separate meshes, and we can specify a different material for each mesh.
   std::vector<std::array<Material, 5>> m_MaterialShaderBall = {
      // Ball 0
      {
         redPlastic    /* outer ball*/,
         neutral       /* inner base */,
         neutral       /* stand */,
         neutral       /* outer base */,
         neutral       /* inner ball */
      },
      // Ball 1
      {
         goldMetal     /* outer ball*/,
         goldMetal     /* inner base */,
         goldMetal     /* stand */,
         goldMetal     /* outer base */,
         goldMetal     /* inner ball */
      },
      // Ball 2
      {
         buttonedLeather  /* outer ball */,
         bamboo           /* inner base */,
         bamboo           /* stand */,
         bamboo           /* outer base */,
         goldMetal        /* inner ball */,
      },
      // Ball 3
      {
         concrete      /* outer ball*/,
         bamboo       /* inner base */,
         bamboo       /* stand */,
         bamboo       /* outer base */,
         bamboo       /* inner ball */
      },
      // Ball 4
      {
         woodVeneer    /* outer ball */,
         bamboo   /* inner base */,
         bamboo   /* stand */,
         bamboo   /* outer base */,
         bamboo   /* inner ball */
      }
   };

   Pikzel::DeltaTime m_DeltaTime = {};
   float m_Exposure = 1.0;
   int m_ToneMap = 2;
};


std::unique_ptr<Pikzel::Application> CreateApplication(int argc, const char* argv[]) {
   return std::make_unique<PBRdemo>();
}
