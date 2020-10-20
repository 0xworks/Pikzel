#include "Camera.h"

#include "Pikzel/Core/Application.h"
#include "Pikzel/Core/Utility.h"
#include "Pikzel/Input/Input.h"
#include "Pikzel/Renderer/ModelRenderer.h"
#include "Pikzel/Scene/Light.h"
#include "Pikzel/Scene/ModelSerializer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <filesystem>

class ModelAndMesh final : public Pikzel::Application {
public:
   ModelAndMesh(int argc, const char* argv[])
   : Pikzel::Application {argc, argv, {.Title = "Model and Mesh Demo", .ClearColor = {0.5f, 0.5f, 0.9f, 1.0f}, .IsVSync = true}}
   , m_bindir {argv[0]}
   , m_Input {GetWindow()}
   {
      m_bindir.remove_filename();

      CreateModelRenderer();

      m_Camera.Projection = glm::perspective(m_Camera.FoVRadians, static_cast<float>(GetWindow().GetWidth()) / static_cast<float>(GetWindow().GetHeight()), 1.0f, 10000.0f);
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

      m_ModelRenderer->Draw(
         GetWindow().GetGraphicsContext(),
         {m_Camera.Projection, glm::lookAt(m_Camera.Position, m_Camera.Position + m_Camera.Direction, m_Camera.UpVector), m_Camera.Position, m_DirectionalLights, m_PointLights},
         transform
      );
   }


private:

   void CreateModelRenderer() {
      m_ModelRenderer = std::make_unique<Pikzel::ModelRenderer>(GetWindow().GetGraphicsContext(), Pikzel::ModelSerializer::Import("../Data/Models/Sponza/sponza.gltf"));
   }


private:

   Pikzel::Input m_Input;
   std::filesystem::path m_bindir;

   Camera m_Camera = {
      .Position = {-900.0f, 100.0f, 0.0f},
      .Direction = glm::normalize(glm::vec3{900.0f, -100.0f, 0.0f}),
      .UpVector = {0.0f, 1.0f, 0.0f},
      .FoVRadians = glm::radians(60.f),
      .MoveSpeed = 20.0f
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
         .Position = {-700.0f, 50.0f, 200.0f},
         .Color = {1.0f, 0.0f, 0.0f},
         .Constant = 1.0f,
         .Linear = 0.00009f,
         .Quadratic = 0.000032f
      },
      {
         .Position = {100.0f, 50.0f, 200.0f},
         .Color = {0.0f, 1.0f, 0.0f},
         .Constant = 1.0f,
         .Linear = 0.0009f,
         .Quadratic = 0.00032f
      },
      {
         .Position = {700.0f, 50.0f, 200.0f},
         .Color = {0.0f, 1.0f, 1.0f},
         .Constant = 1.0f,
         .Linear = 0.00009f,
         .Quadratic = 0.000032f
      },
      {
         .Position = {900.0f, 50.0f, 200.0f},
         .Color = {1.0f, 1.0f, 0.0f},
         .Constant = 1.0f,
         .Linear = 0.00009f,
         .Quadratic = 0.000032f
      }
   };

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
