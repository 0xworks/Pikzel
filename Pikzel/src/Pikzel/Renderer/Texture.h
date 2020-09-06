#pragma once

namespace Pikzel {

   class Texture2D {
   public:
      virtual ~Texture2D() = default;

      virtual uint32_t GetWidth() const = 0;
      virtual uint32_t GetHeight() const = 0;
      virtual uint32_t GetRendererID() const = 0;

      virtual void SetData(void* data, uint32_t size) = 0;

      virtual bool operator==(const Texture2D& other) const = 0;
   };

}