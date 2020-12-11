#pragma once

#include "Pikzel/Renderer/Texture.h"

#include <filesystem>

namespace Pikzel {

   GLenum TextureFormatToInternalFormat(const TextureFormat format);
   GLenum TextureFormatToDataFormat(const TextureFormat format);
   GLenum TextureFormatToDataType(const TextureFormat format);

   class OpenGLTexture2D : public Texture {
   public:
      OpenGLTexture2D(uint32_t width, uint32_t height, const TextureFormat format, const uint32_t mipLevels);
      OpenGLTexture2D(const std::filesystem::path& path, const bool isSRGB);
      virtual ~OpenGLTexture2D();

      virtual TextureFormat GetFormat() const override;
      virtual TextureType GetType() const override;

      virtual uint32_t GetWidth() const override;
      virtual uint32_t GetHeight() const override;

      virtual void SetData(void* data, const uint32_t size) override;

      bool operator==(const Texture& that) override;

   public:
      uint32_t GetRendererId() const;

   private:
      std::filesystem::path m_Path;
      TextureFormat m_Format = {};
      uint32_t m_Width = {};
      uint32_t m_Height = {};
      uint32_t m_RendererId = {};
   };


   class OpenGLTextureCube : public Texture {
   public:

      OpenGLTextureCube(uint32_t size, TextureFormat format, const uint32_t mipLevels);
      OpenGLTextureCube(const std::filesystem::path& path, const bool isSRGB);
      virtual ~OpenGLTextureCube();

      virtual TextureFormat GetFormat() const override;
      virtual TextureType GetType() const override;
      virtual uint32_t GetWidth() const override;
      virtual uint32_t GetHeight() const override;

      virtual void SetData(void* data, const uint32_t size) override;

      bool operator==(const Texture& that) override;

   public:
      uint32_t GetRendererId() const;

   private:
      void AllocateStorage(const uint32_t mipLevels);

   private:
      std::filesystem::path m_Path;
      TextureFormat m_Format = {};     // format of the cubemap texture (currently always RGBA8)
      TextureFormat m_DataFormat = {}; // format of the data texture was originally loaded from
      uint32_t m_Size = {};
      uint32_t m_RendererId = {};
   };
}
