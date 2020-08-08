#pragma once

#include "Pikzel/Renderer/RenderCore.h"

namespace Pikzel {

   class OpenGLRenderCore : public RenderCore {
   public:
      OpenGLRenderCore();
      virtual ~OpenGLRenderCore();

      std::unique_ptr<GraphicsContext> CreateGraphicsContext(const Window& window) override;

      virtual void BeginFrame() override;
      virtual void EndFrame() override;

   };

}
