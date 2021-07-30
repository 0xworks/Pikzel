#include "OpenGLTexture.h"

#include "OpenGLComputeContext.h"
#include "OpenGLPipeline.h"

#include <glm/gtc/type_ptr.hpp>

namespace Pikzel {

   GLenum TextureTypeToGLTarget(const TextureType type) {
      switch (type) {
         case TextureType::Texture2D:        return GL_TEXTURE_2D;
         case TextureType::Texture2DArray:   return GL_TEXTURE_2D_ARRAY;
         case TextureType::TextureCube:      return GL_TEXTURE_CUBE_MAP;
         case TextureType::TextureCubeArray: return GL_TEXTURE_CUBE_MAP_ARRAY;
      }
      PKZL_CORE_ASSERT(false, "Unsupported TextureType!");
      return 0;
   }


   GLenum TextureFormatToInternalFormat(const TextureFormat format) {
      switch (format) {
         case TextureFormat::RGB8:      return GL_RGB8;
         case TextureFormat::RGBA8:     return GL_RGBA8;
         case TextureFormat::SRGB8:     return GL_SRGB8;
         case TextureFormat::SRGBA8:    return GL_SRGB8_ALPHA8;
         case TextureFormat::RG16F:     return GL_RG16F;
         case TextureFormat::RGB16F:    return GL_RGB16F;
         case TextureFormat::RGBA16F:   return GL_RGBA16F;
         case TextureFormat::RG32F:     return GL_RG32F;
         case TextureFormat::RGB32F:    return GL_RGB32F;
         case TextureFormat::RGBA32F:   return GL_RGBA32F;
         case TextureFormat::R8:        return GL_R8;
         case TextureFormat::R32F:      return GL_R32F;
         case TextureFormat::D32F:      return GL_DEPTH_COMPONENT32F;
         case TextureFormat::D24S8:     return GL_DEPTH24_STENCIL8;
         case TextureFormat::D32S8:     return GL_DEPTH32F_STENCIL8;
         case TextureFormat::DXT1RGBA:  return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
         case TextureFormat::DXT1SRGBA: return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT;
         case TextureFormat::DXT3RGBA:  return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
         case TextureFormat::DXT3SRGBA: return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT;
         case TextureFormat::DXT5RGBA:  return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
         case TextureFormat::DXT5SRGBA: return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
         case TextureFormat::RGTC1R:    return GL_COMPRESSED_RED_RGTC1;
         case TextureFormat::RGTC1SR:   return GL_COMPRESSED_SIGNED_RED_RGTC1;
         case TextureFormat::RGTC2RG:   return GL_COMPRESSED_RG_RGTC2;
         case TextureFormat::RGTC2SRG:  return GL_COMPRESSED_SIGNED_RG_RGTC2;
      }
      PKZL_CORE_ASSERT(false, "Unsupported TextureFormat!");
      return 0;
   }


   GLenum TextureFormatToDataFormat(const TextureFormat format) {
      switch (format) {
         case TextureFormat::RGB8: return GL_RGB;
         case TextureFormat::RGBA8: return GL_RGBA;
         case TextureFormat::SRGB8: return GL_RGB;
         case TextureFormat::SRGBA8: return GL_RGBA;
         case TextureFormat::RG16F: return GL_RG;
         case TextureFormat::RGB16F: return GL_RGB;
         case TextureFormat::RGBA16F: return GL_RGBA;
         case TextureFormat::RG32F: return GL_RG;
         case TextureFormat::RGB32F: return GL_RGB;
         case TextureFormat::RGBA32F: return GL_RGBA;
         case TextureFormat::R8: return GL_RED;
         case TextureFormat::R32F: return GL_RED;
            // no need (yet) to set depth data yourself, so no depth formats here
            // no need (yet) to set compressed texture data yourself, so no compressed formats here
      }
      PKZL_CORE_ASSERT(false, "Unsupported TextureFormat!");
      return 0;
   }


