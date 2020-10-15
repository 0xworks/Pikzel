#pragma once

#include "Mesh.h"

#include <glm/glm.hpp>
#include <vector>

namespace Pikzel {

   struct Model {
      std::vector<Mesh> Meshes;

      ~Model() {
         ;
      }

   };


   class ModelRenderer {
   public:
      ModelRenderer(GraphicsContext& gc);
      virtual ~ModelRenderer();

      struct DrawData {
         const glm::mat4& ProjView;
         const glm::vec3& ViewPos;
         const std::vector<DirectionalLight>& DirectionalLights;
         const std::vector<PointLight>& PointLights;
      };

      void Draw(GraphicsContext& gc, const DrawData drawData, const Model& model, const glm::mat4& transform);

   private:
      std::unique_ptr<MeshRenderer> m_MeshRenderer;
   };

}
