#include "Pikzel/Pikzel.h"
#include "Pikzel/Core/EntryPoint.h"

// For debugging, this can be set to 1 to render directly to window,
// bypassing all of the framebuffer stuff
#define RENDER_DIRECTLY_TO_WINDOW 0

// note: Pikzel uses reverse-Z so near and far planes are swapped
const float nearPlane = 1000.f;
const float farPlane = 0.1f;


class Framebuffers final : public Pikzel::Application {
using super = Pikzel::Application;
public:
   Framebuffers()
   : Pikzel::Application {{.title = APP_DESCRIPTION, .clearColor = Pikzel::sRGB{0.5f, 0.5f, 0.5f}, .isVSync = true}}
   , m_Input {GetWindow()}
   {
      CreateVertexBuffer();
      CreateTextures();
      CreateFramebuffer();
      CreatePipelines();

      m_Camera.projection = glm::perspective(m_Camera.fovRadians, static_cast<float>(GetWindow().GetWidth()) / static_cast<float>(GetWindow().GetHeight()), nearPlane, farPlane);

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

      static int postProcess = 0;
      static float offset = 1.0 / 300.0f;

      glm::mat4 transform = glm::identity<glm::mat4>();
      glm::mat4 view = glm::lookAt(m_Camera.position, m_Camera.position + m_Camera.direction, m_Camera.upVector);

      // Conceptually, there is a "context" for each render target
      // Although in practice, OpenGL only has one "context", and if you try and mix rendering commands between different GraphicsContext instances it wont work
      // You have to do one at a time.

#if RENDER_DIRECTLY_TO_WINDOW
      GetWindow().BeginFrame();
      Pikzel::GraphicsContext& gc = GetWindow().GetGraphicsContext();
#else
      // Render to framebuffer
      Pikzel::GraphicsContext& gc = m_Framebuffer->GetGraphicsContext();
      gc.BeginFrame();  // this will "bind" the frame buffer object.
#endif
      // Everything else is same as if you're rendering to a window graphics context
      gc.Bind(*m_ScenePipeline);

      glm::mat4 model = glm::translate(glm::identity<glm::mat4>(), glm::vec3(-1.0f, 0.0f, -1.0f));
      gc.PushConstant("constants.mvp"_hs, m_Camera.projection * view * model);
      gc.Bind("uTexture"_hs, *m_TextureContainer);
      gc.DrawTriangles(*m_VertexBuffer, 36);

      model = glm::translate(glm::identity<glm::mat4>(), glm::vec3(2.0f, 0.0f, 0.0f));
      gc.PushConstant("constants.mvp"_hs, m_Camera.projection * view * model);
      gc.DrawTriangles(*m_VertexBuffer, 36);

      model = glm::identity<glm::mat4>();
      gc.PushConstant("constants.mvp"_hs, m_Camera.projection * view * model);
      gc.Bind("uTexture"_hs, *m_TextureFloor);
      gc.DrawTriangles(*m_VertexBuffer, 6, 36);

#if RENDER_DIRECTLY_TO_WINDOW
#else
      gc.EndFrame();    // This submits the rendering tasks to the graphics queue, and returns immediately.  Rendering is not necessarily complete.
      gc.SwapBuffers(); // This "swaps buffers" for framebuffer. A framebuffer doesn't have anything to swap, so all this does is waits for rendering to that buffer to complete.
                        // You could swapbuffers here, or you could do other stuff on the CPU here (while GPU is rendering), and then swapbuffers later...

      // Render the framebuffer to the window, e.g. with post-processing effects
      GetWindow().BeginFrame();    // In OpenGL, this will bind the default FBO, and doing that blocks until rendering the other FBO is finished.  In Vulkan, it doesn't block - you need to "swapbuffers" on the framebuffer gc somewhere

      Pikzel::GraphicsContext& wgc = GetWindow().GetGraphicsContext();
      wgc.Bind(*m_PostProcessingPipeline);
      wgc.PushConstant("constants.postprocess"_hs, postProcess);
      wgc.PushConstant("constants.offset"_hs, offset);
      wgc.Bind("uTexture"_hs, m_Framebuffer->GetColorTexture(0));
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
      ImGui::Image(m_Framebuffer->GetImGuiColorTextureId(0), size, ImVec2 {0, 1}, ImVec2 {1, 0});
      ImGui::End();
      GetWindow().EndImGuiFrame();
#endif

      GetWindow().EndFrame();
   }


   virtual void RenderEnd() override {}


   virtual void OnWindowResize(const Pikzel::WindowResizeEvent& event) override {
      super::OnWindowResize(event);
      m_Camera.projection = glm::perspective(m_Camera.fovRadians, static_cast<float>(GetWindow().GetWidth()) / static_cast<float>(GetWindow().GetHeight()), nearPlane, farPlane);

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

      Pikzel::BufferLayout layout = {
         {"inPos",       Pikzel::DataType::Vec3},
         {"inTexCoords", Pikzel::DataType::Vec2},
      };
      m_VertexBuffer = Pikzel::RenderCore::CreateVertexBuffer(layout, sizeof(vertices), vertices);
   }


   void CreateTextures() {
      m_TextureContainer = Pikzel::RenderCore::CreateTexture({.path = "Assets/" APP_NAME "/Textures/Container.jpg"});
      m_TextureFloor = Pikzel::RenderCore::CreateTexture({.path = "Assets/" APP_NAME "/Textures/Floor.png"});
   }


   void CreateFramebuffer() {
      m_Framebuffer = Pikzel::RenderCore::CreateFramebuffer({.width = GetWindow().GetWidth(), .height = GetWindow().GetHeight(), .msaaNumSamples = 4, .clearColorValue = GetWindow().GetClearColor()});
   }


   void CreatePipelines() {
      m_PostProcessingPipeline = GetWindow().GetGraphicsContext().CreatePipeline({
         .shaders = {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/PostProcess.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/PostProcess.frag.spv" }
         },
         .bufferLayout = m_VertexBuffer->GetLayout(),
      });
#if RENDER_DIRECTLY_TO_WINDOW
      m_ScenePipeline = GetWindow().GetGraphicsContext().CreatePipeline({
#else
      m_ScenePipeline = m_Framebuffer->GetGraphicsContext().CreatePipeline({
#endif
         .shaders = {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/TexturedModel.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/TexturedModel.frag.spv" }
         },
         .bufferLayout = m_VertexBuffer->GetLayout(),
      });
   }


private:
   Pikzel::Input m_Input;

   Camera m_Camera = {
      .position = {0.0f, 0.0f, 3.0f},
      .direction = glm::normalize(glm::vec3{0.0f, 0.0f, -1.0f}),
      .upVector = {0.0f, 1.0f, 0.0f},
      .fovRadians = glm::radians(60.f),
      .moveSpeed = 2.5f,
      .rotateSpeed = 10.0f
   };

   std::unique_ptr<Pikzel::VertexBuffer> m_VertexBuffer;
   std::unique_ptr<Pikzel::Texture> m_TextureContainer;
   std::unique_ptr<Pikzel::Texture> m_TextureFloor;
   std::unique_ptr<Pikzel::Framebuffer> m_Framebuffer;
   std::unique_ptr<Pikzel::Pipeline> m_ScenePipeline;
   std::unique_ptr<Pikzel::Pipeline> m_PostProcessingPipeline;

};


std::unique_ptr<Pikzel::Application> CreateApplication(int argc, const char* argv[]) {
   return std::make_unique<Framebuffers>();
}
