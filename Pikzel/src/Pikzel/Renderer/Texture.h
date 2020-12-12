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
      BGRA8            /* BGRA in sRGB color space. 8 bits per component */,
      R8               /* linear single channel, 8 bits */,
      R32F             /* linear single channel, 32-bit floating point */,
      D32F             /* linear depth component, 32-bit floating point */,
      D24S8            /* 24-bit depth component packed with 8-bit stencil component */,
      D32S8            /* 32-bit depth component, 8-bit stencil component */
   };


   enum class TextureType {
      Texture2D,
      Texture2DArray,
      TextureCube,
      TextureCubeArray
   };


   inline bool IsColorFormat(const TextureFormat format) {
      switch (format) {
         case TextureFormat::RGB8:    return true;
         case TextureFormat::RGBA8:   return true;
         case TextureFormat::SRGB8:   return true;
         case TextureFormat::SRGBA8:  return true;
         case TextureFormat::RGB32F:  return true;
         case TextureFormat::RGBA32F: return true;
         case TextureFormat::BGR8:    return true;
         case TextureFormat::BGRA8:   return true;
         case TextureFormat::R8:      return true;
         case TextureFormat::R32F:    return true;
         case TextureFormat::D32F:    return false;
         case TextureFormat::D24S8:   return false;
         case TextureFormat::D32S8:   return false;
      }
      return false;
   }


   inline bool IsDepthFormat(const TextureFormat format) {
      switch (format) {
         case TextureFormat::RGB8:    return false;
         case TextureFormat::RGBA8:   return false;
         case TextureFormat::SRGB8:   return false;
         case TextureFormat::SRGBA8:  return false;
         case TextureFormat::RGB32F:  return false;
         case TextureFormat::RGBA32F: return false;
         case TextureFormat::BGR8:    return false;
         case TextureFormat::BGRA8:   return false;
         case TextureFormat::R8:      return false;
         case TextureFormat::R32F:    return false;
         case TextureFormat::D32F:    return true;
         case TextureFormat::D24S8:   return true;
         case TextureFormat::D32S8:   return true;
      }
      return false;
   }


   class PKZL_API Texture {
   public:
      virtual ~Texture() = default;

      virtual TextureFormat GetFormat() const = 0;

      virtual TextureType GetType() const = 0;

      virtual uint32_t GetWidth() const = 0;
      virtual uint32_t GetHeight() const = 0;

      virtual uint32_t GetLayers() const = 0;

      virtual void SetData(void* data, const uint32_t size) = 0;

      virtual bool operator==(const Texture& that) = 0;

   public:
      static uint32_t CalculateMipMapLevels(const uint32_t width, const uint32_t height);
      static uint32_t BPP(const TextureFormat format);
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
