#include "Pikzel/Pikzel.h"
#include "Pikzel/Core/EntryPoint.h"

#include <optional>

// note: Pikzel uses reverse-Z so near and far planes are swapped
const float nearPlane = 1000.0f;
const float farPlane = 0.1f;


class Skybox final : public Pikzel::Application {
public:
   Skybox()
   : Pikzel::Application {{.Title = APP_DESCRIPTION, .ClearColor = Pikzel::sRGB{0.1f, 0.1f, 0.2f}, .IsVSync = true}}
   , m_Input {GetWindow()}
   {
      CreateVertexBuffer();
      CreateTextures();
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

      if (m_NewSkyboxPath.has_value()) {
         LoadSkybox(m_NewSkyboxPath.value());
         m_NewSkyboxPath.reset();
      }
   }


   virtual void Render() override {
      PKZL_PROFILE_FUNCTION();

      static int lod = 0;

      glm::mat4 transform = glm::identity<glm::mat4>();
      glm::mat4 view = glm::lookAt(m_Camera.Position, m_Camera.Position + m_Camera.Direction, m_Camera.UpVector);

      Pikzel::GraphicsContext& gc = GetWindow().GetGraphicsContext();

      gc.Bind(*m_ScenePipeline);

      gc.Bind(*m_TextureContainer, "uTexture"_hs);
      glm::mat4 model = glm::translate(glm::identity<glm::mat4>(), glm::vec3(-1.0f, 0.0f, -1.0f));
      gc.PushConstant("constants.mvp"_hs, m_Camera.Projection * view * model);
      gc.DrawTriangles(*m_VertexBuffer, 36, 36);

      model = glm::translate(glm::identity<glm::mat4>(), glm::vec3(2.0f, 0.0f, 0.0f));
      gc.PushConstant("constants.mvp"_hs, m_Camera.Projection * view * model);
      gc.DrawTriangles(*m_VertexBuffer, 36, 36);

      gc.Bind(*m_TextureFloor, "uTexture"_hs);
      model = glm::identity<glm::mat4>();
      gc.PushConstant("constants.mvp"_hs, m_Camera.Projection * view * model);
      gc.DrawTriangles(*m_VertexBuffer, 6, 72);


      view = glm::mat3(view);
      gc.Bind(*m_SkyboxPipeline);

      gc.Bind(*m_Skybox, "uSkybox"_hs);
      gc.PushConstant("constants.vp"_hs, m_Camera.Projection * view);
      gc.PushConstant("constants.lod"_hs, lod);
      gc.DrawTriangles(*m_VertexBuffer, 36);

      GetWindow().BeginImGuiFrame();
      ImGui::Begin("Environment");
      if (ImGui::Button("Load skybox")) {
         m_NewSkyboxPath = Pikzel::OpenFileDialog();
      }
      ImGui::InputInt("Lod", &lod, 1.0f);
      ImGui::End();
      GetWindow().EndImGuiFrame();
   }


   virtual void OnWindowResize(const Pikzel::WindowResizeEvent& event) override {
      __super::OnWindowResize(event);
      m_Camera.Projection = glm::perspective(m_Camera.FoVRadians, static_cast<float>(GetWindow().GetWidth()) / static_cast<float>(GetWindow().GetHeight()), nearPlane, farPlane);
   }


private:

   struct Vertex {
      glm::vec3 Pos;
      glm::vec2 TexCoords;
   };

