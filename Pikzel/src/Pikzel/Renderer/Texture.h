#pragma once

#include "Pikzel/Core/Core.h"

#include <filesystem>

namespace Pikzel {

   enum class TextureFormat {
      Undefined,
      RGB8,
      RGBA8,
      RGB32F,
      RGBA32F,
      BGRA8
   };


   class PKZL_API Texture {
   public:
      virtual ~Texture() = default;

      virtual TextureFormat GetFormat() const = 0;
      virtual uint32_t GetWidth() const = 0;
      virtual uint32_t GetHeight() const = 0;

      virtual void SetData(void* data, const uint32_t size) = 0;

   public:
      static uint32_t CalculateMipMapLevels(const uint32_t width, const uint32_t height);
      static uint32_t BPP(const TextureFormat format);
   };


   class PKZL_API Texture2D : public Texture {
   public:
      virtual ~Texture2D() = default;

      virtual bool operator==(const Texture2D& that) = 0;
   };


   class PKZL_API TextureCube : public Texture {
   public:
      virtual ~TextureCube() = default;

      virtual bool operator==(const TextureCube& that) = 0;
   };


   inline uint32_t Texture::BPP(const TextureFormat format) {
      switch (format) {
         case TextureFormat::RGB8: return 3;
         case TextureFormat::RGBA8: return 4;
         case TextureFormat::RGB32F: return 12;
         case TextureFormat::RGBA32F: return 16;
         case TextureFormat::BGRA8: return 4;
         default: return 0;
      }
   }


   inline uint32_t Texture::CalculateMipMapLevels(const uint32_t width, const uint32_t height) {
      uint32_t levels = 1;
      while ((width | height) >> levels) {
         ++levels;
      }
      return levels;
   }

}
