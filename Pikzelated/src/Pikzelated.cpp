#include "Pikzel/Pikzel.h"
#include "Pikzel/Core/EntryPoint.h"
#include "Pikzel/Scene/AssetCache.h"

#include <imgui_internal.h>

// TODO: get these from somewhere
const float nearPlane = 1000.f;
const float farPlane = 0.1f;

class Pikzelated final : public Pikzel::Application {
   using super = Pikzel::Application;
public:
   Pikzelated()
   : Pikzel::Application{{.title = APP_DESCRIPTION, .clearColor = {0.0f, 0.0f, 0.0f, 1.0f}, .isMaximized = true, .isVSync = true}}
   , m_Input{GetWindow()}
   {
      PKZL_PROFILE_FUNCTION();

      Pikzel::ImGuiEx::Init(GetWindow());
      ImGuiIO& io = ImGui::GetIO();
      io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
      io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
      io.ConfigWindowsMoveFromTitleBarOnly = true;

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
      m_ScenePath.clear();

#if _DEBUG
      auto model = Pikzel::AssetCache::LoadModelResource("triangle", "Assets/Pikzelated/Models/Triangle.obj");

      Pikzel::Object triangle = m_Scene->CreateObject();
      m_Scene->AddComponent<Pikzel::Id>(triangle, 1234u);
      m_Scene->AddComponent<Pikzel::Transform>(triangle, glm::translate(glm::identity<glm::mat4>(), { 0.5, 0.5, 0.0 }));
      m_Scene->AddComponent<Pikzel::Model>(triangle, model);


      Pikzel::Object triangle2 = m_Scene->CreateObject();
      m_Scene->AddComponent<Pikzel::Id>(triangle2, 9999u);
      m_Scene->AddComponent<Pikzel::Transform>(triangle2, glm::scale(glm::identity<glm::mat4>(), { 0.5, 0.5, 1.0 }));
      m_Scene->AddComponent<Pikzel::Model>(triangle2, model);
#endif
   }


   void OnFileOpen() {
      auto path = Pikzel::OpenFileDialog("*.pkzl", "Pikzel Scene File (*.pkzl)");
      if (path.has_value()) {
         Pikzel::SceneSerializerYAML yaml{ {.Path = path.value() } };
         m_Scene = yaml.Deserialize();
         m_ScenePath = path.value();
      }
   }


   void SaveScene() {
      Pikzel::SceneSerializerYAML yaml{ {.Path = m_ScenePath} };
      yaml.Serialize(*m_Scene);
   }


   void OnFileSave() {
      if (m_Scene) {
         if (m_ScenePath.empty()) {
            OnFileSaveAs();
         } else {
            SaveScene();
         }
      }
   }


   void OnFileSaveAs() {
      if (m_Scene) {
         auto path = Pikzel::SaveFileDialog("*.pkzl", "Pikzel Scene File (*.pkzl)");
         if (path.has_value()) {
            if (path.value().extension() != ".pkzl") {
               path.value() += ".pkzl";
            }
            m_ScenePath = path.value();
            SaveScene();
         }
      }
   }


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
   }


   virtual void Render() override {
      PKZL_PROFILE_FUNCTION();

      Pikzel::GraphicsContext& gc = m_Framebuffer->GetGraphicsContext();
      gc.BeginFrame();
      if (m_Scene) {
         PKZL_PROFILE_SCOPE("render scene");
         m_SceneRenderer->Render(gc, m_Camera, *m_Scene);
      }
      gc.EndFrame();

      GetWindow().BeginFrame();
      GetWindow().BeginImGuiFrame();

      BeginDockSpace();
      {
         RenderMenuBar();
         RenderStatisticsPanel();
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
         //ImGuiWindowFlags_MenuBar |
         //ImGuiWindowFlags_NoDocking |
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

      ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

      ImGui::Begin("Dockspace", nullptr, dockspace_window_flags);
      {
         ImGuiID dockspaceId = ImGui::GetID("Dockspace");

         if (!ImGui::DockBuilderGetNode(dockspaceId)) {
            ImGuiID dockNode = dockspaceId;
            ImGui::DockBuilderAddNode(dockNode);
            ImGui::DockBuilderSetNodeSize(dockNode, { ImGui::GetIO().DisplaySize.x * ImGui::GetIO().DisplayFramebufferScale.x, ImGui::GetIO().DisplaySize.y * ImGui::GetIO().DisplayFramebufferScale.y });

            ImGuiID dockLeft = ImGui::DockBuilderSplitNode(dockNode, ImGuiDir_Left, 0.2f, nullptr, &dockNode);

            ImGui::DockBuilderDockWindow("Viewport", dockNode);
            ImGui::DockBuilderDockWindow("Statistics", dockLeft);

            ImGui::DockBuilderFinish(dockspaceId);
         }

         if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable) {
            ImGui::DockSpace(dockspaceId, {}, dockspace_flags);
         }
      }
   }


   void EndDockSpace() {
      ImGui::End();
      ImGui::PopStyleVar(3);
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
#if _DEBUG
            ImGui::Separator();
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
         ImGui::EndMainMenuBar();
      }
   }


   void RenderStatisticsPanel() {
      static ImGuiWindowFlags statisticPanelFlags = ImGuiWindowFlags_NoCollapse;
      if (m_ShowStatisticsPanel) {
         ImGui::Begin("Statistics", &m_ShowStatisticsPanel, statisticPanelFlags);
         {
            // auto stats = Renderer2D::GetStats();
            auto& io = ImGui::GetIO();
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
         }
         ImGui::End();
      }
   }

   void RenderViewport(Pikzel::GraphicsContext& gc) {
      //static ImGuiDockNodeFlags viewport_dockspace_flags = ImGuiDockNodeFlags_NoDockingOverMe | ImGuiDockNodeFlags_NoDockingOverOther | ImGuiDockNodeFlags_NoTabBar;

      static ImGuiWindowFlags viewport_window_flags = ImGuiWindowFlags_NoDecoration;// | ImGuiWindowFlags_NoDocking;
      //ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
      //ImGui::SetNextWindowSize(viewportPanelSize);
      //ImGui::SetNextWindowViewport(viewport->ID);
      ImGui::Begin("Viewport", nullptr, viewport_window_flags);
      {
         ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
         m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };
         //ImGui::DockSpace(ImGui::GetID("DockSpace"), ImVec2{ 0,0 }, viewport_dockspace_flags);
         gc.SwapBuffers();
         ImGui::Image(m_Framebuffer->GetImGuiColorTextureId(0), viewportPanelSize, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
         ImGui::GetIO().DisplaySize = ImVec2((float)m_ViewportSize.x, (float)m_ViewportSize.y); // what is this actually for?
      }
      ImGui::End();
   }


   virtual void RenderEnd() override {
      PKZL_PROFILE_FUNCTION();
   }


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
   std::filesystem::path m_ScenePath;
   std::unique_ptr<Pikzel::Scene> m_Scene;
   std::unique_ptr<Pikzel::SceneRenderer> m_SceneRenderer;
   std::unique_ptr<Pikzel::Framebuffer> m_Framebuffer;
   bool m_ShowStatisticsPanel = true;
#if _DEBUG
   bool m_ShowDemoWindow = false;
#endif
};


std::unique_ptr<Pikzel::Application> CreateApplication(int argc, const char* argv[]) {
   return std::make_unique<Pikzelated>();
}
