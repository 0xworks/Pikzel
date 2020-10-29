#include "Camera.h"
#include "ImGuiEx.h"

#include "Pikzel/Core/Application.h"
#include "Pikzel/Core/Utility.h"
#include "Pikzel/Input/Input.h"
#include "Pikzel/Renderer/ModelRenderer.h"
#include "Pikzel/Renderer/RenderCore.h"
#include "Pikzel/Scene/Light.h"
#include "Pikzel/Scene/ModelSerializer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

//#include <imgui.h>
//#include <imgui/imgui_internal.h>
#include <filesystem>


class ModelAndMesh final : public Pikzel::Application {
public:
   ModelAndMesh(int argc, const char* argv[])
   : Pikzel::Application {argc, argv, {.Title = APP_DESCRIPTION, .ClearColor = {0.5f, 0.5f, 0.9f, 1.0f}, .IsVSync = true}}
   , m_bindir {argv[0]}
   , m_Input {GetWindow()}
   {
      m_bindir.remove_filename();

      // for rendering point lights as cubes
      CreateVertexBuffer();
      CreateIndexBuffer();
      CreatePipelines();

      // renders the actual scene
      CreateModelRenderer();

      m_Camera.Projection = glm::perspective(m_Camera.FoVRadians, static_cast<float>(GetWindow().GetWidth()) / static_cast<float>(GetWindow().GetHeight()), 1.0f, 10000.0f);

      // In order to render ImGui widgets, you need to initialize ImGui integration.
      // Clients that do not wish to use ImGui simply don't call this (and so "pay" nothing)
      GetWindow().InitializeImGui();

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


   ~ModelAndMesh() {
      m_ModelRenderer.reset();
   }


   virtual void Update(const Pikzel::DeltaTime deltaTime) override {
      PKZL_PROFILE_FUNCTION();
      if (m_Input.IsKeyPressed(Pikzel::KeyCode::Escape)) {
         Exit();
      }
      m_Camera.Update(m_Input, deltaTime);
   }


   virtual void Render() override {
      PKZL_PROFILE_FUNCTION();

      glm::mat4 transform = glm::identity<glm::mat4>();
      glm::mat4 view = glm::lookAt(m_Camera.Position, m_Camera.Position + m_Camera.Direction, m_Camera.UpVector);

      // Render point lights as little cubes
      Pikzel::GraphicsContext& gc = GetWindow().GetGraphicsContext();
      glm::mat4 projView = m_Camera.Projection * view;
      {
         gc.Bind(*m_PipelineLight);
         for (const auto& pointLight : m_PointLights) {
            glm::mat4 model = glm::translate(glm::identity<glm::mat4>(), pointLight.Position);
            gc.PushConstant("constants.mvp"_hs, projView * model);
            gc.PushConstant("constants.lightColor"_hs, pointLight.Color);
            gc.DrawIndexed(*m_VertexBuffer, *m_IndexBuffer);
         }
      }

      // Render the model, positioned according to given transform, with camera and lighting as given
      m_ModelRenderer->Draw(
         GetWindow().GetGraphicsContext(),
         {m_Camera.Projection, view, m_Camera.Position, m_DirectionalLights, m_PointLights},
         transform
      );

      // If you want to render ImGui windows, do it between calls to BeginImGuiFrame() and EndImGuiFrame()
      GetWindow().BeginImGuiFrame();
      {
         // You can draw multiple ImGui windows in here, you do not need to call Begin/End ImGuiFrame() again.
         ImGui::Begin("Point Lights");
         for (size_t i = 0; i < m_PointLights.size(); ++i) {
            ImGuiDrawPointLight(fmt::format("light {0}", i).c_str(), m_PointLights[i]);
         }
         ImGui::End();
      }
      GetWindow().EndImGuiFrame();
   }


private:

   void CreateVertexBuffer() {
      glm::vec3 vertices[] = {
         {-0.5f, -0.5f, -0.5f},
         { 0.5f,  0.5f, -0.5f},
         { 0.5f, -0.5f, -0.5f},
         { 0.5f,  0.5f, -0.5f},
         {-0.5f, -0.5f, -0.5f},
         {-0.5f,  0.5f, -0.5f},

         {-0.5f, -0.5f,  0.5f},
         { 0.5f, -0.5f,  0.5f},
         { 0.5f,  0.5f,  0.5f},
         { 0.5f,  0.5f,  0.5f},
         {-0.5f,  0.5f,  0.5f},
         {-0.5f, -0.5f,  0.5f},

         {-0.5f,  0.5f,  0.5f},
         {-0.5f,  0.5f, -0.5f},
         {-0.5f, -0.5f, -0.5f},
         {-0.5f, -0.5f, -0.5f},
         {-0.5f, -0.5f,  0.5f},
         {-0.5f,  0.5f,  0.5f},

         { 0.5f,  0.5f,  0.5f},
         { 0.5f, -0.5f, -0.5f},
         { 0.5f,  0.5f, -0.5f},
         { 0.5f, -0.5f, -0.5f},
         { 0.5f,  0.5f,  0.5f},
         { 0.5f, -0.5f,  0.5f},

         {-0.5f, -0.5f, -0.5f},
         { 0.5f, -0.5f, -0.5f},
         { 0.5f, -0.5f,  0.5f},
         { 0.5f, -0.5f,  0.5f},
         {-0.5f, -0.5f,  0.5f},
         {-0.5f, -0.5f, -0.5f},

         {-0.5f,  0.5f, -0.5f},
         { 0.5f,  0.5f,  0.5f},
         { 0.5f,  0.5f, -0.5f},
         { 0.5f,  0.5f,  0.5f},
         {-0.5f,  0.5f, -0.5f},
         {-0.5f,  0.5f,  0.5f}
      };

      m_VertexBuffer = Pikzel::RenderCore::CreateVertexBuffer(sizeof(vertices), vertices);
      m_VertexBuffer->SetLayout({
         { "inPos",      Pikzel::DataType::Vec3 },
      });
   }


   void CreateIndexBuffer() {
      uint32_t indices[] = {
         0,1,2,
         3,4,5,
         6,7,8,
         9,10,11,
         12,13,14,
         15,16,17,
         18,19,20,
         21,22,23,
         24,25,26,
         27,28,29,
         30,31,32,
         33,34,35
      };

      m_IndexBuffer = Pikzel::RenderCore::CreateIndexBuffer(sizeof(indices) / sizeof(uint32_t), indices);
   }


   void CreatePipelines() {
      m_PipelineLight = GetWindow().GetGraphicsContext().CreatePipeline({
         m_VertexBuffer->GetLayout(),
         {
            { Pikzel::ShaderType::Vertex, m_bindir / "Assets/Shaders/Light.vert.spv" },
            { Pikzel::ShaderType::Fragment, m_bindir / "Assets/Shaders/Light.frag.spv" }
         }
      });
   }


   void CreateModelRenderer() {
      m_ModelRenderer = std::make_unique<Pikzel::ModelRenderer>(GetWindow().GetGraphicsContext(), Pikzel::ModelSerializer::Import("../Data/Models/Sponza/sponza.gltf"));
   }


   static void ImGuiDrawPointLight(const char* label, Pikzel::PointLight& pointLight) {
      ImGui::PushID(label);
      if (ImGui::TreeNode(label)) {
         ImGuiEx::EditVec3("Position", pointLight.Position);
         ImGuiEx::EditVec3Color("Color", pointLight.Color);
         ImGui::TreePop();
      }
      ImGui::PopID();
   }

private:

   Pikzel::Input m_Input;
   std::filesystem::path m_bindir;

   Camera m_Camera = {
      .Position = {-900.0f, 100.0f, 0.0f},
      .Direction = glm::normalize(glm::vec3{900.0f, -100.0f, 0.0f}),
      .UpVector = {0.0f, 1.0f, 0.0f},
      .FoVRadians = glm::radians(60.f),
      .MoveSpeed = 200.0f,
      .RotateSpeed = 20.0f
   };

   // note: currently shader expects exactly 1 directional light
   std::vector<Pikzel::DirectionalLight> m_DirectionalLights = {
      {
         .Direction = {-0.0f, -1.0f, -2.0f},
         .Color = {0.6f, 0.6f, 0.6f},
         .Ambient = {0.1f, 0.1f, 0.1f}
      }
   };

   // note: currently shader expects exactly 4 point lights
   std::vector<Pikzel::PointLight> m_PointLights = {
      {
         .Position = {-619.3f, 130.3f, -219.5f},
         .Color = {1.0f, 0.0f, 0.0f},
         .Constant = 1.0f,
         .Linear = 0.00009f,
         .Quadratic = 0.000032f
      },
      {
         .Position = {487.3f, 130.3f, -219.5f},
         .Color = {0.0f, 1.0f, 0.0f},
         .Constant = 1.0f,
         .Linear = 0.0009f,
         .Quadratic = 0.00032f
      },
      {
         .Position = {487.3f, 130.3f, 141.1f},
         .Color = {0.0f, 1.0f, 1.0f},
         .Constant = 1.0f,
         .Linear = 0.00009f,
         .Quadratic = 0.000032f
      },
      {
         .Position = {-619.3f, 130.3f, 141.1f},
         .Color = {1.0f, 1.0f, 0.0f},
         .Constant = 1.0f,
         .Linear = 0.00009f,
         .Quadratic = 0.000032f
      }
   };

   std::shared_ptr<Pikzel::VertexBuffer> m_VertexBuffer;
   std::shared_ptr<Pikzel::IndexBuffer> m_IndexBuffer;
   std::unique_ptr<Pikzel::Pipeline> m_PipelineLight;
   std::unique_ptr<Pikzel::Pipeline> m_PipelineLighting;
   std::unique_ptr<Pikzel::ModelRenderer> m_ModelRenderer;

};


std::unique_ptr<Pikzel::Application> Pikzel::CreateApplication(int argc, const char* argv[]) {
   PKZL_LOG_INFO(APP_DESCRIPTION);
   PKZL_LOG_INFO("Linked against {0} {1}", PKZL_DESCRIPTION, PKZL_VERSION);
#ifdef PKZL_DEBUG
   PKZL_LOG_INFO("DEBUG build");
#endif

   return std::make_unique<ModelAndMesh>(argc, argv);
}
