#include "Pikzel/Pikzel.h"
#include "Pikzel/Core/EntryPoint.h"

// For debugging, this can be set to 1 to render directly to window,
// bypassing all of the framebuffer stuff
#define RENDER_DIRECTLY_TO_WINDOW 0

class SRGBDemo final : public Pikzel::Application {
public:
   SRGBDemo()
   : Pikzel::Application {{.Title = APP_DESCRIPTION, .ClearColor = Pikzel::sRGB{1.0f, 1.0f, 1.0f}}}
   , m_Input {GetWindow()}
   {
      CreateVertexBuffer();
      CreateFramebuffer();
      CreatePipelines();

      Pikzel::ImGuiEx::Init(GetWindow());
   }


   virtual void Update(const Pikzel::DeltaTime deltaTime) override {
      float dx = m_Input.GetAxis("X"_hs) * deltaTime.count();
      float dy = m_Input.GetAxis("Z"_hs) * deltaTime.count();
      m_View = glm::translate(m_View, {dx, dy, 0.0f});
   }


   virtual void RenderBegin() override {}


   virtual void Render() override {

      static glm::vec4 greenOpaque = Pikzel::sRGB{0.0f, 1.0f, 0.0f};
      static glm::vec4 redOpaque = Pikzel::sRGB{1.0f, 0.0f, 0.0f};

      static glm::vec4 pickedColor = {1.0f, 0.0f, 0.0f, 0.5f};


#if RENDER_DIRECTLY_TO_WINDOW
      GetWindow().BeginFrame();
      Pikzel::GraphicsContext& gc = GetWindow().GetGraphicsContext();
#else
      // Render to framebuffer 
      Pikzel::GraphicsContext& gc = m_Framebuffer->GetGraphicsContext();
      gc.BeginFrame();  // this will "bind" the frame buffer object.
#endif
      {
         // Draw a vertical green opaque bar
         // Draw two horizonal bars:
         //     one is red opaque
         //     one is red and 50% transparent  (you can change this one with ImGui color picker)
         //
         // If Pikzel is working in a "gamma correct" way, then the perceived brightness of the 
         // transparent bar should be consistent across the screen.
         // In particular, it should not appear darker where it crosses over the green vertical bar.
         // Refer: https://blog.johnnovak.net/2016/09/21/what-every-coder-should-know-about-gamma/

         glm::mat4 model = glm::scale(glm::translate(glm::identity<glm::mat4>(), {0.0f, 0.0f, 0.0f}), {2.0f, 8.0f, 1.0f});

         gc.Bind(*m_ScenePipeline);
         gc.PushConstant("constants.mvp"_hs, m_Projection * m_View * model);
         gc.PushConstant("constants.color"_hs, greenOpaque);
         gc.DrawTriangles(*m_VertexBuffer, 6);

         model = glm::scale(glm::translate(glm::identity<glm::mat4>(), {0.0f, 0.0f, 0.0f}), {14.0f, 0.5f, 1.0f});
         gc.PushConstant("constants.mvp"_hs, m_Projection * m_View * model);
         gc.PushConstant("constants.color"_hs, redOpaque);
         gc.DrawTriangles(*m_VertexBuffer, 6);

         model = glm::scale(glm::translate(glm::identity<glm::mat4>(), {0.0f, -1.0f, 0.0f}), {14.0f, 0.5f, 1.0f});
         gc.PushConstant("constants.mvp"_hs, m_Projection * m_View * model);
         gc.PushConstant("constants.color"_hs, static_cast<glm::vec4>(Pikzel::sRGBA {pickedColor.r, pickedColor.g, pickedColor.b, pickedColor.a}));
         gc.DrawTriangles(*m_VertexBuffer, 6);
      }

      {
         // Draw 21 grey squares, ranging from fully dark (0.0) to fully bright (1.0), in increments of 0.05
         // You should end up with an even looking spread of colors from black on the left to white on the right.
         // Mid-grey should be in the middle (a shade that most graphics programs will produce if you tell them
         // the color has R=128, B=128, and G=128)
         for (int i = 0; i <= 20; ++i) {
            glm::mat4 model = glm::scale(glm::translate(glm::identity<glm::mat4>(), {-14.0f + 1.4f * i, -8.0f, 0.0f}), {0.7f, 0.7f, 1.0f});
            gc.PushConstant("constants.mvp"_hs, m_Projection * m_View * model);
            float val = 0.05f * i;
            glm::vec4 color = Pikzel::sRGB(val, val, val);
            gc.PushConstant("constants.color"_hs, color);
            gc.DrawTriangles(*m_VertexBuffer, 6);
         }
      }

#if RENDER_DIRECTLY_TO_WINDOW
#else
      gc.EndFrame();
      gc.SwapBuffers();

      GetWindow().BeginFrame();
      Pikzel::GraphicsContext& wgc = GetWindow().GetGraphicsContext();
      wgc.Bind(*m_PostProcessingPipeline);
      wgc.Bind(m_Framebuffer->GetColorTexture(0), "uTexture"_hs);
      wgc.DrawTriangles(*m_VertexBuffer, 6);

      GetWindow().BeginImGuiFrame();
      ImGui::Begin("Framebuffer");
      float rgba[4] = {pickedColor.r, pickedColor.g, pickedColor.b, pickedColor.a};
      ImGui::ColorEdit4("Color:", rgba);
      pickedColor = {rgba[0], rgba[1], rgba[2], rgba[3]};
      ImVec2 size = ImGui::GetContentRegionAvail();

      // Note: when displayed in the ImGui image, the partly transparent red bar will look darker than it should.
      // This is because the final texture referenced by GetImGuiColorTextureId is has an alpha component,
      // and that partly transparent red bar has alpha 0.75.
      // So the black background of the ImGui window will show through a little bit, hence darkening the color.
      ImGui::Image(m_Framebuffer->GetImGuiColorTextureId(0), size, ImVec2 {0, 1}, ImVec2 {1, 0});
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
      glm::vec2 TexCoords;
   };

