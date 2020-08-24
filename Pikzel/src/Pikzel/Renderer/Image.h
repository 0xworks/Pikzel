#pragma once

namespace Pikzel {

   struct ImageSettings {
      uint32_t Width = 800;
      uint32_t Height = 600;
      // format...?
   };


   class Image {
   public:
      virtual ~Image() = default;

      virtual void* GetImGuiTextureId() const = 0;

   };

}
