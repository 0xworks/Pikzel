#include "Panel.h"
#include "HierarchyPanel.h"
#include "SceneEditor.h"

#include "Pikzel/Core/EntryPoint.h"
#include "Pikzel/Core/PlatformUtility.h"
#include "Pikzel/ImGui/ImGuiEx.h"
#include "Pikzel/Input/KeyCodes.h"
#include "Pikzel/Renderer/RenderCore.h"
#include "Pikzel/Scene/SceneRenderer.h"
#include "Pikzel/Scene/SceneSerializer.h"

#include <imgui_internal.h>


std::string MakeViewportWindowName(const std::filesystem::path& path) {
   std::string windowName = "<unnamed scene>###Viewport";
   if (!path.empty()) {
      windowName = path.filename().string() + "###Viewport";
   }
   return windowName;
}


class Pikzelated final : public Pikzel::Application {
   using super = Pikzel::Application;
public:
   Pikzelated()
   : Pikzel::Application{{.title = APP_DESCRIPTION, .clearColor = {0.0f, 0.0f, 0.0f, 1.0f}, .isMaximized = true, .isVSync = true}}
   , m_Editor{std::move(std::make_unique<Pikzel::Scene>())}
   , m_Input{GetWindow()}
   {
      PKZL_PROFILE_FUNCTION();

      Pikzel::ImGuiEx::Init(GetWindow());

      m_Framebuffer = Pikzel::RenderCore::CreateFramebuffer({.width = m_Editor.GetViewportSize().x, .height = m_Editor.GetViewportSize().y, .msaaNumSamples = 4});
      m_SceneRenderer = Pikzel::CreateSceneRenderer(m_Framebuffer->GetGraphicsContext());

      m_Panels.emplace_back(std::make_unique<HierarchyPanel>(m_Editor));
   }


protected:

   virtual void Update(Pikzel::DeltaTime deltaTime) override {
      PKZL_PROFILE_FUNCTION();
      if (m_Input.IsKeyPressed(Pikzel::KeyCode::Escape)) {
         Exit();
      }
      m_Editor.Update(m_Input, deltaTime);
   }


   void OnFileNew() {
      m_Editor.SetScene(std::move(std::make_unique<Pikzel::Scene>()));
      m_ScenePath.clear();
      m_ViewportWindowName = MakeViewportWindowName(m_ScenePath);
   }


   void OnFileOpen() {
      auto path = Pikzel::OpenFileDialog("*.pkzl", "Pikzel Scene File (*.pkzl)");
      if (path.has_value()) {
         Pikzel::SceneSerializerYAML yaml{{.Path = path.value() }};
         m_Editor.SetScene(std::move(yaml.Deserialize()));
         m_ScenePath = path.value();
         m_ViewportWindowName = MakeViewportWindowName(m_ScenePath);
      }
   }


   void SaveScene() {
      Pikzel::SceneSerializerYAML yaml{{.Path = m_ScenePath}};
      yaml.Serialize(m_Editor.GetScene());
   }


   void OnFileSave() {
      if (m_ScenePath.empty()) {
         OnFileSaveAs();
      } else {
         SaveScene();
      }
   }


   void OnFileSaveAs() {
      auto path = Pikzel::SaveFileDialog("*.pkzl", "Pikzel Scene File (*.pkzl)");
      if (path.has_value()) {
         if (path.value().extension() != ".pkzl") {
            path.value() += ".pkzl";
         }
         m_ScenePath = path.value();
         m_ViewportWindowName = MakeViewportWindowName(m_ScenePath);
         SaveScene();
      }
   }


   void OnFileExit() {
      Exit();
   }


   virtual void RenderBegin() override {
      PKZL_PROFILE_FUNCTION();
      if (auto size = m_Editor.GetViewportSize();
         (size.x != m_Framebuffer->GetWidth()) ||
         (size.y != m_Framebuffer->GetHeight())
      ) {
         m_Framebuffer = Pikzel::RenderCore::CreateFramebuffer({.width = size.x, .height = size.y, .msaaNumSamples = 4});
      }
   }


   virtual void Render() override {
      PKZL_PROFILE_FUNCTION();

      Pikzel::GraphicsContext& gc = m_Framebuffer->GetGraphicsContext();
      gc.BeginFrame();
      {
         PKZL_PROFILE_SCOPE("render scene");
         m_SceneRenderer->Render(gc, m_Editor.GetCamera(), m_Editor.GetScene());
      }
      gc.EndFrame();

      GetWindow().BeginFrame();
      GetWindow().BeginImGuiFrame();

      BeginDockSpace();
      {
         RenderMenuBar();
         for (auto& panel : m_Panels) {
            panel->Render();
         }
         RenderViewport(gc);
      }
      EndDockSpace();

#if _DEBUG
      if (m_ShowDemoWindow) {
          ImGui::ShowDemoWindow(&m_ShowDemoWindow);
      }
#endif

      GetWindow().EndImGuiFrame();
      GetWindow().EndFrame();
   }


