#include "OpenGLTexture.h"

#include "OpenGLPipeline.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Pikzel {

   static GLenum TextureFormatToInternalFormat(const TextureFormat format) {
      switch (format) {
         case TextureFormat::RGB8: return GL_RGB8;
         case TextureFormat::RGBA8: return GL_RGBA8;
         case TextureFormat::RGB32F: return GL_RGB32F;
         case TextureFormat::RGBA32F: return GL_RGBA32F;
      }

      PKZL_CORE_ASSERT(false, "Unknown TextureFormat!");
      return 0;
   }


   static GLenum TextureFormatToDataFormat(const TextureFormat format) {
      switch (format) {
         case TextureFormat::RGB8: return GL_RGB;
         case TextureFormat::RGBA8: return GL_RGBA;
         case TextureFormat::RGB32F: return GL_RGB;
         case TextureFormat::RGBA32F: return GL_RGBA;
      }

      PKZL_CORE_ASSERT(false, "Unknown TextureFormat!");
      return 0;
   }


   static GLenum TextureFormatToDataType(const TextureFormat format) {
      switch (format) {
         case TextureFormat::RGB8: return GL_UNSIGNED_BYTE;
         case TextureFormat::RGBA8: return GL_UNSIGNED_BYTE;
         case TextureFormat::RGB32F: return GL_FLOAT;
         case TextureFormat::RGBA32F: return GL_FLOAT;
      }

      PKZL_CORE_ASSERT(false, "Unknown TextureFormat!");
      return 0;
   }


   static stbi_uc* STBILoad(const std::filesystem::path& path, uint32_t& width, uint32_t& height, TextureFormat& format) {
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
      width = static_cast<uint32_t>(iWidth);
      height = static_cast<uint32_t>(iHeight);

      if (channels == 4) {
         format = isHDR ? TextureFormat::RGBA32F : TextureFormat::RGBA8;
      } else if (channels == 3) {
         format = isHDR ? TextureFormat::RGB32F : TextureFormat::RGB8;
      } else {
         throw std::runtime_error {fmt::format("'{0}': Image format not supported!", path.string())};
      }

      return data;
   }


   OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height, TextureFormat format, const uint32_t mipLevels)
   : m_Format {format}
   , m_Width {width}
   , m_Height {height}
   {
      glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererId);
      glTextureStorage2D(m_RendererId, mipLevels, TextureFormatToInternalFormat(m_Format), m_Width, m_Height);

      glTextureParameteri(m_RendererId, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      glTextureParameteri(m_RendererId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      glTextureParameteri(m_RendererId, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTextureParameteri(m_RendererId, GL_TEXTURE_WRAP_T, GL_REPEAT);
   }


   OpenGLTexture2D::OpenGLTexture2D(const std::filesystem::path& path)
   : m_Format {TextureFormat::Undefined}
   , m_Path {path}
   {
      stbi_uc* data = STBILoad(path, m_Width, m_Height, m_Format);
      uint32_t levels = CalculateMipMapLevels(m_Width, m_Height);
      glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererId);
      glTextureStorage2D(m_RendererId, levels, TextureFormatToInternalFormat(m_Format), m_Width, m_Height);

      glTextureParameteri(m_RendererId, GL_TEXTURE_MIN_FILTER, levels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
      glTextureParameteri(m_RendererId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      glTextureParameteri(m_RendererId, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTextureParameteri(m_RendererId, GL_TEXTURE_WRAP_T, GL_REPEAT);

      glTextureSubImage2D(m_RendererId, 0, 0, 0, m_Width, m_Height, TextureFormatToDataFormat(m_Format), TextureFormatToDataType(m_Format), data);
      stbi_image_free(data);

      glGenerateTextureMipmap(m_RendererId);
   }


   OpenGLTexture2D::~OpenGLTexture2D() {
      glDeleteTextures(1, &m_RendererId);
   }


   TextureFormat OpenGLTexture2D::GetFormat() const {
      return m_Format;
   }


   uint32_t OpenGLTexture2D::GetWidth() const {
      return m_Width;
   }


   uint32_t OpenGLTexture2D::GetHeight() const {
      return m_Height;
   }


   void OpenGLTexture2D::SetData(void* data, const uint32_t size) {
      PKZL_CORE_ASSERT(size == m_Width * m_Height * BPP(m_Format), "Data must be entire texture!");
      glTextureSubImage2D(m_RendererId, 0, 0, 0, m_Width, m_Height, TextureFormatToDataFormat(m_Format), TextureFormatToDataType(m_Format), data);
   }


   bool OpenGLTexture2D::operator==(const Texture2D& that) {
      return m_RendererId == static_cast<const OpenGLTexture2D&>(that).m_RendererId;
   }


   uint32_t OpenGLTexture2D::GetRendererId() const {
      return m_RendererId;
   }



   OpenGLTextureCube::OpenGLTextureCube(uint32_t size, TextureFormat format)
   : m_Format {format}
   , m_Size {size}
   {
      AllocateStorage();
   }


   OpenGLTextureCube::OpenGLTextureCube(const std::filesystem::path& path)
   : m_Path {path}
   , m_Format {TextureFormat::RGBA8}
   , m_DataFormat {TextureFormat::Undefined}
   {
      uint32_t width;
      uint32_t height;
      stbi_uc* data = STBILoad(path, width, height, m_DataFormat);

      // guess whether the data is the 6-faces of a cube, or whether it's equirectangular
      // width is twice the height -> equirectangular (probably)
      // width is 4/3 the height -> 6 faces of a cube (probably)
      if (width / 2 == height) {
         m_Size = height;
      } else {
         m_Size = width / 4;
      }
      AllocateStorage();
      SetData(data, width * height * BPP(m_DataFormat));
      stbi_image_free(data);
   }


   OpenGLTextureCube::~OpenGLTextureCube() {
      glDeleteTextures(1, &m_RendererId);
   }


   TextureFormat OpenGLTextureCube::GetFormat() const {
      return m_Format;
   }


   uint32_t OpenGLTextureCube::GetWidth() const {
      return m_Size;
   }


   uint32_t OpenGLTextureCube::GetHeight() const {
      return m_Size;
   }


   void OpenGLTextureCube::SetData(void* data, uint32_t size) {
      uint32_t width = m_Size;
      uint32_t height = m_Size;
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

      std::unique_ptr<OpenGLTexture2D> tex2d = std::make_unique<OpenGLTexture2D>(width, height, m_DataFormat, 1);
      tex2d->SetData(data, size);
      int tonemap = 0;
      float gamma = 1.0f;
      if (
         (tex2d->GetFormat() == TextureFormat::RGB32F) ||
         (tex2d->GetFormat() == TextureFormat::RGBA32F)
      ) {
         tonemap = 1;
         gamma = 2.2;
      }

      std::unique_ptr<OpenGLPipeline> pipeline = std::make_unique<OpenGLPipeline>(PipelineSettings {
         {},
         {
            { Pikzel::ShaderType::Compute, shader }
         }
      });

      glUseProgram(pipeline->GetRendererId());
      glUniform1i(glGetUniformLocation(pipeline->GetRendererId(), "constants.tonemap"), tonemap);
      glUniform1f(glGetUniformLocation(pipeline->GetRendererId(), "constants.gamma"), gamma);
      glBindTextureUnit(pipeline->GetSamplerBinding("uTexture"_hs), tex2d->GetRendererId());
      glBindImageTexture(pipeline->GetStorageImageBinding("outCubeMap"_hs), m_RendererId, 0, GL_TRUE, 0, GL_WRITE_ONLY, TextureFormatToInternalFormat(m_Format));
      glDispatchCompute(GetWidth() / 32, GetHeight() / 32, 6);
      glGenerateTextureMipmap(m_RendererId);
   }


   bool OpenGLTextureCube::operator==(const TextureCube& that) {
      return m_RendererId == static_cast<const OpenGLTextureCube&>(that).m_RendererId;
   }


   uint32_t OpenGLTextureCube::GetRendererId() const {
      return m_RendererId;
   }


   void OpenGLTextureCube::AllocateStorage() {
      if ((GetWidth() % 32)) {
         throw std::runtime_error {"Cube texture size must be a multiple of 32!"};
      }

      if ((GetWidth() != GetHeight())) {
         throw std::runtime_error {"Cube texture size must be square (width == height)!"};
      }

      uint32_t levels = CalculateMipMapLevels(m_Size, m_Size);
      glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_RendererId);
      glTextureStorage2D(m_RendererId, levels, TextureFormatToInternalFormat(TextureFormat::RGBA8), m_Size, m_Size); // cubemap is always RGBA8 for now
      glTextureParameteri(m_RendererId, GL_TEXTURE_MIN_FILTER, levels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
      glTextureParameteri(m_RendererId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
   }

}
