#pragma once

namespace Pikzel {

   class GraphicsContext {
   public:
      virtual ~GraphicsContext() = default;

      virtual void UploadImGuiFonts() = 0;

      virtual void BeginFrame() = 0;
      virtual void EndFrame() = 0;

      virtual void BeginImGuiFrame() = 0;
      virtual void EndImGuiFrame() = 0;

      virtual void SwapBuffers() = 0;
   };

}
