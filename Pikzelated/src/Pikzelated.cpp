#include "Pikzel/Core/Application.h"
#include "Pikzel/Core/Window.h"
#include "Pikzel/Events/EventDispatcher.h"
#include "Pikzel/Events/KeyEvents.h"
#include "Pikzel/Events/MouseEvents.h"
#include "Pikzel/Events/WindowEvents.h"
#include "Pikzel/Renderer/GraphicsContext.h"
#include "Pikzel/Renderer/Renderer.h"


#include <glm/glm.hpp>
#include <imgui.h>
#include <imgui_internal.h>


// HACK: remove...
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "Pikzel/Platform/Vulkan/VulkanBuffer.h"

class Pikzelated final : public Pikzel::Application {
public:
   Pikzelated() {
      PKZL_PROFILE_FUNCTION();

      m_Image = Pikzel::Renderer::CreateImage();
      m_ImageGC = Pikzel::Renderer::CreateGraphicsContext(*m_Image);

      m_Window = Pikzel::Window::Create({APP_DESCRIPTION});
      Pikzel::EventDispatcher::Connect<Pikzel::WindowCloseEvent, &Pikzelated::OnWindowClose>(*this);

      ImGuiIO& io = ImGui::GetIO();
      ImGui::StyleColorsDark();
      ImGuiStyle& style = ImGui::GetStyle();
      if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
         style.WindowRounding = 0.0f;
         style.Colors[ImGuiCol_WindowBg].w = 1.0f;
      }
      float scaleFactor = m_Window->ContentScale();
      style.ScaleAllSizes(scaleFactor);

      if (!io.Fonts->AddFontFromFileTTF("assets/fonts/Cousine-Regular.ttf", 13 * scaleFactor)) {
         throw std::runtime_error("Failed to load ImGui font!");
      }

      m_Window->UploadImGuiFonts();

      ImGui::LoadIniSettingsFromDisk(io.IniFilename);
      if (!ImGui::GetCurrentContext()->SettingsLoaded) {
         ImGui::LoadIniSettingsFromDisk("EditorImGui.ini");
      }

   }


   ~Pikzelated() {
      m_Window.reset();
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

      m_ImageGC->BeginFrame();
      //
      // render to image GC here...
      //
      m_ImageGC->EndFrame();  // so, this submits the frame... but when is that completed? need a fence?  or does this block?  maybe the GC has a "waitidle" function?

      m_Window->BeginFrame();

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

            m_ImageGC->SwapBuffers(); // blocks until image has been rendered
            ImGui::Image(m_Image->GetImGuiTextureId(), ImVec2 {m_ViewportSize.x, m_ViewportSize.y});
            ImGui::End();
         }

         ImGui::End();
      }
      ImGui::PopStyleVar(3);
      io.DisplaySize = ImVec2((float)m_ViewportSize.x, (float)m_ViewportSize.y);

      ImGui::Render();

      m_Window->EndFrame();

   }


private:
   void OnWindowClose(const Pikzel::WindowCloseEvent& event) {
      if (event.Sender == m_Window->GetNativeWindow()) {
         m_Running = false;
      }
   }


   // HACK: remove...
   void CreateTextureResources() {
      int texWidth;
      int texHeight;
      int texChannels;

      stbi_uc* pixels = stbi_load("Assets/Textures/Statue.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
      vk::DeviceSize size = static_cast<vk::DeviceSize>(texWidth) * static_cast<vk::DeviceSize>(texHeight) * 4;

      if (!pixels) {
         throw std::runtime_error("failed to load texture image!");
      }

      std::unique_ptr<Pikzel::Buffer> stagingBuffer = Pikzel::Renderer::CreateBuffer(size);
      stagingBuffer->CopyFromHost(0, size, pixels);

      stbi_image_free(pixels);

      m_Texture = Pikzel::Renderer::CreateImage({static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight)});

      //TransitionImageLayout(m_Texture->m_Image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, 1); // <-- I dont understand WTF this is

      //m_Texture->CopyFromBuffer(stagingBuffer);

   }


private:
   glm::vec2 m_ViewportSize = {};
   std::unique_ptr<Pikzel::Image> m_Image;
   std::unique_ptr<Pikzel::GraphicsContext> m_ImageGC;
   std::unique_ptr<Pikzel::Window> m_Window;

   std::unique_ptr<Pikzel::Image> m_Texture;

};


std::unique_ptr<Pikzelated::Application> Pikzel::CreateApplication(int argc, const char* argv[]) {
   PKZL_PROFILE_FUNCTION();
   PKZL_CORE_LOG_INFO(APP_DESCRIPTION);
   PKZL_CORE_LOG_INFO("Linked against {0} {1}", PKZL_DESCRIPTION, PKZL_VERSION);
#ifdef PKZL_DEBUG
   PKZL_CORE_LOG_INFO("DEBUG build");
#endif
   return std::make_unique<Pikzelated>();
}