   GLenum TextureFormatToDataType(const TextureFormat format) {
      switch (format) {
         case TextureFormat::RGB8: return GL_UNSIGNED_BYTE;
         case TextureFormat::RGBA8: return GL_UNSIGNED_BYTE;
         case TextureFormat::SRGB8: return GL_UNSIGNED_BYTE;
         case TextureFormat::SRGBA8: return GL_UNSIGNED_BYTE;
         case TextureFormat::RG16F: return GL_HALF_FLOAT;
         case TextureFormat::RGB16F: return GL_HALF_FLOAT;
         case TextureFormat::RGBA16F: return GL_HALF_FLOAT;
         case TextureFormat::RG32F: return GL_FLOAT;
         case TextureFormat::RGB32F: return GL_FLOAT;
         case TextureFormat::RGBA32F: return GL_FLOAT;
         case TextureFormat::R8: return GL_UNSIGNED_BYTE;
         case TextureFormat::R32F: return GL_FLOAT;
            // no need to set depth data yourself, so no depth formats here
            // no need (yet) to set compressed texture data yourself, so no compressed formats here
      }
      PKZL_CORE_ASSERT(false, "Unsupported TextureFormat!");
      return 0;
   }


   GLenum TextureFilterToGLTextureFilter(const TextureFilter filter) {
      switch (filter) {
         case TextureFilter::Nearest: return GL_NEAREST;
         case TextureFilter::NearestMipmapNearest: return GL_NEAREST_MIPMAP_NEAREST;
         case TextureFilter::NearestMipmapLinear: return GL_NEAREST_MIPMAP_LINEAR;
         case TextureFilter::Linear: return GL_LINEAR;
         case TextureFilter::LinearMipmapNearest: return GL_LINEAR_MIPMAP_NEAREST;
         case TextureFilter::LinearMipmapLinear: return GL_LINEAR_MIPMAP_LINEAR;
      }
      PKZL_CORE_ASSERT(false, "Unsupported TextureFilter!");
      return GL_LINEAR;
   }


   GLenum TextureWrapToGLTextureWrap(const TextureWrap wrap) {
      switch (wrap) {
         case TextureWrap::ClampToEdge: return GL_CLAMP_TO_EDGE;
         case TextureWrap::ClampToBorder: return GL_CLAMP_TO_BORDER;
         case TextureWrap::Repeat: return GL_REPEAT;
         case TextureWrap::MirrorRepeat: return GL_MIRRORED_REPEAT;
      }
      PKZL_CORE_ASSERT(false, "Unsupported TextureWrap!");
      return GL_CLAMP_TO_EDGE;
   }


   OpenGLTexture::~OpenGLTexture() {
      glDeleteTextures(1, &m_RendererId);
   }


