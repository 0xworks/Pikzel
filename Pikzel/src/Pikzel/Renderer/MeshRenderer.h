#pragma once

#include "GraphicsContext.h"
#include "Pikzel/Scene/Light.h"
#include "Pikzel/Scene/Mesh.h"
#include "Pikzel/Scene/Model.h"

#include <glm/glm.hpp>

#include <memory>

namespace Pikzel {

   struct PKZL_API DrawData {
      const glm::mat4& Projection;
      const glm::mat4& View;
      const glm::vec3& ViewPosition;
      const std::vector<DirectionalLight>& DirectionalLights;
      const std::vector<PointLight>& PointLights;
   };


   class PKZL_API MeshRenderer {
   public:
      MeshRenderer(GraphicsContext& gc);
      virtual ~MeshRenderer() = default;

      virtual void BindToGC(GraphicsContext& gc) const;

      virtual void SetTransforms(GraphicsContext& gc, const DrawData& drawData, const glm::mat4& transform) const;
      virtual void SetLighting(GraphicsContext& gc, const DrawData& drawData) const;

      virtual void Draw(GraphicsContext& gc, const Mesh& mesh) const;

   public:
      static std::unique_ptr<MeshRenderer> Create(GraphicsContext& gc, const Model& model);

   protected:
      std::unique_ptr<Pipeline> m_Pipeline;

   private:
      std::unique_ptr<UniformBuffer> m_DirectionalLightBuffer;
      std::unique_ptr<UniformBuffer> m_PointLightBuffer;

      using MESHRENDERERCREATEPROC = MeshRenderer* (__cdecl*)(GraphicsContext*, const Model*);
      inline static MESHRENDERERCREATEPROC CreateMeshRenderer;

      friend class RenderCore;
   };

}
