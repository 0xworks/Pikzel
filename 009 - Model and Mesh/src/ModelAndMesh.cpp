#include "Camera.h"

#include "Pikzel/Pikzel.h"

#include "Pikzel/Core/EntryPoint.h"

class ModelAndMesh final : public Pikzel::Application {
public:
   ModelAndMesh()
   : Pikzel::Application {{.Title = APP_DESCRIPTION, .ClearColor = Pikzel::sRGB{0.1f, 0.1f, 0.2f}, .IsVSync = true}}
   , m_Input {GetWindow()}
   {

      m_Camera.Projection = glm::perspective(m_Camera.FoVRadians, static_cast<float>(GetWindow().GetWidth()) / static_cast<float>(GetWindow().GetHeight()), 1.0f, 10000.0f);

      // for rendering point lights as cubes
      CreateVertexBuffer();
      CreatePipelines();

      // renders the actual scene
      CreateModelRenderer();

      // In order to render ImGui widgets, you need to initialize ImGui integration.
      // Clients that do not wish to use ImGui simply don't call this (and so "pay" nothing)
      Pikzel::ImGuiEx::Init(GetWindow());
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
            gc.DrawTriangles(*m_VertexBuffer, 36);
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


   void CreatePipelines() {
      m_PipelineLight = GetWindow().GetGraphicsContext().CreatePipeline({
         m_VertexBuffer->GetLayout(),
         {
            { Pikzel::ShaderType::Vertex, "Assets/" APP_NAME "/Shaders/Light.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Assets/" APP_NAME "/Shaders/Light.frag.spv" }
         }
      });
   }


   void CreateModelRenderer() {
      m_ModelRenderer = std::make_unique<Pikzel::ModelRenderer>(GetWindow().GetGraphicsContext(), Pikzel::ModelSerializer::Import("Assets/Models/Sponza/sponza.gltf"));
   }


   static void ImGuiDrawPointLight(const char* label, Pikzel::PointLight& pointLight) {
      ImGui::PushID(label);
      if (ImGui::TreeNode(label)) {
         Pikzel::ImGuiEx::EditVec3("Position", &pointLight.Position);
         Pikzel::ImGuiEx::EditVec3Color("Color", &pointLight.Color); 
         Pikzel::ImGuiEx::EditFloat("Power", &pointLight.Power);
         ImGui::TreePop();
      }
      ImGui::PopID();
   }


   virtual void OnWindowResize(const Pikzel::WindowResizeEvent& event) override {
      __super::OnWindowResize(event);

      m_Camera.Projection = glm::perspective(m_Camera.FoVRadians, static_cast<float>(event.Width) / static_cast<float>(event.Height), 1.0f, 10000.0f);
   }


private:

   Pikzel::Input m_Input;

   Camera m_Camera = {
      .Position = {-900.0f, 100.0f, 0.0f},
      .Direction = glm::normalize(glm::vec3{900.0f, -100.0f, 0.0f}),
      .UpVector = {0.0f, 1.0f, 0.0f},
      .FoVRadians = glm::radians(45.f),
      .MoveSpeed = 200.0f,
      .RotateSpeed = 20.0f
   };

   // note: currently shader expects exactly 1 directional light
   std::vector<Pikzel::DirectionalLight> m_DirectionalLights = {
      {
         .Direction = {-1.0f, -1.0f, 0.0f},
         .Color = Pikzel::sRGB{0.5f, 0.5f, 0.5f},
         .Ambient = Pikzel::sRGB{0.1f, 0.1f, 0.1f}
      }
   };

   // note: currently shader expects exactly 4 point lights
   std::vector<Pikzel::PointLight> m_PointLights = {
      {
         .Position = {-619.3f, 130.3f, -219.5f},
         .Color = Pikzel::sRGB{0.8f, 0.8f, 0.0f},
         .Power = 6000.0f
      },
      {
         .Position = {487.3f, 130.3f, -219.5f},
         .Color = Pikzel::sRGB{0.8f, 0.8f, 0.0f},
         .Power = 6000.0f
      },
      {
         .Position = {487.3f, 130.3f, 141.1f},
         .Color = Pikzel::sRGB{0.8f, 0.8f, 1.0f},
         .Power = 6000.0f
      },
      {
         .Position = {-619.3f, 130.3f, 141.1f},
         .Color = Pikzel::sRGB{0.8f, 0.8f, 0.0f},
         .Power = 6000.0f
      }
   };

   std::shared_ptr<Pikzel::VertexBuffer> m_VertexBuffer;
   std::unique_ptr<Pikzel::Pipeline> m_PipelineLight;
   std::unique_ptr<Pikzel::Pipeline> m_PipelineLighting;
   std::unique_ptr<Pikzel::ModelRenderer> m_ModelRenderer;

};


std::unique_ptr<Pikzel::Application> CreateApplication(int argc, const char* argv[]) {
   return std::make_unique<ModelAndMesh>();
}
