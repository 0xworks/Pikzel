#pragma once

#include "Pikzel/Core/Core.h"

#include <filesystem>

namespace Pikzel {

   enum class TextureFormat {
      Undefined,
      RGB8             /* linear RGB, 8 bits per component */,
      RGBA8            /* linear RGBA, 8 bits per component */,
      SRGB8            /* non-linear sRGB, 8 bits per component */,
      SRGBA8           /* non-linear sRGBA, 8 bits per component (alpha channel linear) */,
      RGB32F           /* linear RGB, 32-bit floating point components */,
      RGBA32F          /* linear RGBA, 32-bit floating point components */,
      BGR8             /* BGR in sRGB color space, 8 bits per component (except: on Vulkan this one is B10 G11 R11, for 32-bit texels)*/,
      BGRA8            /* BGRA in sRGB color space. 8 bits per component */
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
         case TextureFormat::SRGB8: return 3;
         case TextureFormat::SRGBA8: return 4;
         case TextureFormat::RGB32F: return 12;
         case TextureFormat::RGBA32F: return 16;
         case TextureFormat::BGR8: return 3; // warning: with Vulkan back-end this format is actually 4 bytes
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