   void OpenGLTexture::Init(const TextureSettings& settings) {
      glCreateTextures(TextureTypeToGLTarget(GetType()), 1, &m_RendererId);
      m_Path = settings.path;
      m_MIPLevels = settings.mipLevels;

      if (m_Path.empty()) {
         m_Width = settings.width;
         m_Height = settings.height;
         SetDepth(settings.depth);
         SetLayers(settings.layers);
         m_Format = settings.format;
         if (m_MIPLevels == 0) {
            m_MIPLevels = CalculateMipmapLevels(m_Width, m_Height);
         }
         GLTextureStorage();
      } else {
         TextureLoader loader{ m_Path };
         if (!loader.IsLoaded()) {
            throw std::runtime_error{std::format("failed to load image '{}'", m_Path.string())};
         }
         m_Width = loader.GetWidth();
         m_Height = loader.GetHeight();
         SetDepth(loader.GetDepth());
         SetLayers(loader.GetLayers());
         m_Format = loader.GetFormat();

         if (
            (
               (GetType() == TextureType::TextureCube) ||
               (GetType() == TextureType::TextureCubeArray)
            ) &&
            (loader.GetDepth() == 1)
         ) {
            // work out storage...
            m_Format = TextureFormat::RGBA16F;
            uint32_t width = loader.GetWidth();
            uint32_t height = loader.GetHeight();

            m_DataFormat = loader.GetFormat();
            if (!IsLinearColorSpace(settings.format)) {
               if (m_DataFormat == TextureFormat::RGBA8) {
                  m_DataFormat = TextureFormat::SRGBA8;
               } else if (m_DataFormat == TextureFormat::RGB8) {
                  m_DataFormat = TextureFormat::SRGB8;
               }
            }

            // guess whether the data is the 6-faces of a cube, or whether it's equirectangular
            // width is twice the height -> equirectangular (probably)
            // width is 4/3 the height -> 6 faces of a cube (probably)
            if (width / 2 == height) {
               m_Width = height;
               m_Height = m_Width;
            } else {
               m_Width = width / 4;
               m_Height = m_Width;
            }

            if (loader.GetMIPLevels() > 1) {
               PKZL_CORE_LOG_WARN("Image file has {0} mip levels.  Only the base mip level will be used for 2D texture -> cubemap transformation!", loader.GetMIPLevels());
            }
            if (m_MIPLevels == 0) {
               m_MIPLevels = CalculateMipmapLevels(m_Width, m_Height);
            }

            GLTextureStorage();
            const auto [data, size] = loader.GetData(0, 0, 0);
            SetData(data, size);
         } else {
            // Depending on image file format, the loader may not be able to tell whether the image was in
            // sRGB colorspace.  Adjust format via hint in settings
            if (!IsLinearColorSpace(settings.format)) {
               if (m_Format == TextureFormat::RGBA8) {
                  m_Format = TextureFormat::SRGBA8;
               } else if (m_Format == TextureFormat::RGB8) {
                  m_Format = TextureFormat::SRGB8;
               }
            }

            if (m_MIPLevels == 0) {
               m_MIPLevels = std::max(CalculateMipmapLevels(m_Width, m_Height), loader.GetMIPLevels());
            }

            GLTextureStorage();
            for (uint32_t layer = 0; layer < m_Layers; ++layer) {

               // note: loader.GetDepth() is the actual number of slices in the data
               //       this->GetDepth() could be different (e.g. cubemaps will have loader.GetDepth() = 6, and this->GetDepth() = 1)
               for (uint32_t slice = 0; slice < loader.GetDepth(); ++slice) {
                  for (int mipLevel = 0; mipLevel < std::min(m_MIPLevels, loader.GetMIPLevels()); ++mipLevel) {
                     const auto [data, size] = loader.GetData(layer, slice, mipLevel);
                     if (loader.IsCompressed()) {
                        GLCompressedTextureSubImage(layer, slice, mipLevel, 0, 0, size, data);
                     } else {
                        GLTextureSubImage(layer, slice, mipLevel, 0, 0, data);
                     }
                  }
               }
            }
            uint32_t baseMipLevel = std::min(m_MIPLevels, loader.GetMIPLevels()) - 1;
            Commit(baseMipLevel);
         }
      }
      SetTextureParameters(settings);
   }


   TextureFormat OpenGLTexture::GetFormat() const {
      return m_Format;
   }


   TextureType OpenGLTexture::GetType() const {
      return TextureType::Undefined;
   }

   uint32_t OpenGLTexture::GetWidth() const {
      return m_Width;
   }


   uint32_t OpenGLTexture::GetHeight() const {
      return m_Height;
   }


   uint32_t OpenGLTexture::GetDepth() const {
      return m_Depth;
   }


   uint32_t OpenGLTexture::GetLayers() const {
      return m_Layers;
   }


   uint32_t OpenGLTexture::GetMIPLevels() const {
      return m_MIPLevels;
   }


