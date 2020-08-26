#pragma once

#include <memory>

namespace Pikzel {

   class GraphicsContext {
   public:
      virtual ~GraphicsContext() = default;

      virtual void BeginFrame() = 0;
      virtual void EndFrame() = 0;

      virtual void SwapBuffers() = 0;

   };

}