   void CreateVertexBuffer() {
      Vertex vertices[] = {
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

         // Cube (note opposite winding order from skybox)
         {.Pos{-0.5f, -0.5f, -0.5f}, .TexCoords{1.0f, 0.0f}},
         {.Pos{ 0.5f,  0.5f, -0.5f}, .TexCoords{0.0f, 1.0f}},
         {.Pos{ 0.5f, -0.5f, -0.5f}, .TexCoords{0.0f, 0.0f}},
         {.Pos{ 0.5f,  0.5f, -0.5f}, .TexCoords{0.0f, 1.0f}},
         {.Pos{-0.5f, -0.5f, -0.5f}, .TexCoords{1.0f, 0.0f}},
         {.Pos{-0.5f,  0.5f, -0.5f}, .TexCoords{1.0f, 1.0f}},

         {.Pos{-0.5f, -0.5f,  0.5f}, .TexCoords{0.0f, 0.0f}},
         {.Pos{ 0.5f, -0.5f,  0.5f}, .TexCoords{1.0f, 0.0f}},
         {.Pos{ 0.5f,  0.5f,  0.5f}, .TexCoords{1.0f, 1.0f}},
         {.Pos{ 0.5f,  0.5f,  0.5f}, .TexCoords{1.0f, 1.0f}},
         {.Pos{-0.5f,  0.5f,  0.5f}, .TexCoords{0.0f, 1.0f}},
         {.Pos{-0.5f, -0.5f,  0.5f}, .TexCoords{0.0f, 0.0f}},

         {.Pos{-0.5f,  0.5f,  0.5f}, .TexCoords{1.0f, 1.0f}},
         {.Pos{-0.5f,  0.5f, -0.5f}, .TexCoords{0.0f, 1.0f}},
         {.Pos{-0.5f, -0.5f, -0.5f}, .TexCoords{0.0f, 0.0f}},
         {.Pos{-0.5f, -0.5f, -0.5f}, .TexCoords{0.0f, 0.0f}},
         {.Pos{-0.5f, -0.5f,  0.5f}, .TexCoords{1.0f, 0.0f}},
         {.Pos{-0.5f,  0.5f,  0.5f}, .TexCoords{1.0f, 1.0f}},

         {.Pos{ 0.5f,  0.5f,  0.5f}, .TexCoords{0.0f, 1.0f}},
         {.Pos{ 0.5f, -0.5f, -0.5f}, .TexCoords{1.0f, 0.0f}},
         {.Pos{ 0.5f,  0.5f, -0.5f}, .TexCoords{1.0f, 1.0f}},
         {.Pos{ 0.5f, -0.5f, -0.5f}, .TexCoords{1.0f, 0.0f}},
         {.Pos{ 0.5f,  0.5f,  0.5f}, .TexCoords{0.0f, 1.0f}},
         {.Pos{ 0.5f, -0.5f,  0.5f}, .TexCoords{0.0f, 0.0f}},

         {.Pos{-0.5f, -0.5f, -0.5f}, .TexCoords{0.0f, 0.0f}},
         {.Pos{ 0.5f, -0.5f, -0.5f}, .TexCoords{1.0f, 0.0f}},
         {.Pos{ 0.5f, -0.5f,  0.5f}, .TexCoords{1.0f, 1.0f}},
         {.Pos{ 0.5f, -0.5f,  0.5f}, .TexCoords{1.0f, 1.0f}},
         {.Pos{-0.5f, -0.5f,  0.5f}, .TexCoords{0.0f, 1.0f}},
         {.Pos{-0.5f, -0.5f, -0.5f}, .TexCoords{0.0f, 0.0f}},

         {.Pos{-0.5f,  0.5f, -0.5f}, .TexCoords{1.0f, 0.0f}},
         {.Pos{ 0.5f,  0.5f,  0.5f}, .TexCoords{0.0f, 1.0f}},
         {.Pos{ 0.5f,  0.5f, -0.5f}, .TexCoords{0.0f, 0.0f}},
         {.Pos{ 0.5f,  0.5f,  0.5f}, .TexCoords{0.0f, 1.0f}},
         {.Pos{-0.5f,  0.5f, -0.5f}, .TexCoords{1.0f, 0.0f}},
         {.Pos{-0.5f,  0.5f,  0.5f}, .TexCoords{1.0f, 1.0f}},

         // Plane
         {.Pos{ 5.0f, -0.5f,  5.0f}, .TexCoords{2.0f, 0.0f}},
         {.Pos{-5.0f, -0.5f, -5.0f}, .TexCoords{0.0f, 2.0f}},
         {.Pos{-5.0f, -0.5f,  5.0f}, .TexCoords{0.0f, 0.0f}},

         {.Pos{ 5.0f, -0.5f,  5.0f}, .TexCoords{2.0f, 0.0f}},
         {.Pos{ 5.0f, -0.5f, -5.0f}, .TexCoords{2.0f, 2.0f}},
         {.Pos{-5.0f, -0.5f, -5.0f}, .TexCoords{0.0f, 2.0f}},

         // Fullscreen Quad
         {.Pos{-1.0f,  1.0f,  0.0f}, .TexCoords{0.0f, 1.0f}},
         {.Pos{-1.0f, -1.0f,  0.0f}, .TexCoords{0.0f, 0.0f}}, // bottom-left is UV 0,0
         {.Pos{ 1.0f, -1.0f,  0.0f}, .TexCoords{1.0f, 0.0f}},

         {.Pos{-1.0f,  1.0f,  0.0f}, .TexCoords{0.0f, 1.0f}},
         {.Pos{ 1.0f, -1.0f,  0.0f}, .TexCoords{1.0f, 0.0f}},
         {.Pos{ 1.0f,  1.0f,  0.0f}, .TexCoords{1.0f, 1.0f}},

      };

      Pikzel::BufferLayout layout = {
         {"inPos",       Pikzel::DataType::Vec3},
         {"inTexCoords", Pikzel::DataType::Vec2},
      };
      m_VertexBuffer = Pikzel::RenderCore::CreateVertexBuffer(layout, sizeof(vertices), vertices);
   }


