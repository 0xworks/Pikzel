#include "SceneRenderer.h"

#include "Pikzel/Components/Model.h"
#include "Pikzel/Components/Transform.h"
#include "Pikzel/Scene/AssetCache.h"

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
         .bufferLayout = Mesh::VertexBufferLayout
      });

   }


   void SceneRenderer::Render(GraphicsContext& gc, Camera& camera, Scene& scene) {
      gc.Bind(*m_Pipeline);

      glm::mat4 vp = camera.projection * glm::lookAt(camera.position, camera.position + camera.direction, camera.upVector);

      // something like this.. only more complicated.. (e.g need materials, shadows, animation, ...)
      for (auto&& [entity, transform, model] : scene.m_Registry.group<const Transform, const Model>().each()) {
         gc.PushConstant("constants.mvp"_hs, vp * transform.Matrix);

         auto modelResource = AssetCache::GetModelResource(model.Id);

         for (const auto& mesh : modelResource->Meshes) {
            //gc.PushConstant("constants.mvp"_hs, transform * mesh.Transform);
            //gc.Bind("uAlbedo"_hs, *mesh.AlbedoTexture);
            //gc.Bind("uMetallicRoughness"_hs, *mesh.MetallicRoughnessTexture);
            //gc.Bind("uNormals"_hs, *mesh.NormalTexture);
            //gc.Bind("uAmbientOcclusion"_hs, *mesh.AmbientOcclusionTexture);
            //gc.Bind("uHeightMap"_hs, *mesh.HeightTexture);
            gc.DrawIndexed(*mesh.VertexBuffer, *mesh.IndexBuffer);
         }
      }
   }

}