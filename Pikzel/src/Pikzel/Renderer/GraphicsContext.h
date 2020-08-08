#pragma once

namespace Pikzel {

   class GraphicsContext {
   public:
      virtual ~GraphicsContext() = default;

      virtual void SwapBuffers() = 0;
   };

}