   void OpenGLTexture::CopyFrom(const Texture& srcTexture, const TextureCopySettings& settings) {
      if (GetType() != srcTexture.GetType()) {
         throw std::logic_error{std::format("Texture::CopyFrom() source and destination textures are not the same type!")};
      }
      if (GetFormat() != srcTexture.GetFormat()) {
         throw std::logic_error{std::format("Texture::CopyFrom() source and destination textures are not the same format!")};
      }
      if (settings.srcMipLevel >= srcTexture.GetMIPLevels()) {
         throw std::logic_error{std::format("Texture::CopyFrom() source texture does not have requested mip level!")};
      }
      if (settings.dstMipLevel >= GetMIPLevels()) {
         throw std::logic_error{std::format("Texture::CopyFrom() destination texture does not have requested mip level!")};
      }

      uint32_t layerCount = settings.layerCount == 0 ? srcTexture.GetLayers() : settings.layerCount;
      if (settings.srcLayer + layerCount > srcTexture.GetLayers()) {
         throw std::logic_error{std::format("Texture::CopyFrom() source texture does not have requested layer!")};
      }
      if (settings.dstLayer + layerCount > GetLayers()) {
         throw std::logic_error{std::format("Texture::CopyFrom() destination texture does not have requested layer!")};
      }

      uint32_t width = settings.width == 0 ? srcTexture.GetWidth() / (1 << settings.srcMipLevel) : settings.width;
      uint32_t height = settings.height == 0 ? srcTexture.GetHeight() / (1 << settings.srcMipLevel) : settings.height;
      uint32_t depth = settings.depth == 0 ? srcTexture.GetDepth() : settings.depth;

      if (width + settings.dstX > (GetWidth() >> settings.dstMipLevel)) {
         throw std::logic_error{std::format("Texture::CopyFrom() requested width (+ destination xOffset) is larger than destination texture width!")};
      }
      if (height + settings.dstY > (GetHeight() >> settings.dstMipLevel)) {
         throw std::logic_error{std::format("Texture::CopyFrom() requested height (+ destination yOffset) is larger than destination texture height!")};
      }
      if (depth + settings.dstZ > GetDepth()) {
         throw std::logic_error{std::format("Texture::CopyFrom() requested depth (+ destination zOffset) is larger than destination texture depth!")};
      }

      uint32_t depthFactor = 1;
      if (
         (GetType() == TextureType::TextureCube) ||
         (GetType() == TextureType::TextureCubeArray)
      ) {
         depthFactor = 6;
      }

      for (uint32_t layer = 0; layer < layerCount; ++layer) {
         glCopyImageSubData(
            static_cast<const OpenGLTexture&>(srcTexture).GetRendererId(),
            TextureTypeToGLTarget(GetType()),
            settings.srcMipLevel, settings.srcX, settings.srcY, ((settings.srcLayer + layer) * (srcTexture.GetDepth() * depthFactor)) + settings.srcZ,
            GetRendererId(),
            TextureTypeToGLTarget(GetType()),
            settings.dstMipLevel, settings.dstX, settings.dstY, ((settings.dstLayer + layer) * (GetDepth() * depthFactor)) + settings.dstZ,
            width, height, depth * depthFactor
         );
      }

   }


   void OpenGLTexture::Commit(const uint32_t baseMipLevel) {
      if (baseMipLevel < GetMIPLevels() - 1) {
         GLenum target = TextureTypeToGLTarget(GetType());
         glTextureParameteri(m_RendererId, GL_TEXTURE_BASE_LEVEL, baseMipLevel);
         glGenerateTextureMipmap(m_RendererId);
         glTextureParameteri(m_RendererId, GL_TEXTURE_BASE_LEVEL, 0);
      }
   }


   uint32_t OpenGLTexture::GetRendererId() const {
      return m_RendererId;
   }


