#pragma once

#include "Framebuffer.h"
#include "GraphicsContext.h"
#include "Pikzel/Scene/Light.h"
#include "Pikzel/Scene/Mesh.h"
#include "Pikzel/Scene/Model.h"

#include <glm/glm.hpp>

#include <memory>

namespace Pikzel {

   class PKZL_API MeshRenderer {
   public:
      MeshRenderer(GraphicsContext& gc);
      virtual ~MeshRenderer() = default;

      virtual void BindToGC(GraphicsContext& gc) const;
      virtual void Draw(GraphicsContext& gc, const Mesh& mesh) const;

   public:
      static std::unique_ptr<MeshRenderer> Create(GraphicsContext& gc, const Model& model);

   protected:
      std::unique_ptr<Pipeline> m_Pipeline;

   private:
      struct Matrices {
         glm::mat4 vp;
         glm::mat4 lightSpace;
         glm::vec3 viewPos;
      };

      using MESHRENDERERCREATEPROC = MeshRenderer* (__cdecl*)(GraphicsContext*, const Model*);
      inline static MESHRENDERERCREATEPROC CreateMeshRenderer;

      friend class RenderCore;
   };

}