   void CreateVertexBuffer() {
      Vertex vertices[] = {
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


   void CreateFramebuffer() {
      m_Framebuffer = Pikzel::RenderCore::CreateFramebuffer({.Width = GetWindow().GetWidth(), .Height = GetWindow().GetHeight(), .MSAANumSamples = 4, .ClearColorValue = GetWindow().GetClearColor()});
   }


   void CreatePipelines() {
      m_PostProcessingPipeline = GetWindow().GetGraphicsContext().CreatePipeline({
         .Shaders = {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/PostProcess.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/PostProcess.frag.spv" }
         },
         .BufferLayout = m_VertexBuffer->GetLayout()
      });

#if RENDER_DIRECTLY_TO_WINDOW
      m_ScenePipeline = GetWindow().GetGraphicsContext().CreatePipeline({
#else
      m_ScenePipeline = m_Framebuffer->GetGraphicsContext().CreatePipeline({
#endif
         .Shaders = {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/sRGB.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/sRGB.frag.spv" }
         },
         .BufferLayout = m_VertexBuffer->GetLayout(),
      });
   }


private:
   Pikzel::Input m_Input;
   glm::mat4 m_View = glm::identity<glm::mat4>();
   glm::mat4 m_Projection = glm::ortho(-16.0f, 16.0f, -9.0f, 9.0f);
   std::shared_ptr<Pikzel::VertexBuffer> m_VertexBuffer;
   std::unique_ptr<Pikzel::Framebuffer> m_Framebuffer;
   std::unique_ptr<Pikzel::Pipeline> m_ScenePipeline;
   std::unique_ptr<Pikzel::Pipeline> m_PostProcessingPipeline;
};


std::unique_ptr<Pikzel::Application> CreateApplication(int argc, const char* argv[]) {
   return std::make_unique<SRGBDemo>();
}