   void OpenGLTexture::SetTextureParameters(const TextureSettings& settings) {
      static glm::vec4 borderColor = {0.0f, 0.0f, 0.0f, 1.0f};
      TextureFilter minFilter = settings.minFilter;
      TextureFilter magFilter = settings.magFilter;
      TextureWrap wrapU = settings.wrapU;
      TextureWrap wrapV = settings.wrapV;
      TextureWrap wrapW = settings.wrapW;

      if (minFilter == TextureFilter::Undefined) {
         minFilter = IsDepthFormat(m_Format) ? TextureFilter::Nearest : m_MIPLevels == 1 ? TextureFilter::Linear : TextureFilter::LinearMipmapLinear;
      }
      if (magFilter == TextureFilter::Undefined) {
         magFilter = IsDepthFormat(m_Format) ? TextureFilter::Nearest : TextureFilter::Linear;
      }

      if (wrapU == TextureWrap::Undefined) {
         wrapU = IsDepthFormat(m_Format) ? TextureWrap::ClampToEdge : TextureWrap::Repeat;
      }
      if (wrapV == TextureWrap::Undefined) {
         wrapV = IsDepthFormat(m_Format) ? TextureWrap::ClampToEdge : TextureWrap::Repeat;
      }
      if (wrapW == TextureWrap::Undefined) {
         wrapW = IsDepthFormat(m_Format) ? TextureWrap::ClampToEdge : TextureWrap::Repeat;
      }

      GLenum target = TextureTypeToGLTarget(GetType());
      glTextureParameteri(m_RendererId, GL_TEXTURE_MIN_FILTER, TextureFilterToGLTextureFilter(minFilter));
      glTextureParameteri(m_RendererId, GL_TEXTURE_MAG_FILTER, TextureFilterToGLTextureFilter(magFilter));
      glTextureParameteri(m_RendererId, GL_TEXTURE_WRAP_S, TextureWrapToGLTextureWrap(wrapU));
      glTextureParameteri(m_RendererId, GL_TEXTURE_WRAP_T, TextureWrapToGLTextureWrap(wrapV));
      glTextureParameteri(m_RendererId, GL_TEXTURE_WRAP_R, TextureWrapToGLTextureWrap(wrapW));
      glTextureParameterfv(m_RendererId, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(borderColor));
   }


   bool OpenGLTexture::operator==(const Texture& that) {
      return m_RendererId = static_cast<const OpenGLTexture&>(that).m_RendererId;
   }


   OpenGLTexture2D::OpenGLTexture2D(const TextureSettings& settings) {
      Init(settings);
   }


   TextureType OpenGLTexture2D::GetType() const {
      return TextureType::Texture2D;
   }


   void OpenGLTexture2D::SetData(const void* data, const uint32_t size) {
      PKZL_CORE_ASSERT(size == m_Width * m_Height * BPP(m_Format), "Data must be entire texture!");
      GLTextureSubImage(0, 0, 0, 0, 0, data);
      Commit(0);
   }


   void OpenGLTexture2D::SetDepth(const uint32_t depth) {
      PKZL_CORE_ASSERT(depth == 1, "Depth for 2D texture should be 1!");
      m_Depth = 1;
   }


   void OpenGLTexture2D::SetLayers(const uint32_t layers) {
      PKZL_CORE_ASSERT(layers == 1, "Layers for 2D texture should be 1!");
      m_Layers = 1;
   }


   void OpenGLTexture2D::GLTextureStorage() {
      glTextureStorage2D(m_RendererId, m_MIPLevels, TextureFormatToInternalFormat(m_Format), m_Width, m_Height);
   }


   void OpenGLTexture2D::GLCompressedTextureSubImage(const uint32_t /*layer*/, const uint32_t /*slice*/, const int mipLevel, const int xOffset, const int yOffset, const uint32_t size, const void* data) {
      const uint32_t width = m_Width >> mipLevel;
      const uint32_t height = m_Height >> mipLevel;
      glCompressedTextureSubImage2D(m_RendererId, mipLevel, xOffset, yOffset, width, height, TextureFormatToInternalFormat(m_Format), size, data);
   }


   void OpenGLTexture2D::GLTextureSubImage(const uint32_t /*layer*/, const uint32_t /*slice*/, const int mipLevel, const int xOffset, const int yOffset, const void* data) {
      const uint32_t width = m_Width >> mipLevel;
      const uint32_t height = m_Height >> mipLevel;
      glTextureSubImage2D(m_RendererId, mipLevel, xOffset, yOffset, width, height, TextureFormatToDataFormat(m_Format), TextureFormatToDataType(m_Format), data);
   }


   OpenGLTexture2DArray::OpenGLTexture2DArray(const TextureSettings& settings) {
      Init(settings);
   }


