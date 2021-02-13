#pragma once

#include "Pikzel/Core/Core.h"

#include <filesystem>

namespace Pikzel {

   enum class TextureFormat {
      Undefined        /* determine automatically from supplied data (e.g. image file) */,
      RGB8             /* linear RGB, 8 bits per component */,
      RGBA8            /* linear RGBA, 8 bits per component */,
      SRGB8            /* non-linear sRGB, 8 bits per component */,
      SRGBA8           /* non-linear sRGBA, 8 bits per component (alpha channel linear) */,
      RG16F            /* linear RG, 16-bit floating point components */,
      RGB16F           /* linear RGB, 16-bit floating point components */,
      RGBA16F          /* linear RGBA, 16-bit floating point components */,
      RG32F            /* linear RG, 32-bit floating point components */,
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


   enum class TextureFilter {
      Undefined  /* means guess an appropriate setting from other supplied data */,
      Nearest,
      NearestMipmapNearest,
      NearestMipmapLinear,
      Linear,
      LinearMipmapNearest,
      LinearMipmapLinear
   };


   enum class TextureWrap {
      Undefined  /* means guess an appropriate setting from other supplied data */,
      ClampToEdge,
      ClampToBorder /* border is always opaque black for now */,
      Repeat,
      MirrorRepeat
   };


   inline bool IsColorFormat(const TextureFormat format) {
      switch (format) {
         case TextureFormat::RGB8:    return true;
         case TextureFormat::RGBA8:   return true;
         case TextureFormat::SRGB8:   return true;
         case TextureFormat::SRGBA8:  return true;
         case TextureFormat::RG16F:   return true;
         case TextureFormat::RGB16F:  return true;
         case TextureFormat::RGBA16F: return true;
         case TextureFormat::RG32F:   return true;
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
         case TextureFormat::RG16F:   return false;
         case TextureFormat::RGB16F:  return false;
         case TextureFormat::RGBA16F: return false;
         case TextureFormat::RG32F:   return false;
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


   inline bool IsLinearColorSpace(const TextureFormat format) {
      switch (format) {
         case TextureFormat::RGB8:    return true;
         case TextureFormat::RGBA8:   return true;
         case TextureFormat::SRGB8:   return false;
         case TextureFormat::SRGBA8:  return false;
         case TextureFormat::RG16F:   return true;
         case TextureFormat::RGB16F:  return true;
         case TextureFormat::RGBA16F: return true;
         case TextureFormat::RG32F:   return true;
         case TextureFormat::RGB32F:  return true;
         case TextureFormat::RGBA32F: return true;
         case TextureFormat::BGR8:    return true;
         case TextureFormat::BGRA8:   return true;
         case TextureFormat::R8:      return true;
         case TextureFormat::R32F:    return true;
         case TextureFormat::D32F:    return true;
         case TextureFormat::D24S8:   return true;
         case TextureFormat::D32S8:   return true;
      }
      return false;
   }


   struct PKZL_API TextureSettings {
      TextureType textureType = TextureType::Texture2D;
      std::filesystem::path path;
      uint32_t width = 1;
      uint32_t height = 1;
      uint32_t depth = 1;
      uint32_t layers = 1;
      TextureFormat format = TextureFormat::SRGBA8;
      TextureFilter minFilter = TextureFilter::LinearMipmapLinear;
      TextureFilter magFilter = TextureFilter::Linear;
      TextureWrap wrapU = TextureWrap::Repeat;
      TextureWrap wrapV = TextureWrap::Repeat;
      TextureWrap wrapW = TextureWrap::Repeat;
      uint32_t mipLevels = 0; // 0 = auto calculate
      bool imageStorage = false;  // true = allow writing to this image in shaders (via imagestore(...))
   };


   struct PKZL_API TextureCopySettings {
      int32_t srcX = 0;
      int32_t srcY = 0;
      int32_t srcZ = 0;
      uint32_t srcLayer = 0;
      uint32_t srcMipLevel = 0;

      int32_t dstX = 0;
      int32_t dstY = 0;
      int32_t dstZ = 0;
      uint32_t dstLayer = 0;
      uint32_t dstMipLevel = 0;
      
      uint32_t width = 0;      // 0 means full width
      uint32_t height = 0;     // 0 means full height
      uint32_t depth = 0;      // 0 means full depth
      uint32_t layerCount = 0; // 0 means all layers
                               // nb: can only copy 1 mip level at a time
   };


   class PKZL_API Texture {
   public:
      virtual ~Texture() = default;

      virtual TextureFormat GetFormat() const = 0;

      virtual TextureType GetType() const = 0;

      virtual uint32_t GetWidth() const = 0;
      virtual uint32_t GetHeight() const = 0;

      virtual uint32_t GetLayers() const = 0;

      virtual uint32_t GetMIPLevels() const = 0;

      virtual void SetData(void* data, const uint32_t size) = 0;

      // See default TextureCopySettings.
      // By default CopyFrom() will copy the full extent and all array layers of srcTexture, mipLevel 0 into self, mipLevel 0
      // To copy all miplevels you need multiple calls to CopyFrom()
      // The srcTexture must be the same type and format as self.
      // self must have dimensions that are big enough to accommodate the extent of srcTexture that is being copied.
      // No resizing is done - this is a straight copy.
      virtual void CopyFrom(const Texture& srcTexture, const TextureCopySettings& settings = {}) = 0;

      // Textures that are created with .ImageStorage = true need to be Commit()'d before they are able to
      // be used in shaders. (e.g. in Vulkan, this transitions the image to shader_read_only)
      // Mipmap levels can optionally also be automatically generated at this point.
      virtual void Commit(const bool generateMipmap = true) = 0;

      virtual bool operator==(const Texture& that) = 0;

   public:
      static uint32_t CalculateMipmapLevels(const uint32_t width, const uint32_t height);
      static uint32_t BPP(const TextureFormat format);
   };


   inline uint32_t Texture::BPP(const TextureFormat format) {
      switch (format) {
         case TextureFormat::R8:      return 1;
         case TextureFormat::R32F:    return 4;
         case TextureFormat::RG16F:   return 4;
         case TextureFormat::RG32F:   return 8;
         case TextureFormat::RGB8:    return 3;
         case TextureFormat::RGBA8:   return 4;
         case TextureFormat::SRGB8:   return 3;
         case TextureFormat::SRGBA8:  return 4;
         case TextureFormat::RGB16F:  return 6;
         case TextureFormat::RGBA16F: return 8;
         case TextureFormat::RGB32F:  return 12;
         case TextureFormat::RGBA32F: return 16;
         case TextureFormat::BGR8:    return 3; // warning: with Vulkan back-end this format is actually 4 bytes
         case TextureFormat::BGRA8:   return 4;
         default: PKZL_CORE_ASSERT(false, "Unknown texture format!"); return 0;
      }
   }


   inline uint32_t Texture::CalculateMipmapLevels(const uint32_t width, const uint32_t height) {
      uint32_t levels = 1;
      while ((width | height) >> levels) {
         ++levels;
      }
      return levels;
   }

}
