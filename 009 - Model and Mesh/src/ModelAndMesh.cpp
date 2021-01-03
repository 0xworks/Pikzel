#include "Pikzel/Pikzel.h"
#include "Pikzel/Core/EntryPoint.h"

// note: Pikzel uses reverse-Z so near and far planes are swapped
const float nearPlane = 10000.0f;
const float farPlane = 1.f;

class ModelAndMesh final : public Pikzel::Application {
public:
   ModelAndMesh()
   : Pikzel::Application {{.Title = APP_DESCRIPTION, .ClearColor = Pikzel::sRGB{0.1f, 0.1f, 0.2f}, .IsVSync = true}}
   , m_Input {GetWindow()}
   {

      m_Camera.Projection = glm::perspective(m_Camera.FoVRadians, static_cast<float>(GetWindow().GetWidth()) / static_cast<float>(GetWindow().GetHeight()), nearPlane, farPlane);

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
      m_DeltaTime = deltaTime;
      if (m_Input.IsKeyPressed(Pikzel::KeyCode::Escape)) {
         Exit();
      }
      m_Camera.Update(m_Input, deltaTime);
   }


   virtual void RenderBegin() override {}


   virtual void Render() override {
      PKZL_PROFILE_FUNCTION();

      glm::mat4 transform = glm::identity<glm::mat4>();
      glm::mat4 view = glm::lookAt(m_Camera.Position, m_Camera.Position + m_Camera.Direction, m_Camera.UpVector);
      {
         PKZL_PROFILE_SCOPE("Render model");

         // Render the model, positioned according to given transform, with camera and lighting as given
         m_ModelRenderer->Draw(
            GetWindow().GetGraphicsContext(),
            {m_Camera.Projection, view, m_Camera.Position, m_LightSpace, m_DirectionalLights, m_PointLights},
            transform
         );
      }

      {
         PKZL_PROFILE_SCOPE("Render light cubes");
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
      }

      {
         PKZL_PROFILE_SCOPE("Render ImGui");
         GetWindow().BeginImGuiFrame();
         {
            // You can draw multiple ImGui windows in here, you do not need to call Begin/End ImGuiFrame() again.
            ImGui::Begin("Lighting");
            for (size_t i = 0; i < m_PointLights.size(); ++i) {
               ImGuiDrawPointLight(fmt::format("light {0}", i).c_str(), m_PointLights[i]);
            }
            ImGui::Text("Frame time: %.3fms (%.0f FPS)", m_DeltaTime.count() * 1000.0f, 1.0f / m_DeltaTime.count());
            ImGui::End();
         }
         GetWindow().EndImGuiFrame();
      }
      GetWindow().EndFrame();
   }


   virtual void RenderEnd() override {}

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

      Pikzel::BufferLayout layout = {
         {"inPos", Pikzel::DataType::Vec3},
      };
      m_VertexBuffer = Pikzel::RenderCore::CreateVertexBuffer(layout, sizeof(vertices), vertices);
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
      glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 15.0f, -5.0f);  // TODO: how does one determine the parameters here?
      glm::mat4 lightView = glm::lookAt(-m_DirectionalLights[0].Direction, glm::vec3 {0.0f, 0.0f, 0.0f}, glm::vec3 {0.0f, 1.0f, 0.0f});
      m_LightSpace = lightProjection * lightView;

      //
      // TODO: change this so that the model renderer does not need to take model pointer.
      //       (and then can be used to render multiple different models)
      m_ModelRenderer = std::make_unique<Pikzel::ModelRenderer>(GetWindow().GetGraphicsContext(), Pikzel::ModelSerializer::Import("Assets/Models/Sponza/sponza.gltf"));
   }


   static void ImGuiDrawPointLight(const char* label, Pikzel::PointLight& pointLight) {
      ImGui::PushID(label);
      if (ImGui::TreeNode(label)) {
         Pikzel::ImGuiEx::EditVec3("Position", &pointLight.Position);
         Pikzel::ImGuiEx::EditVec3Color("Color", &pointLight.Color); 
         Pikzel::ImGuiEx::EditFloat("Size", &pointLight.Size);
         Pikzel::ImGuiEx::EditFloat("Power", &pointLight.Power);
         ImGui::TreePop();
      }
      ImGui::PopID();
   }


   virtual void OnWindowResize(const Pikzel::WindowResizeEvent& event) override {
      __super::OnWindowResize(event);
      m_Camera.Projection = glm::perspective(m_Camera.FoVRadians, static_cast<float>(event.Width) / static_cast<float>(event.Height), nearPlane, farPlane);
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
         .Color = Pikzel::sRGB{0.0f, 0.0f, 0.0f},
         .Ambient = Pikzel::sRGB{0.1f, 0.1f, 0.1f},
         .Size = 1.0f
      }
   };

   glm::mat4 m_LightSpace;

   // note: currently shader supports between 1 and 32 point lights
   // for zero points lights, pass in one, with power set to 0
   std::vector<Pikzel::PointLight> m_PointLights = {
      {
         .Position = {-619.3f, 130.3f, -219.5f},
         .Color = Pikzel::sRGB{1.0f, 1.0f, 1.0f},
         .Size = 1.0f,
         .Power = 30000.0f
      },
      {
         .Position = {487.3f, 130.3f, -219.5f},
         .Color = Pikzel::sRGB{1.0f, 1.0f, 1.0f},
         .Size = 1.0f,
         .Power = 30000.0f
      },
      {
         .Position = {487.3f, 130.3f, 141.1f},
         .Color = Pikzel::sRGB{1.0f, 1.0f, 1.0f},
         .Size = 1.0f,
         .Power = 30000.0f
      },
      {
         .Position = {-619.3f, 130.3f, 141.1f},
         .Color = Pikzel::sRGB{1.0f, 1.0f, 1.0f},
         .Size = 1.0f,
         .Power = 30000.0f
      }
   };

   std::shared_ptr<Pikzel::VertexBuffer> m_VertexBuffer;
   std::unique_ptr<Pikzel::Pipeline> m_PipelineLight;
   std::unique_ptr<Pikzel::ModelRenderer> m_ModelRenderer;

   Pikzel::DeltaTime m_DeltaTime;
};


std::unique_ptr<Pikzel::Application> CreateApplication(int argc, const char* argv[]) {
   return std::make_unique<ModelAndMesh>();
}
