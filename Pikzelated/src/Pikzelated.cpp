#include "Pikzel/Pikzel.h"
#include "Pikzel/Core/EntryPoint.h"

#include <imgui_internal.h>

// TODO: get these from somewhere
const float nearPlane = 1000.f;
const float farPlane = 0.1f;

class Pikzelated final : public Pikzel::Application {
   using super = Pikzel::Application;
public:
   Pikzelated()
   : Pikzel::Application{{.title = APP_DESCRIPTION, .clearColor = {0.0f, 0.0f, 0.0f, 1.0f}, .isVSync = true}}
   , m_Input{GetWindow()}
   {
      PKZL_PROFILE_FUNCTION();

      Pikzel::ImGuiEx::Init(GetWindow());
      ImGuiIO& io = ImGui::GetIO();
      ImGui::LoadIniSettingsFromDisk(io.IniFilename);
      if (!ImGui::GetCurrentContext()->SettingsLoaded) {
         ImGui::LoadIniSettingsFromDisk("EditorImGui.ini");
      }

      m_Framebuffer = Pikzel::RenderCore::CreateFramebuffer({.width = m_ViewportSize.x, .height = m_ViewportSize.y, .msaaNumSamples = 4, .clearColorValue = {1.0f, 1.0f, 1.0f, 1.0f}});
      m_SceneRenderer = Pikzel::CreateSceneRenderer(m_Framebuffer->GetGraphicsContext());

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


   void OnFileNew() {
      m_Scene = std::make_unique<Pikzel::Scene>();

      Vertex vertices[] = {
         {.Pos{-0.5f, -0.5f, 0.0f}, .Color{Pikzel::sRGB{1.0f, 0.0f, 0.0f}}},
         {.Pos{ 0.5f, -0.5f, 0.0f}, .Color{Pikzel::sRGB{0.0f, 1.0f, 0.0f}}},
         {.Pos{ 0.0f,  0.5f, 0.0f}, .Color{Pikzel::sRGB{0.0f, 0.0f, 1.0f}}}
      };

      Pikzel::BufferLayout layout{
         {"inPos",   Pikzel::DataType::Vec3},
         {"inColor", Pikzel::DataType::Vec3}
      };

      std::vector<uint32_t> indices = {0, 1, 2};

      Pikzel::Entity triangle = m_Scene->CreateEntity();
      m_Scene->AddComponent<Pikzel::Transform>(triangle, glm::identity<glm::mat4>());
      m_Scene->AddComponent<Pikzel::Mesh>(
         triangle,
         Pikzel::RenderCore::CreateVertexBuffer(layout, sizeof(vertices), vertices),
         Pikzel::RenderCore::CreateIndexBuffer(indices.size(), indices.data())
      );
   }


   void OnFileOpen() {}


   void OnFileSave() {}


   void OnFileSaveAs() {}


   void OnFileExit() {
      Exit();
   }


   virtual void RenderBegin() override {
      PKZL_PROFILE_FUNCTION();
      if(
         (m_ViewportSize.x != m_Framebuffer->GetWidth()) ||
         (m_ViewportSize.y != m_Framebuffer->GetHeight())
      ) {
         m_Framebuffer = Pikzel::RenderCore::CreateFramebuffer({.width = m_ViewportSize.x, .height = m_ViewportSize.y, .msaaNumSamples = 4, .clearColorValue = {1.0f, 1.0f, 1.0f, 1.0f}});
         m_Camera.projection = glm::perspective(m_Camera.fovRadians, static_cast<float>(m_ViewportSize.x) / static_cast<float>(m_ViewportSize.y), nearPlane, farPlane);
      }
      m_SceneRenderer->RenderBegin();
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

      Pikzel::GraphicsContext& gc = m_Framebuffer->GetGraphicsContext();
      {
         PKZL_PROFILE_SCOPE("render scene");
         if (m_Scene) {
            m_SceneRenderer->Render(gc, m_Camera, *m_Scene);
         }
      }

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

         if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
               if (ImGui::MenuItem("New...", "Ctrl+N")) {
                  OnFileNew();
               }
               if (ImGui::MenuItem("Open...", "Ctrl+O")) {
                  OnFileOpen();
               }
               if (ImGui::MenuItem("Save", "Ctrl+S")) {
                  OnFileSave();
               }
               if (ImGui::MenuItem("Save As...")) {
                  OnFileSaveAs();
               }
               if (ImGui::MenuItem("Exit", "Alt+F4")) {
                  Exit();
               }
               ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
         }

         {
            ImGui::Begin("Statistics");

            // auto stats = Renderer2D::GetStats();
            ImGui::Text("Draw Calls: XXX"); // % d", stats.DrawCalls);
            ImGui::Text("Quads: XXX");      // % d", stats.QuadCount);
            ImGui::Text("Vertices: XXX");   // % d", stats.GetTotalVertexCount());
            ImGui::Text("Indices: XXX");    // % d", stats.GetTotalIndexCount());
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            static float frameRates[90] = {};
            static int frameOffset = 0;
            static double refresh = ImGui::GetTime();
            while (refresh <= ImGui::GetTime()) {
               frameRates[frameOffset] = 1000.0f / io.Framerate;
               frameOffset = (frameOffset + 1) % IM_ARRAYSIZE(frameRates);
               refresh += 1.0f / 60.0f;
            }

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
      PKZL_PROFILE_FUNCTION();
      m_SceneRenderer->RenderEnd();
   }


private:

   // TODO: get rid of this in favor of Pikzel::Mesh::Vertex
   struct Vertex {
      glm::vec3 Pos;
      glm::vec3 Color;
   };


private:
   Camera m_Camera = {
      .position = {0.0f, 0.0f, 3.0f},
      .direction = glm::normalize(glm::vec3{0.0f, 0.0f, -1.0f}),
      .upVector = {0.0f, 1.0f, 0.0f},
      .fovRadians = glm::radians(60.f),
      .moveSpeed = 2.5f,
      .rotateSpeed = 10.0f
   };

   Pikzel::Input m_Input;
   glm::u32vec2 m_ViewportSize = {800, 600};
   std::unique_ptr<Pikzel::Scene> m_Scene;
   std::unique_ptr<Pikzel::SceneRenderer> m_SceneRenderer;
   std::unique_ptr<Pikzel::Framebuffer> m_Framebuffer;

};


std::unique_ptr<Pikzel::Application> CreateApplication(int argc, const char* argv[]) {
   return std::make_unique<Pikzelated>();
}