   TextureType OpenGLTexture2DArray::GetType() const {
      return TextureType::Texture2DArray;
   }


   void OpenGLTexture2DArray::SetData(const void* data, const uint32_t size) {
      PKZL_CORE_ASSERT(size == m_Width * m_Height * m_Layers * BPP(m_Format), "Data must be entire texture!");
      glTextureSubImage3D(m_RendererId, 0, 0, 0, 0, m_Width, m_Height, m_Layers, TextureFormatToDataFormat(m_Format), TextureFormatToDataType(m_Format), data);
      Commit(0);
   }


   void OpenGLTexture2DArray::SetDepth(const uint32_t depth) {
      PKZL_CORE_ASSERT(depth == 1, "Depth for 2D texture array should be 1!");
      m_Depth = 1;
   }


   void OpenGLTexture2DArray::SetLayers(const uint32_t layers) {
      m_Layers = layers;
   }


   void OpenGLTexture2DArray::GLTextureStorage() {
      glTextureStorage3D(m_RendererId, m_MIPLevels, TextureFormatToInternalFormat(m_Format), m_Width, m_Height, m_Layers);
   }


   void OpenGLTexture2DArray::GLCompressedTextureSubImage(const uint32_t layer, const uint32_t slice, const int mipLevel, const int xOffset, const int yOffset, const uint32_t size, const void* data) {
      const uint32_t width = m_Width >> mipLevel;
      const uint32_t height = m_Height >> mipLevel;
      glCompressedTextureSubImage3D(m_RendererId, mipLevel, xOffset, yOffset, slice, width, height, 1, TextureFormatToInternalFormat(m_Format), size, data);
   }


   void OpenGLTexture2DArray::GLTextureSubImage(const uint32_t layer, const uint32_t slice, const int mipLevel, const int xOffset, const int yOffset, const void* data) {
      const uint32_t width = m_Width >> mipLevel;
      const uint32_t height = m_Height >> mipLevel;
      glTextureSubImage3D(m_RendererId, mipLevel, xOffset, yOffset, slice, width, height, 1, TextureFormatToDataFormat(m_Format), TextureFormatToDataType(m_Format), data);
   }


   OpenGLTextureCube::OpenGLTextureCube(const TextureSettings& settings) {
      Init(settings);
   }


   TextureType OpenGLTextureCube::GetType() const {
      return TextureType::TextureCube;
   }


   void OpenGLTextureCube::SetData(const void* data, uint32_t size) {
      uint32_t width = m_Width;
      uint32_t height = m_Height;
      const char* shader = nullptr;
      if (size == (width * 2 * height * BPP(m_DataFormat))) {
         width *= 2;
         shader = "Renderer/EquirectangularToCubeMap.comp.spv";
      } else if (size == width * 4 * height * 3 * BPP(m_DataFormat)) {
         width *= 4;
         height *= 3;
         shader = "Renderer/SixFacesToCubeMap.comp.spv";
      } else {
         throw std::runtime_error("Data must be entire texture!");
      }

      std::unique_ptr<Texture> tex2d = std::make_unique<OpenGLTexture2D>(TextureSettings{.width = width, .height = height, .format = m_DataFormat, .mipLevels = 1});
      tex2d->SetData(data, size);

      std::unique_ptr<ComputeContext> compute = std::make_unique<OpenGLComputeContext>();

      std::unique_ptr<Pipeline> pipeline = compute->CreatePipeline({
         .shaders = {
            { Pikzel::ShaderType::Compute, shader }
         }
      });

      compute->Begin();
      compute->Bind(*pipeline);
      compute->Bind("uTexture"_hs, *tex2d);
      compute->Bind("outCubeMap"_hs, *this);
      compute->Dispatch(GetWidth() / 32, GetHeight() / 32, 6);
      compute->End();
      Commit(0);
   }


   void OpenGLTextureCube::SetDepth(const uint32_t depth) {
      PKZL_CORE_ASSERT(depth == 1 || depth == 6, "Source depth for cube texture should be 1!");  // nb: 6 is also acceptable, but we'll just ignore that and set 1 anyway.
      m_Depth = 1;
   }


