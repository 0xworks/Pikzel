#include "Camera.h"
#include "ImGuiEx.h"

#include "Pikzel/Core/Application.h"
#include "Pikzel/Core/Utility.h"
#include "Pikzel/Input/Input.h"
#include "Pikzel/Renderer/Framebuffer.h"
#include "Pikzel/Renderer/RenderCore.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <filesystem>


class Framebuffers final : public Pikzel::Application {
public:
   Framebuffers(int argc, const char* argv[])
   : Pikzel::Application {argc, argv, {.Title = APP_DESCRIPTION, .ClearColor = {0.1f, 0.1f, 0.2f, 1.0f}, .IsVSync = true}}
   , m_bindir {argv[0]}
   , m_Input {GetWindow()}
   {
      m_bindir.remove_filename();

      CreateVertexBuffer();
      CreateTextures();
      CreateFramebuffer();
      CreatePipelines();

      m_Camera.Projection = glm::perspective(m_Camera.FoVRadians, static_cast<float>(GetWindow().GetWidth()) / static_cast<float>(GetWindow().GetHeight()), 0.1f, 1000.0f);

      GetWindow().GetGraphicsContext().InitializeImGui();

      // Optional tweaking of ImGui style
      ImGuiIO& io = ImGui::GetIO();
      ImGui::StyleColorsDark();
      ImGuiStyle& style = ImGui::GetStyle();
      if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
         style.WindowRounding = 0.0f;
         style.Colors[ImGuiCol_WindowBg].w = 1.0f;
      }
      float scaleFactor = GetWindow().ContentScale();
      style.ScaleAllSizes(scaleFactor);

      io.Fonts->AddFontFromFileTTF("Assets/Fonts/OpenSans-Bold.ttf", 15 * scaleFactor);
      io.FontDefault = io.Fonts->AddFontFromFileTTF("Assets/Fonts/OpenSans-Regular.ttf", 15 * scaleFactor);

      // This is required (even if you do not override default font) to upload ImGui font texture to the ImGui backend
      // We cannot do it as part of InitialiseImGui() in case client _does_ want to override default fonts
      Pikzel::RenderCore::UploadImGuiFonts();
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

      static int postProcess = 0;
      static float offset = 1.0 / 300.0f;

      glm::mat4 transform = glm::identity<glm::mat4>();
      glm::mat4 view = glm::lookAt(m_Camera.Position, m_Camera.Position + m_Camera.Direction, m_Camera.UpVector);

      // Conceptually, there is a "context" for each render target
      // Although in practice, OpenGL only has one "context", and if you try and mix rendering commands between different GraphicsContext instances it wont work
      // You have to do one at a time.

#define RENDER_TO_FRAMEBUFFER 1

#if RENDER_TO_FRAMEBUFFER
      // Render to framebuffer
      Pikzel::GraphicsContext& gc = m_Framebuffer->GetGraphicsContext();
      gc.BeginFrame();  // this will "bind" the frame buffer object.
#else
      GetWindow().BeginFrame();
      Pikzel::GraphicsContext& gc = GetWindow().GetGraphicsContext();
#endif
      // Everything else is same as if you're rendering to a window graphics context
      gc.Bind(*m_ScenePipeline);

      gc.Bind(*m_TextureContainer, "uTexture"_hs);
      glm::mat4 model = glm::translate(glm::identity<glm::mat4>(), glm::vec3(-1.0f, 0.0f, -1.0f));
      gc.PushConstant("constants.mvp"_hs, m_Camera.Projection * view * model);
      gc.DrawTriangles(*m_VertexBuffer, 36);

      model = glm::translate(glm::identity<glm::mat4>(), glm::vec3(2.0f, 0.0f, 0.0f));
      gc.PushConstant("constants.mvp"_hs, m_Camera.Projection * view * model);
      gc.DrawTriangles(*m_VertexBuffer, 36);

      gc.Bind(*m_TextureFloor, "uTexture"_hs);
      model = glm::identity<glm::mat4>();
      gc.PushConstant("constants.mvp"_hs, m_Camera.Projection * view * model);
      gc.DrawTriangles(*m_VertexBuffer, 6, 36);

#if RENDER_TO_FRAMEBUFFER
      gc.EndFrame();    // This submits the rendering tasks to the graphics queue, and returns immediately.  Rendering is not necessarily complete.
      gc.SwapBuffers(); // This "swaps buffers" for framebuffer. A framebuffer doesn't have anything to swap, so all this does is waits for rendering to that buffer to complete.
                        // You could swapbuffers here, or you could do other stuff on the CPU here (while GPU is rendering), and then swapbuffers later...

      // Render the framebuffer to the window, e.g. with post-processing effects
      GetWindow().BeginFrame();    // In OpenGL, this will bind the default FBO, and doing that blocks until rendering the other FBO is finished.  In Vulkan, it doesn't block - you need to "swapbuffers" on the framebuffer gc somewhere

      Pikzel::GraphicsContext& wgc = GetWindow().GetGraphicsContext();
      wgc.Bind(*m_PostProcessingPipeline);