   void BeginDockSpace() {
      static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoCloseButton;
      static ImGuiWindowFlags dockspace_window_flags =
         ImGuiWindowFlags_NoTitleBar |
         ImGuiWindowFlags_NoCollapse |
         ImGuiWindowFlags_NoResize |
         ImGuiWindowFlags_NoMove |
         ImGuiWindowFlags_NoBringToFrontOnFocus |
         ImGuiWindowFlags_NoNavFocus
      ;

      ImGuiViewport* viewport = ImGui::GetMainViewport();

      auto pos = viewport->Pos;
      auto size = viewport->Size;
      const float infoBarSize = ImGui::GetFrameHeight();
      pos.y += infoBarSize;
      size.y -= infoBarSize;

      ImGui::SetNextWindowPos(pos);
      ImGui::SetNextWindowSize(size);
      ImGui::SetNextWindowViewport(viewport->ID);

      ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

      ImGui::Begin("Dockspace", nullptr, dockspace_window_flags);
      {
         ImGuiID dockspaceId = ImGui::GetID("Dockspace");

         if (!ImGui::DockBuilderGetNode(dockspaceId) || m_ResetWindowLayout) {
            ImGui::DockBuilderRemoveNode(dockspaceId);

            ImGuiID dockNode = dockspaceId;
            ImGui::DockBuilderAddNode(dockNode);
            ImGui::DockBuilderSetNodeSize(dockNode, { ImGui::GetIO().DisplaySize.x * ImGui::GetIO().DisplayFramebufferScale.x, ImGui::GetIO().DisplaySize.y * ImGui::GetIO().DisplayFramebufferScale.y });

            ImGuiID dockRight = ImGui::DockBuilderSplitNode(dockNode, ImGuiDir_Right, 0.2f, nullptr, &dockNode);

            ImGui::DockBuilderDockWindow("###Viewport", dockNode);
            ImGui::DockBuilderDockWindow("###Hierarchy", dockRight);

            ImGui::DockBuilderFinish(dockspaceId);
            m_ResetWindowLayout = false;
         }

         if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable) {
            ImGui::DockSpace(dockspaceId, {}, dockspace_flags);
         }
         ImGui::PopStyleVar(); // window padding
      }
   }


   void EndDockSpace() {
      ImGui::End();
      ImGui::PopStyleVar(); // window border
   }


   void RenderMenuBar() {
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
            ImGui::Separator();
            if (ImGui::BeginMenu("Theme")) {
               if (ImGui::MenuItem("Light")) {
                  Pikzel::ImGuiEx::SetColors(Pikzel::ImGuiEx::Theme::Light);
               }
               if (ImGui::MenuItem("Dark")) {
                  Pikzel::ImGuiEx::SetColors(Pikzel::ImGuiEx::Theme::Dark);
               }
               ImGui::EndMenu();
            }
            if (ImGui::MenuItem("Reset Window Layout")) {
               m_ResetWindowLayout = true;
            }
#if _DEBUG
            if (ImGui::MenuItem("Show ImGui Demo")) {
               m_ShowDemoWindow = true;
            }
#endif
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
               Exit();
            }
            ImGui::EndMenu();
         }

         auto& io = ImGui::GetIO();
         std::string fps = fmt::format("Frame time {:.3f} ms ({:.0f} FPS)", 1000.0f / io.Framerate, io.Framerate);
         auto size = ImGui::CalcTextSize(fps.data());
         ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - size.x - ImGui::GetStyle().ItemSpacing.x);
         ImGui::Text(fps.data());

         ImGui::EndMainMenuBar();
      }
   }


   void RenderViewport(Pikzel::GraphicsContext& gc) {
      static ImGuiWindowFlags viewportWindowFlags = ImGuiWindowFlags_NoCollapse;

      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
      ImGui::Begin(m_ViewportWindowName.data(), nullptr, viewportWindowFlags);
      {
         ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
         m_Editor.SetViewportSize({viewportPanelSize.x, viewportPanelSize.y});
         gc.SwapBuffers();
         ImGui::Image(m_Framebuffer->GetImGuiColorTextureId(0), viewportPanelSize, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
      }
      ImGui::End();
      ImGui::PopStyleVar();
   }


   virtual void RenderEnd() override {
      PKZL_PROFILE_FUNCTION();
   }


private:
   SceneEditor m_Editor;
   std::filesystem::path m_ScenePath;

   Pikzel::Input m_Input;

   std::vector<std::unique_ptr<Panel>> m_Panels;

   std::string m_ViewportWindowName = MakeViewportWindowName({});

   std::unique_ptr<Pikzel::SceneRenderer> m_SceneRenderer;
   std::unique_ptr<Pikzel::Framebuffer> m_Framebuffer;

   bool m_ResetWindowLayout = false;
   bool m_ShowHierarchy = true;
#if _DEBUG
   bool m_ShowDemoWindow = false;
#endif
};


std::unique_ptr<Pikzel::Application> CreateApplication(int argc, const char* argv[]) {
   return std::make_unique<Pikzelated>();
}
