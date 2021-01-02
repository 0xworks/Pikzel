#include "OpenGLTexture.h"

#include "OpenGLPipeline.h"

#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


namespace Pikzel {

   GLenum TextureFormatToInternalFormat(const TextureFormat format) {
      switch (format) {
         case TextureFormat::RGB8: return GL_RGB8;
         case TextureFormat::RGBA8: return GL_RGBA8;
         case TextureFormat::SRGB8: return GL_SRGB8;
         case TextureFormat::SRGBA8: return GL_SRGB8_ALPHA8;
         case TextureFormat::RGB16F: return GL_RGB16F;
         case TextureFormat::RGBA16F: return GL_RGBA16F;
         case TextureFormat::RGB32F: return GL_RGB32F;
         case TextureFormat::RGBA32F: return GL_RGBA32F;
         case TextureFormat::R8: return GL_R8;
         case TextureFormat::R32F: return GL_R32F;
         case TextureFormat::D32F: return GL_DEPTH_COMPONENT32F;
         case TextureFormat::D24S8: return GL_DEPTH24_STENCIL8;
         case TextureFormat::D32S8: return GL_DEPTH32F_STENCIL8;

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
         case TextureFormat::RGB16F: return GL_RGB;
         case TextureFormat::RGBA16F: return GL_RGBA;
         case TextureFormat::RGB32F: return GL_RGB;
         case TextureFormat::RGBA32F: return GL_RGBA;
         case TextureFormat::R8: return GL_R;
         case TextureFormat::R32F: return GL_R;
         // no need to set depth data yourself, so no depth formats here
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
         case TextureFormat::RGB16F: return GL_HALF_FLOAT;
         case TextureFormat::RGBA16F: return GL_HALF_FLOAT;
         case TextureFormat::RGB32F: return GL_FLOAT;
         case TextureFormat::RGBA32F: return GL_FLOAT;
         case TextureFormat::R8: return GL_UNSIGNED_BYTE;
         case TextureFormat::R32F: return GL_FLOAT;
         // no need to set depth data yourself, so no depth formats here
      }
      PKZL_CORE_ASSERT(false, "Unsupported TextureFormat!");
      return 0;
   }