      wgc.Bind(m_Framebuffer->GetColorTexture(), "uTexture"_hs);
      wgc.PushConstant("constants.postprocess"_hs, postProcess);
      wgc.PushConstant("constants.offset"_hs, offset);
      wgc.DrawTriangles(*m_VertexBuffer, 6, 42);

      // Can also render the framebuffer content into ImGui window
      GetWindow().BeginImGuiFrame();
      ImGui::Begin("Framebuffer");
      ImGui::Text("Post Processing:");
      ImGui::RadioButton("None", &postProcess, 0);
      ImGui::RadioButton("Invert Colors", &postProcess, 1);
      ImGui::RadioButton("Greyscale", &postProcess, 2);
      ImGui::RadioButton("Sharpen", &postProcess, 3);
      ImGui::RadioButton("Blur", &postProcess, 4);
      ImGui::Text("Original framebuffer:");
      ImVec2 size = ImGui::GetContentRegionAvail();
      ImGui::Image(m_Framebuffer->GetImGuiTextureId(), size, ImVec2 {0, 1}, ImVec2 {1, 0});
      ImGui::End();
      GetWindow().EndImGuiFrame();
#endif

      GetWindow().EndFrame();
   }


   virtual void RenderEnd() override {}


   virtual void OnWindowResize(const Pikzel::WindowResizeEvent& event) override {
      __super::OnWindowResize(event);

      // recreate framebuffer with new size
      CreateFramebuffer();
   }


