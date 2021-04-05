#include "Pikzel/Pikzel.h"
#include "Pikzel/Core/EntryPoint.h"

#include <imgui_internal.h>

// TODO: get these from the scene
const float nearPlane = 1000.f;
const float farPlane = 0.1f;

class Pikzelated final : public Pikzel::Application {
using super = Pikzel::Application; 
public:
   Pikzelated()
   : Pikzel::Application {{.title = APP_DESCRIPTION, .clearColor = {0.0f, 0.0f, 0.0f, 1.0f}, .isVSync = true}}
   , m_Input{ GetWindow() }
   {
      PKZL_PROFILE_FUNCTION();

      Pikzel::ImGuiEx::Init(GetWindow());
      ImGuiIO& io = ImGui::GetIO();
      ImGui::LoadIniSettingsFromDisk(io.IniFilename);
      if (!ImGui::GetCurrentContext()->SettingsLoaded) {
         ImGui::LoadIniSettingsFromDisk("EditorImGui.ini");
      }

      CreateFramebuffer();
      CreateVertexBuffer();
      CreatePipelines();

      m_Camera.projection = glm::perspective(m_Camera.fovRadians, static_cast<float>(m_ViewportSize.x) / static_cast<float>(m_ViewportSize.y), nearPlane, farPlane);

   }


protected:

   virtual void Update(Pikzel::DeltaTime deltaTime) override {
      PKZL_PROFILE_FUNCTION();
      if (m_Input.IsKeyPressed(Pikzel::KeyCode::Escape)) {
         Exit();
      }
      m_Camera.Update(m_Input, deltaTime);
   }


   virtual void RenderBegin() override {
      PKZL_PROFILE_FUNCTION();
      if(
         (m_ViewportSize.x != m_Framebuffer->GetWidth()) ||
         (m_ViewportSize.y != m_Framebuffer->GetHeight())
      ) {
         CreateFramebuffer();
         m_Camera.projection = glm::perspective(m_Camera.fovRadians, static_cast<float>(m_ViewportSize.x) / static_cast<float>(m_ViewportSize.y), nearPlane, farPlane);
      }
   }


   virtual void Render() override {
      static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
      static ImGuiWindowFlags dockspace_window_flags =
         ImGuiWindowFlags_MenuBar |
         ImGuiWindowFlags_NoDocking |
         ImGuiWindowFlags_NoTitleBar |
         ImGuiWindowFlags_NoCollapse |
         ImGuiWindowFlags_NoResize |
         ImGuiWindowFlags_NoMove |
         ImGuiWindowFlags_NoBringToFrontOnFocus |
         ImGuiWindowFlags_NoNavFocus
      ;

      static ImGuiWindowFlags viewport_window_flags = ImGuiWindowFlags_None;
      //         ImGuiWindowFlags_NoDocking |
      //         ImGuiWindowFlags_NoMove |
      //         ImGuiWindowFlags_NoResize
      //      ;

      PKZL_PROFILE_FUNCTION();

      glm::mat4 view = glm::lookAt(m_Camera.position, m_Camera.position + m_Camera.direction, m_Camera.upVector);

      Pikzel::GraphicsContext& gc = m_Framebuffer->GetGraphicsContext();
      gc.BeginFrame();
      {
         gc.Bind(*m_ScenePipeline);
         gc.PushConstant("constants.mvp"_hs, m_Camera.projection * view);
         gc.DrawTriangles(*m_VertexBuffer, 3);
      }
      gc.EndFrame();

      GetWindow().BeginFrame();
      GetWindow().BeginImGuiFrame();

      ImGuiIO& io = ImGui::GetIO();
      ImGuiViewport* viewport = ImGui::GetMainViewport();
      ImGui::SetNextWindowPos(viewport->Pos);
      ImGui::SetNextWindowSize(viewport->Size);
      ImGui::SetNextWindowViewport(viewport->ID);

      ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

      {
         ImGui::Begin("Pikzelated", nullptr, dockspace_window_flags);
         if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
            ImGuiID dockspace_id = ImGui::GetID("DockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
         }

         if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
               if (ImGui::MenuItem("Exit")) {
                  Exit();
               }
               ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
         }

         {
            ImGui::Begin("Statistics");

            // auto stats = Renderer2D::GetStats();
            ImGui::Text("Draw Calls: XXX"); // % d", stats.DrawCalls);
            ImGui::Text("Quads: XXX");      // % d", stats.QuadCount);
            ImGui::Text("Vertices: XXX");   // % d", stats.GetTotalVertexCount());
            ImGui::Text("Indices: XXX");    // % d", stats.GetTotalIndexCount());
            ImGui::End();
         }

         {
            ImGui::Begin("Viewport", nullptr, viewport_window_flags);

            // m_ViewportFocused = ImGui::IsWindowFocused();
            // m_ViewportHovered = ImGui::IsWindowHovered();
            // Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportFocused || !m_ViewportHovered);

            ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
            m_ViewportSize = {viewportPanelSize.x, viewportPanelSize.y};

            gc.SwapBuffers();
            ImGui::Image(m_Framebuffer->GetImGuiColorTextureId(0), viewportPanelSize, ImVec2 {0, 1}, ImVec2 {1, 0});
            ImGui::End();
         }

         ImGui::End();
      }
      ImGui::PopStyleVar(3);
      io.DisplaySize = ImVec2((float)m_ViewportSize.x, (float)m_ViewportSize.y);

      GetWindow().EndImGuiFrame();
      GetWindow().EndFrame();
   }


   virtual void RenderEnd() override {
      ;
   }


private:

   struct Vertex {
      glm::vec3 Pos;
      glm::vec3 Color;
   };


   void CreateFramebuffer() {
      m_Framebuffer = Pikzel::RenderCore::CreateFramebuffer({ .width = m_ViewportSize.x, .height = m_ViewportSize.y, .msaaNumSamples = 4, .clearColorValue = {1.0f, 1.0f, 1.0f, 1.0f} });
   }


   void CreateVertexBuffer() {
      Vertex vertices[] = {
         {.Pos{-0.5f, -0.5f, 0.0f}, .Color{Pikzel::sRGB{1.0f, 0.0f, 0.0f}}},
         {.Pos{ 0.5f, -0.5f, 0.0f}, .Color{Pikzel::sRGB{0.0f, 1.0f, 0.0f}}},
         {.Pos{ 0.0f,  0.5f, 0.0f}, .Color{Pikzel::sRGB{0.0f, 0.0f, 1.0f}}}
      };

      Pikzel::BufferLayout layout{
         {"inPos",   Pikzel::DataType::Vec3},
         {"inColor", Pikzel::DataType::Vec3}
      };
      m_VertexBuffer = Pikzel::RenderCore::CreateVertexBuffer(layout, sizeof(vertices), vertices);
   }


   void CreatePipelines() {
      m_ScenePipeline = m_Framebuffer->GetGraphicsContext().CreatePipeline({
         .shaders = {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/Triangle.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/Triangle.frag.spv" }
         },
         .bufferLayout = m_VertexBuffer->GetLayout()
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

   std::shared_ptr<Pikzel::VertexBuffer> m_VertexBuffer;
   std::unique_ptr<Pikzel::Framebuffer> m_Framebuffer;
   std::unique_ptr<Pikzel::Pipeline> m_ScenePipeline;
   glm::u32vec2 m_ViewportSize = {800, 600};

};


std::unique_ptr<Pikzel::Application> CreateApplication(int argc, const char* argv[]) {
   return std::make_unique<Pikzelated>();
}
