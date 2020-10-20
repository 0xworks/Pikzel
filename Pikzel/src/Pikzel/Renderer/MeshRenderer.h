#pragma once

#include "GraphicsContext.h"
#include "Pikzel/Scene/Light.h"
#include "Pikzel/Scene/Mesh.h"
#include "Pikzel/Scene/Model.h"

#include <glm/glm.hpp>

#include <memory>

namespace Pikzel {

   struct DrawData {
      const glm::mat4& Projection;
      const glm::mat4& View;
      const glm::vec3& ViewPosition;
      const std::vector<DirectionalLight>& DirectionalLights;
      const std::vector<PointLight>& PointLights;
   };


   class MeshRenderer {
   public:
      MeshRenderer(GraphicsContext& gc, const Model& model);
      virtual ~MeshRenderer() = default;

      virtual void BindToGC(GraphicsContext& gc) const;

      virtual void SetTransforms(GraphicsContext& gc, const DrawData& drawData, const glm::mat4& transform) const;
      virtual void SetLighting(GraphicsContext& gc, const DrawData& drawData) const;

      virtual void Draw(GraphicsContext& gc, const Mesh& mesh) const;

   public:
      // provided by external renderer back-end.  Could return MeshRenderer directly, or a derived class
      static std::unique_ptr<MeshRenderer> Create(GraphicsContext& gc, const Model& model);

   protected:
      std::unique_ptr<Pipeline> m_Pipeline;

   private:
      std::unique_ptr<UniformBuffer> m_DirectionalLightBuffer;
      std::unique_ptr<UniformBuffer> m_PointLightBuffer;
   };

}