private:

   struct Vertex {
      glm::vec3 Pos;
      glm::vec2 TexCoord;
   };

   void CreateVertexBuffer() {
      Vertex vertices[] = {
         // Cube
         {.Pos{-0.5f, -0.5f, -0.5f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{ 0.5f,  0.5f, -0.5f}, .TexCoord{0.0f, 1.0f}},
         {.Pos{ 0.5f, -0.5f, -0.5f}, .TexCoord{0.0f, 0.0f}},
         {.Pos{ 0.5f,  0.5f, -0.5f}, .TexCoord{0.0f, 1.0f}},
         {.Pos{-0.5f, -0.5f, -0.5f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{-0.5f,  0.5f, -0.5f}, .TexCoord{1.0f, 1.0f}},

         {.Pos{-0.5f, -0.5f,  0.5f}, .TexCoord{0.0f, 0.0f}},
         {.Pos{ 0.5f, -0.5f,  0.5f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{ 0.5f,  0.5f,  0.5f}, .TexCoord{1.0f, 1.0f}},
         {.Pos{ 0.5f,  0.5f,  0.5f}, .TexCoord{1.0f, 1.0f}},
         {.Pos{-0.5f,  0.5f,  0.5f}, .TexCoord{0.0f, 1.0f}},
         {.Pos{-0.5f, -0.5f,  0.5f}, .TexCoord{0.0f, 0.0f}},

         {.Pos{-0.5f,  0.5f,  0.5f}, .TexCoord{1.0f, 1.0f}},
         {.Pos{-0.5f,  0.5f, -0.5f}, .TexCoord{0.0f, 1.0f}},
         {.Pos{-0.5f, -0.5f, -0.5f}, .TexCoord{0.0f, 0.0f}},
         {.Pos{-0.5f, -0.5f, -0.5f}, .TexCoord{0.0f, 0.0f}},
         {.Pos{-0.5f, -0.5f,  0.5f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{-0.5f,  0.5f,  0.5f}, .TexCoord{1.0f, 1.0f}},

         {.Pos{ 0.5f,  0.5f,  0.5f}, .TexCoord{0.0f, 1.0f}},
         {.Pos{ 0.5f, -0.5f, -0.5f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{ 0.5f,  0.5f, -0.5f}, .TexCoord{1.0f, 1.0f}},
         {.Pos{ 0.5f, -0.5f, -0.5f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{ 0.5f,  0.5f,  0.5f}, .TexCoord{0.0f, 1.0f}},
         {.Pos{ 0.5f, -0.5f,  0.5f}, .TexCoord{0.0f, 0.0f}},

         {.Pos{-0.5f, -0.5f, -0.5f}, .TexCoord{0.0f, 0.0f}},
         {.Pos{ 0.5f, -0.5f, -0.5f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{ 0.5f, -0.5f,  0.5f}, .TexCoord{1.0f, 1.0f}},
         {.Pos{ 0.5f, -0.5f,  0.5f}, .TexCoord{1.0f, 1.0f}},
         {.Pos{-0.5f, -0.5f,  0.5f}, .TexCoord{0.0f, 1.0f}},
         {.Pos{-0.5f, -0.5f, -0.5f}, .TexCoord{0.0f, 0.0f}},

         {.Pos{-0.5f,  0.5f, -0.5f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{ 0.5f,  0.5f,  0.5f}, .TexCoord{0.0f, 1.0f}},
         {.Pos{ 0.5f,  0.5f, -0.5f}, .TexCoord{0.0f, 0.0f}},
         {.Pos{ 0.5f,  0.5f,  0.5f}, .TexCoord{0.0f, 1.0f}},
         {.Pos{-0.5f,  0.5f, -0.5f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{-0.5f,  0.5f,  0.5f}, .TexCoord{1.0f, 1.0f}},

         // Plane
         {.Pos{ 5.0f, -0.5f,  5.0f}, .TexCoord{2.0f, 0.0f}},
         {.Pos{-5.0f, -0.5f, -5.0f}, .TexCoord{0.0f, 2.0f}},
         {.Pos{-5.0f, -0.5f,  5.0f}, .TexCoord{0.0f, 0.0f}},

         {.Pos{ 5.0f, -0.5f,  5.0f}, .TexCoord{2.0f, 0.0f}},
         {.Pos{ 5.0f, -0.5f, -5.0f}, .TexCoord{2.0f, 2.0f}},
         {.Pos{-5.0f, -0.5f, -5.0f}, .TexCoord{0.0f, 2.0f}},

         // Fullscreen Quad
         {.Pos{-1.0f,  1.0f,  0.0f}, .TexCoord{0.0f, 1.0f}},
         {.Pos{-1.0f, -1.0f,  0.0f}, .TexCoord{0.0f, 0.0f}}, // bottom-left is UV 0,0
         {.Pos{ 1.0f, -1.0f,  0.0f}, .TexCoord{1.0f, 0.0f}},

         {.Pos{-1.0f,  1.0f,  0.0f}, .TexCoord{0.0f, 1.0f}},
         {.Pos{ 1.0f, -1.0f,  0.0f}, .TexCoord{1.0f, 0.0f}},
         {.Pos{ 1.0f,  1.0f,  0.0f}, .TexCoord{1.0f, 1.0f}},
      };

      m_VertexBuffer = Pikzel::RenderCore::CreateVertexBuffer(sizeof(vertices), vertices);
      m_VertexBuffer->SetLayout({
         { "inPos",      Pikzel::DataType::Vec3 },
         { "inTexCoord", Pikzel::DataType::Vec2 }
      });
   }


   void CreateTextures() {
      m_TextureContainer = Pikzel::RenderCore::CreateTexture2D(m_bindir / "Assets/Textures/Container.jpg");
      m_TextureFloor = Pikzel::RenderCore::CreateTexture2D(m_bindir / "Assets/Textures/Floor.png");
   }


   void CreateFramebuffer() {
      m_Framebuffer = GetWindow().GetGraphicsContext().CreateFramebuffer({GetWindow().GetWidth(), GetWindow().GetHeight(), GetWindow().GetClearColor()});
   }


   void CreatePipelines() {
      m_PostProcessingPipeline = GetWindow().GetGraphicsContext().CreatePipeline({
         m_VertexBuffer->GetLayout(),
         {
            { Pikzel::ShaderType::Vertex, m_bindir / "Assets/Shaders/PostProcess.vert.spv" },
            { Pikzel::ShaderType::Fragment, m_bindir / "Assets/Shaders/PostProcess.frag.spv" }
         }
      });
      m_ScenePipeline = GetWindow().GetGraphicsContext().CreatePipeline({
         m_VertexBuffer->GetLayout(),
         {
            { Pikzel::ShaderType::Vertex, m_bindir / "Assets/Shaders/TexturedModel.vert.spv" },
            { Pikzel::ShaderType::Fragment, m_bindir / "Assets/Shaders/TexturedModel.frag.spv" }
         }
      });
   }


private:

   Pikzel::Input m_Input;
   std::filesystem::path m_bindir;

   Camera m_Camera = {
      .Position = {0.0f, 0.0f, 3.0f},
      .Direction = glm::normalize(glm::vec3{0.0f, 0.0f, -1.0f}),
      .UpVector = {0.0f, 1.0f, 0.0f},
      .FoVRadians = glm::radians(60.f),
      .MoveSpeed = 2.5f,
      .RotateSpeed = 10.0f
   };

   std::unique_ptr<Pikzel::VertexBuffer> m_VertexBuffer;
   std::unique_ptr<Pikzel::Texture2D> m_TextureContainer;
   std::unique_ptr<Pikzel::Texture2D> m_TextureFloor;
   std::unique_ptr<Pikzel::Framebuffer> m_Framebuffer;
   std::unique_ptr<Pikzel::Pipeline> m_ScenePipeline;
   std::unique_ptr<Pikzel::Pipeline> m_PostProcessingPipeline;

};


std::unique_ptr<Pikzel::Application> Pikzel::CreateApplication(int argc, const char* argv[]) {
   PKZL_LOG_INFO(APP_DESCRIPTION);
   PKZL_LOG_INFO("Linked against {0} {1}", PKZL_DESCRIPTION, PKZL_VERSION);
#ifdef PKZL_DEBUG
   PKZL_LOG_INFO("DEBUG build");
#endif

   return std::make_unique<Framebuffers>(argc, argv);
}