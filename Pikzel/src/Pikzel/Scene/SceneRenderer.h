#pragma once

#include "Pikzel/Renderer/GraphicsContext.h"
#include "Pikzel/Renderer/Pipeline.h"
#include "Pikzel/Scene/Camera.h"
#include "Pikzel/Scene/Scene.h"

namespace Pikzel {

   class PKZL_API SceneRenderer {
   public:

      SceneRenderer(const GraphicsContext& gc);
      virtual ~SceneRenderer() = default;

      void RenderBegin();
      void Render(GraphicsContext& gc, Camera& camera, Scene& scene);
      void RenderEnd();

   private:
      std::unique_ptr<Pipeline> m_Pipeline;
   };

   std::unique_ptr<SceneRenderer> PKZL_API CreateSceneRenderer(const GraphicsContext& gc);

}