   void CreateTextures() {
      m_TextureContainer = Pikzel::RenderCore::CreateTexture({.Path = "Assets/" APP_NAME "/Textures/Container.jpg"});
      m_TextureFloor = Pikzel::RenderCore::CreateTexture({.Path = "Assets/" APP_NAME "/Textures/Floor.png"});
      m_NewSkyboxPath = "Assets/" APP_NAME "/Textures/Skybox.jpg";
   }


   void LoadSkybox(const std::filesystem::path& path) {
      m_Skybox = Pikzel::RenderCore::CreateTexture({.Type = Pikzel::TextureType::TextureCube, .Path = path});
   }


   void CreatePipelines() {
      m_ScenePipeline = GetWindow().GetGraphicsContext().CreatePipeline({
         m_VertexBuffer->GetLayout(),
         {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/TexturedModel.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/TexturedModel.frag.spv" }
         }
      });

      m_SkyboxPipeline = GetWindow().GetGraphicsContext().CreatePipeline({
         m_VertexBuffer->GetLayout(),
         {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/Skybox.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/Skybox.frag.spv" }
         }
      });
   }


private:

   Pikzel::Input m_Input;
   std::optional<std::filesystem::path> m_NewSkyboxPath;


   Camera m_Camera = {
      .Position = {0.0f, 0.0f, 0.0f},
      .Direction = glm::normalize(glm::vec3{0.0f, 0.0f, -1.0f}),
      .UpVector = {0.0f, 1.0f, 0.0f},
      .FoVRadians = glm::radians(60.f),
      .MoveSpeed = 2.5f,
      .RotateSpeed = 10.0f
   };

   std::unique_ptr<Pikzel::VertexBuffer> m_VertexBuffer;
   std::unique_ptr<Pikzel::Texture> m_TextureContainer;
   std::unique_ptr<Pikzel::Texture> m_TextureFloor;
   std::unique_ptr<Pikzel::Texture> m_Skybox;
   std::unique_ptr<Pikzel::Pipeline> m_ScenePipeline;
   std::unique_ptr<Pikzel::Pipeline> m_SkyboxPipeline;
};


std::unique_ptr<Pikzel::Application> CreateApplication(int argc, const char* argv[]) {
   return std::make_unique<Skybox>();
}
