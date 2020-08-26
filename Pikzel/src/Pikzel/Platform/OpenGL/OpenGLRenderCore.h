#pragma once

#include "Pikzel/Renderer/RenderCore.h"

namespace Pikzel {

   class OpenGLRenderCore : public IRenderCore {
   public:
      OpenGLRenderCore();
      virtual ~OpenGLRenderCore();

      virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

      virtual void SetClearColor(const glm::vec4& color) override;
      virtual void Clear() override;

      virtual std::unique_ptr<GraphicsContext> CreateGraphicsContext(Window& window) override;

   };


}
