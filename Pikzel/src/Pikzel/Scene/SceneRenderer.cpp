#include "Pikzel/Scene/SceneRenderer.h"

#include "Pikzel/Components/Mesh.h"
#include "Pikzel/Components/Transform.h"

namespace Pikzel {

   std::unique_ptr<SceneRenderer> CreateSceneRenderer(const GraphicsContext& gc) {
      return std::make_unique<SceneRenderer>(gc);
   }


   SceneRenderer::SceneRenderer(const GraphicsContext& gc) {

      // HACK: This needs to change,  obviously...
      //       We will want to be able to render with different "materials"
      m_Pipeline = gc.CreatePipeline({
         .shaders = {
            { Pikzel::ShaderType::Vertex, "Renderer/Triangle.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Renderer/Triangle.frag.spv" }
         },
         .bufferLayout = {{"inPos",   Pikzel::DataType::Vec3}, {"inColor", Pikzel::DataType::Vec3}}
      });

   }


   void SceneRenderer::RenderBegin() {
   }


   void SceneRenderer::Render(GraphicsContext& gc, Camera& camera, Scene& scene) {
      gc.BeginFrame();
      gc.Bind(*m_Pipeline);

      glm::mat4 vp = camera.projection * glm::lookAt(camera.position, camera.position + camera.direction, camera.upVector);

      // something like this.. only more complicated.. (e.g need materials, shadows, animation, ...)
      for (auto&& [entity, transform, mesh] : scene.m_Registry.group<const Transform, const Mesh>().each()) {
         gc.PushConstant("constants.mvp"_hs, vp * transform.Matrix);
         gc.DrawIndexed(*mesh.VertexBuffer, *mesh.IndexBuffer);
      }

      gc.EndFrame();
   }


   void SceneRenderer::RenderEnd() {
   }

}