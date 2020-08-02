#include "Pikzel/Core/Application.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>

class Pikzelated : public Pikzel::Application {
public:
   Pikzelated() {
   }


   ~Pikzelated() {
      PKZL_PROFILE_FUNCTION();
      ImGui_ImplOpenGL3_Shutdown();
      ImGui_ImplGlfw_Shutdown();
      ImGui::DestroyContext();
   }


   virtual void Init() override {
      PKZL_PROFILE_FUNCTION();
      Application::Init();

      IMGUI_CHECKVERSION();
      ImGui::CreateContext();
      ImGuiIO& io = ImGui::GetIO();
      io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
      io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
      io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

      ImGui::StyleColorsDark();

      // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
      ImGuiStyle& style = ImGui::GetStyle();
      if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
         style.WindowRounding = 0.0f;
         style.Colors[ImGuiCol_WindowBg].w = 1.0f;
      }

      glfwMakeContextCurrent(m_Window);
      int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
      CORE_ASSERT(status, "Failed to initialize Glad!");

      ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
      ImGui_ImplOpenGL3_Init();

      ImGui::LoadIniSettingsFromDisk(io.IniFilename);
      if (!ImGui::GetCurrentContext()->SettingsLoaded) {
         ImGui::LoadIniSettingsFromDisk("EditorImGui.ini");
      }
   }


   virtual void Update(double deltaTime) override {
      // TODO
   }


   virtual void Render() override {
      static bool dockspaceOpen = true;
      static bool opt_fullscreen_persistant = true;
      static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

      PKZL_PROFILE_FUNCTION();

      bool opt_fullscreen = opt_fullscreen_persistant;

      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();

      // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
      // because it would be confusing to have two docking targets within each others.
      ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
      if (opt_fullscreen) {
         ImGuiViewport* viewport = ImGui::GetMainViewport();
         ImGui::SetNextWindowPos(viewport->Pos);
         ImGui::SetNextWindowSize(viewport->Size);
         ImGui::SetNextWindowViewport(viewport->ID);
         ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
         ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
         window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
         window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
      }

      // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
      if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) {
         window_flags |= ImGuiWindowFlags_NoBackground;
      }

      // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
      // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive, 
      // all active windows docked into it will lose their parent and become undocked.
      // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise 
      // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
      ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
      ImGui::PopStyleVar();

      if (opt_fullscreen) {
         ImGui::PopStyleVar(2);
      }

      // DockSpace
      ImGuiIO& io = ImGui::GetIO();
      if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
         ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
         ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
      }

      if (ImGui::BeginMenuBar()) {
         if (ImGui::BeginMenu("File")) {
            // Disabling fullscreen would allow the window to be moved to the front of other windows, 
            // which we can't undo at the moment without finer window depth/z control.
            //ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen_persistant);

            if (ImGui::MenuItem("Exit")) {
               m_Running = false;
            }
            ImGui::EndMenu();
         }

         ImGui::EndMenuBar();
      }

      ImGui::Begin("Statistics");

//      auto stats = Renderer2D::GetStats();
      ImGui::Text("Draw Calls: XXX"); // % d", stats.DrawCalls);
      ImGui::Text("Quads: XXX");      // % d", stats.QuadCount);
      ImGui::Text("Vertices: XXX");   // % d", stats.GetTotalVertexCount());
      ImGui::Text("Indices: XXX");    // % d", stats.GetTotalIndexCount());

      ImGui::End();

      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2 {0, 0});

      ImGui::Begin("Viewport");

//      m_ViewportFocused = ImGui::IsWindowFocused();
//      m_ViewportHovered = ImGui::IsWindowHovered();
//      Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportFocused || !m_ViewportHovered);

      ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
//      m_ViewportSize = {viewportPanelSize.x, viewportPanelSize.y};

//      uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
//      ImGui::Image((void*)textureID, ImVec2 {m_ViewportSize.x, m_ViewportSize.y}, ImVec2 {0, 1}, ImVec2 {1, 0});
      ImGui::End();
      ImGui::PopStyleVar();

      ImGui::End();
 
      io.DisplaySize = ImVec2(800.0f, 600.0f);

      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

      if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
         GLFWwindow* backup_current_context = glfwGetCurrentContext();
         ImGui::UpdatePlatformWindows();
         ImGui::RenderPlatformWindowsDefault();
         glfwMakeContextCurrent(backup_current_context);
      }
   }

};


std::unique_ptr<Pikzelated::Application> CreateApplication(int argc, const char* argv[]) {
   PKZL_PROFILE_FUNCTION();
   return std::make_unique<Pikzelated>();
}
