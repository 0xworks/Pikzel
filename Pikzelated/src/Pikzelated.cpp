#include "Pikzel/Core/Application.h"
#include "Pikzel/Core/Window.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>

class Pikzelated : public Pikzel::Application {
public:
   Pikzelated() {
      PKZL_PROFILE_FUNCTION();

      m_Window = Pikzel::Window::Create();

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

      glfwMakeContextCurrent((GLFWwindow*)m_Window->GetNativeWindow());
      int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
      PKZL_CORE_ASSERT(status, "Failed to initialize Glad!");

      ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)m_Window->GetNativeWindow(), true);
      ImGui_ImplOpenGL3_Init();

      ImGui::LoadIniSettingsFromDisk(io.IniFilename);
      if (!ImGui::GetCurrentContext()->SettingsLoaded) {
         ImGui::LoadIniSettingsFromDisk("EditorImGui.ini");
      }
   }


   ~Pikzelated() {
      PKZL_PROFILE_FUNCTION();

      // TODO: move somewhere else
      ImGui_ImplOpenGL3_Shutdown();
      ImGui_ImplGlfw_Shutdown();
      ImGui::DestroyContext();
   }


   virtual void Update(Pikzel::DeltaTime deltaTime) override {
      // TODO
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

      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();

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
                  m_Running = false;
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

            // what the heck is this?
            // Need to figure out what "framebuffer" is here - Its an OpenGL framebuffer

            // uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
            // ImGui::Image((void*)textureID, ImVec2 {m_ViewportSize.x, m_ViewportSize.y}, ImVec2 {0, 1}, ImVec2 {1, 0});
            ImGui::End();
         }

         ImGui::End();
      }
      ImGui::PopStyleVar(3);
      io.DisplaySize = ImVec2((float)m_ViewportSize.x, (float)m_ViewportSize.y);

      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

      if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
         GLFWwindow* currentContext = glfwGetCurrentContext();
         ImGui::UpdatePlatformWindows();
         ImGui::RenderPlatformWindowsDefault();
         glfwMakeContextCurrent(currentContext);
      }

      glfwPollEvents();
      glfwSwapBuffers((GLFWwindow*)m_Window->GetNativeWindow());
   }


private:
   glm::vec2 m_ViewportSize = {};
   std::unique_ptr<Pikzel::Window> m_Window;

};


std::unique_ptr<Pikzelated::Application> Pikzel::CreateApplication(int argc, const char* argv[]) {
   PKZL_PROFILE_FUNCTION();
   PKZL_CORE_LOG_INFO(APP_DESCRIPTION);
   PKZL_CORE_LOG_INFO("Linked against {0} {1}", PKZL_DESCRIPTION, PKZL_VERSION);
   return std::make_unique<Pikzelated>();
}