   void OpenGLTextureCube::SetLayers(const uint32_t layers) {
      PKZL_CORE_ASSERT(layers == 1, "Layers for cube texture should be 1!");
      m_Layers = 1;
   }


   void OpenGLTextureCube::GLTextureStorage() {
      PKZL_CORE_ASSERT(m_Width == m_Height, "Cube texture width should equal height!");
      PKZL_CORE_ASSERT(m_Width % 32 == 0, "Cube texture size must be a multiple of 32!"); // because when we use compute shader to convert a 2D image into cubemap, we dispatch blocks of 32x32
      glTextureStorage2D(m_RendererId, m_MIPLevels, TextureFormatToInternalFormat(m_Format), m_Width, m_Height);
   }


   void OpenGLTextureCube::GLCompressedTextureSubImage(const uint32_t /*layer*/, const uint32_t slice, const int mipLevel, const int xOffset, const int yOffset, const uint32_t size, const void* data) {
      const uint32_t width = m_Width >> mipLevel;
      const uint32_t height = m_Height >> mipLevel;
      glCompressedTextureSubImage3D(m_RendererId, mipLevel, xOffset, yOffset, slice, width, height, 1, TextureFormatToInternalFormat(m_Format), size, data);
   }


   void OpenGLTextureCube::GLTextureSubImage(const uint32_t /*layer*/, const uint32_t slice, const int mipLevel, const int xOffset, const int yOffset, const void* data) {
      const uint32_t width = m_Width >> mipLevel;
      const uint32_t height = m_Height >> mipLevel;
      glTextureSubImage3D(m_RendererId, mipLevel, xOffset, yOffset, slice, width, height, 1, TextureFormatToDataFormat(m_Format), TextureFormatToDataType(m_Format), data);
   }


   OpenGLTextureCubeArray::OpenGLTextureCubeArray(const TextureSettings& settings) {
      Init(settings);
   }


   TextureType OpenGLTextureCubeArray::GetType() const {
      return TextureType::TextureCubeArray;
   }


   void OpenGLTextureCubeArray::SetData(const void* data, const uint32_t size) {
      PKZL_NOT_IMPLEMENTED;
   }


   void OpenGLTextureCubeArray::SetDepth(const uint32_t depth) {
      PKZL_CORE_ASSERT(depth == 1 || depth == 6, "Depth for cube texture array should be 1!"); // 6 is also acceptable, but we'll just ignore that and set 1 anyway
      m_Depth = 1;
   }


   void OpenGLTextureCubeArray::SetLayers(const uint32_t layers) {
      m_Layers = layers;
   }


   void OpenGLTextureCubeArray::GLTextureStorage() {
      glTextureStorage3D(m_RendererId, m_MIPLevels, TextureFormatToInternalFormat(m_Format), m_Width, m_Height, m_Layers * m_Depth * 6);
   }


   void OpenGLTextureCubeArray::GLCompressedTextureSubImage(const uint32_t layer, const uint32_t slice, const int mipLevel, const int xOffset, const int yOffset, const uint32_t size, const void* data) {
      const uint32_t width = m_Width >> mipLevel;
      const uint32_t height = m_Height >> mipLevel;
      glCompressedTextureSubImage3D(m_RendererId, mipLevel, xOffset, yOffset, (layer * 6) + slice, width, height, 1, TextureFormatToInternalFormat(m_Format), size, data);
   }


   void OpenGLTextureCubeArray::GLTextureSubImage(const uint32_t layer, const uint32_t slice, const int mipLevel, const int xOffset, const int yOffset, const void* data) {
      const uint32_t width = m_Width >> mipLevel;
      const uint32_t height = m_Height >> mipLevel;
      glTextureSubImage3D(m_RendererId, mipLevel, xOffset, yOffset, (layer * 6) + slice, width, height, 1, TextureFormatToDataFormat(m_Format), TextureFormatToDataType(m_Format), data);
   }

}