   GLenum TextureFilterToGLTextureFilter(const TextureFilter filter) {
      switch (filter) {
         case TextureFilter::Nearest: return GL_NEAREST;
         case TextureFilter::NearestMipMapNearest: return GL_NEAREST_MIPMAP_NEAREST;
         case TextureFilter::NearestMipMapLinear: return GL_NEAREST_MIPMAP_LINEAR;
         case TextureFilter::Linear: return GL_LINEAR;
         case TextureFilter::LinearMipMapNearest: return GL_LINEAR_MIPMAP_NEAREST;
         case TextureFilter::LinearMipMapLinear: return GL_LINEAR_MIPMAP_LINEAR;
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


   static stbi_uc* STBILoad(const std::filesystem::path& path, const bool isSRGB, uint32_t* width, uint32_t* height, TextureFormat* format) {
      int iWidth;
      int iHeight;
      int channels;
      stbi_set_flip_vertically_on_load(1);
      stbi_uc* data = nullptr;
      bool isHDR = stbi_is_hdr(path.string().c_str());
      if (isHDR) {
         data = reinterpret_cast<stbi_uc*>(stbi_loadf(path.string().c_str(), &iWidth, &iHeight, &channels, 0));
      } else {
         data = stbi_load(path.string().c_str(), &iWidth, &iHeight, &channels, 0);
      }
      if (!data) {
         throw std::runtime_error {fmt::format("failed to load image '{0}'", path.string())};
      }
      *width = static_cast<uint32_t>(iWidth);
      *height = static_cast<uint32_t>(iHeight);

      if (channels == 4) {
         *format = isHDR ? TextureFormat::RGBA32F : isSRGB? TextureFormat::SRGBA8 : TextureFormat::RGBA8;
      } else if (channels == 3) {
         *format = isHDR ? TextureFormat::RGB32F : isSRGB? TextureFormat::SRGB8 : TextureFormat::RGB8;
      } else if (channels == 1) {
         *format = isHDR ? TextureFormat::R32F : TextureFormat::R8;
      } else {
         throw std::runtime_error {fmt::format("'{0}': Image format not supported!", path.string())};
      }

      return data;
   }


   OpenGLTexture::~OpenGLTexture() {
      glDeleteTextures(1, &m_RendererId);
   }


   TextureFormat OpenGLTexture::GetFormat() const {
      return m_Format;
   }


   uint32_t OpenGLTexture::GetWidth() const {
      return m_Width;
   }


   uint32_t OpenGLTexture::GetHeight() const {
      return m_Height;
   }


   uint32_t OpenGLTexture::GetRendererId() const {
      return m_RendererId;
   }


   void OpenGLTexture::SetTextureParameters(const TextureSettings& settings, const uint32_t mipLevels) {
      static glm::vec4 borderColor = {0.0f, 0.0f, 0.0f, 1.0f};
      TextureFilter minFilter = settings.MinFilter;
      TextureFilter magFilter = settings.MagFilter;
      TextureWrap wrapU = settings.WrapU;
      TextureWrap wrapV = settings.WrapV;
      TextureWrap wrapW = settings.WrapW;

      if (minFilter == TextureFilter::Undefined) {
         minFilter = IsDepthFormat(m_Format) ? TextureFilter::Nearest : mipLevels == 1 ? TextureFilter::Linear : TextureFilter::LinearMipMapLinear;
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


   OpenGLTexture2D::OpenGLTexture2D(const TextureSettings& settings)
   : m_Path {settings.Path}
   {
      uint32_t levels = settings.MIPLevels;
      if (m_Path.empty()) {
         m_Width = settings.Width;
         m_Height = settings.Height;
         m_Format = settings.Format;
         if (levels == 0) {
            levels = CalculateMipMapLevels(m_Width, m_Height);
         }

         glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererId);
         glTextureStorage2D(m_RendererId, levels, TextureFormatToInternalFormat(m_Format), m_Width, m_Height);
      } else {
         stbi_uc* data = STBILoad(m_Path, !IsLinearColorSpace(settings.Format), &m_Width, &m_Height, &m_Format);
         if (levels == 0) {
            levels = CalculateMipMapLevels(m_Width, m_Height);
         }

         glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererId);
         glTextureStorage2D(m_RendererId, levels, TextureFormatToInternalFormat(m_Format), m_Width, m_Height);
         glTextureSubImage2D(m_RendererId, 0, 0, 0, m_Width, m_Height, TextureFormatToDataFormat(m_Format), TextureFormatToDataType(m_Format), data);
         glGenerateTextureMipmap(m_RendererId);
         stbi_image_free(data);
      }
      SetTextureParameters(settings, levels);
   }


   TextureType OpenGLTexture2D::GetType() const {
      return TextureType::Texture2D;
   }


   uint32_t OpenGLTexture2D::GetLayers() const {
      return 1;
   }


   void OpenGLTexture2D::SetData(void* data, const uint32_t size) {
      PKZL_CORE_ASSERT(size == m_Width * m_Height * BPP(m_Format), "Data must be entire texture!");
      glTextureSubImage2D(m_RendererId, 0, 0, 0, m_Width, m_Height, TextureFormatToDataFormat(m_Format), TextureFormatToDataType(m_Format), data);
   }


   OpenGLTexture2DArray::OpenGLTexture2DArray(const TextureSettings& settings)
   : m_Layers {settings.Layers}
   {
      m_Width = settings.Width;
      m_Height = settings.Height;
      m_Format = settings.Format;
      glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_RendererId);
      uint32_t levels = settings.MIPLevels;
      if (levels == 0) {
         levels = CalculateMipMapLevels(m_Width, m_Height);
      }
      glTextureStorage3D(m_RendererId, levels, TextureFormatToInternalFormat(m_Format), m_Width, m_Height, m_Layers);
      SetTextureParameters(settings, levels);
   }


   TextureType OpenGLTexture2DArray::GetType() const {
      return TextureType::Texture2DArray;
   }


   uint32_t OpenGLTexture2DArray::GetLayers() const {
      return m_Layers;
   }


   void OpenGLTexture2DArray::SetData(void* data, const uint32_t size) {
      PKZL_CORE_ASSERT(size == m_Width * m_Height * m_Layers * BPP(m_Format), "Data must be entire texture!");
      glTextureSubImage3D(m_RendererId, 0, 0, 0, 0, m_Width, m_Height, m_Layers, TextureFormatToDataFormat(m_Format), TextureFormatToDataType(m_Format), data);
   }


   OpenGLTextureCube::OpenGLTextureCube(const TextureSettings& settings)
   : m_Path(settings.Path)
   , m_DataFormat {TextureFormat::Undefined}
   {
      uint32_t levels = settings.MIPLevels;
      if (m_Path.empty()) {
         m_Width = settings.Width;
         m_Height = settings.Height;
         m_Format = settings.Format;
         if (levels == 0) {
            levels = CalculateMipMapLevels(m_Width, m_Height);
         }
         AllocateStorage(levels);
      } else {
         m_Format = TextureFormat::RGBA8;
         uint32_t width;
         uint32_t height;
         stbi_uc* data = STBILoad(m_Path, !IsLinearColorSpace(settings.Format), &width, &height, &m_DataFormat);

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
         if (levels == 0) {
            levels = CalculateMipMapLevels(m_Width, m_Height);
         }
         AllocateStorage(levels);
         SetData(data, width * height * BPP(m_DataFormat));
         stbi_image_free(data);
      }
      SetTextureParameters(settings, levels);
   }


   TextureType OpenGLTextureCube::GetType() const {
      return TextureType::TextureCube;
   }


   uint32_t OpenGLTextureCube::GetLayers() const {
      return 6;
   }


   void OpenGLTextureCube::SetData(void* data, uint32_t size) {
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

      std::unique_ptr<OpenGLTexture2D> tex2d = std::make_unique<OpenGLTexture2D>(TextureSettings{.Width = width, .Height = height, .Format = m_DataFormat, .MIPLevels = 1});
      tex2d->SetData(data, size);
      int tonemap = 0;
      if (
         (tex2d->GetFormat() == TextureFormat::RGB32F) ||
         (tex2d->GetFormat() == TextureFormat::RGBA32F)
      ) {
         tonemap = 1;
      }

      std::unique_ptr<OpenGLPipeline> pipeline = std::make_unique<OpenGLPipeline>(PipelineSettings {
         .Shaders = {
            { Pikzel::ShaderType::Compute, shader }
         }
      });

      glUseProgram(pipeline->GetRendererId());
      glUniform1i(glGetUniformLocation(pipeline->GetRendererId(), "constants.tonemap"), tonemap);
      glBindTextureUnit(pipeline->GetSamplerBinding("uTexture"_hs), tex2d->GetRendererId());
      glBindImageTexture(pipeline->GetStorageImageBinding("outCubeMap"_hs), m_RendererId, 0, GL_TRUE, 0, GL_WRITE_ONLY, TextureFormatToInternalFormat(m_Format));
      glDispatchCompute(GetWidth() / 32, GetHeight() / 32, 6);
      glGenerateTextureMipmap(m_RendererId);
   }


   void OpenGLTextureCube::AllocateStorage(const uint32_t mipLevels) {
      if ((GetWidth() % 32)) {
         throw std::runtime_error {"Cube texture size must be a multiple of 32!"};
      }

      glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_RendererId);
      glTextureStorage2D(m_RendererId, mipLevels, TextureFormatToInternalFormat(m_Format), m_Width, m_Height);
   }


   OpenGLTextureCubeArray::OpenGLTextureCubeArray(const TextureSettings& settings)
   : m_Layers {settings.Layers}
   {
      m_Width = settings.Width;
      m_Height = settings.Height;
      m_Format = settings.Format;
      uint32_t levels = settings.MIPLevels;
      if (levels == 0) {
         levels = CalculateMipMapLevels(m_Width, m_Height);
      }

      if ((GetWidth() % 32)) {
         throw std::runtime_error {"Cube texture size must be a multiple of 32!"};
      }

      glCreateTextures(GL_TEXTURE_CUBE_MAP_ARRAY, 1, &m_RendererId);
      glTextureStorage3D(m_RendererId, levels, TextureFormatToInternalFormat(m_Format), m_Width, m_Height, m_Layers * 6);
      SetTextureParameters(settings, levels);
   }


   TextureType OpenGLTextureCubeArray::GetType() const {
      return TextureType::TextureCubeArray;
   }


   uint32_t OpenGLTextureCubeArray::GetLayers() const {
      return m_Layers * 6;
   }


   void OpenGLTextureCubeArray::SetData(void* data, const uint32_t size) {
      PKZL_NOT_IMPLEMENTED;
   }

}
