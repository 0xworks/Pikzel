#pragma once

#include "GraphicsContext.h"
#include "Pikzel/Core/Window.h"

#include <glm/glm.hpp>
#include <memory>

namespace Pikzel {

   struct IRenderCore {
      virtual ~IRenderCore() = default;

      virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

      virtual void SetClearColor(const glm::vec4& color) = 0;
      virtual void Clear() = 0;

      virtual std::unique_ptr<GraphicsContext> CreateGraphicsContext(Window& window) = 0;
   };


   class RenderCore {
   public:

      enum class API {
         None,
         OpenGL
      };

      static void SetAPI(API api);
      static API GetAPI();

      static void Init();

      static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

      static void SetClearColor(const glm::vec4& color);
      static void Clear();

      static std::unique_ptr<GraphicsContext> CreateGraphicsContext(Window& window);

   private:
      static API s_API;
      static std::unique_ptr<IRenderCore> s_RenderCore;
   };

}
