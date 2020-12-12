#pragma once

#include "Pikzel/Renderer/Texture.h"

#include <filesystem>

namespace Pikzel {

   GLenum TextureFormatToInternalFormat(const TextureFormat format);
   GLenum TextureFormatToDataFormat(const TextureFormat format);
   GLenum TextureFormatToDataType(const TextureFormat format);


   class OpenGLTexture : public Texture {
   public:
      virtual ~OpenGLTexture();

      virtual TextureFormat GetFormat() const override;

      virtual uint32_t GetWidth() const override;
      virtual uint32_t GetHeight() const override;

      bool operator==(const Texture& that) override;

   public:
      uint32_t GetRendererId() const;

   protected:
      TextureFormat m_Format = {};
      uint32_t m_Width = {};
      uint32_t m_Height = {};
      uint32_t m_RendererId = {};
   };


   class OpenGLTexture2D : public OpenGLTexture {
   public:
      OpenGLTexture2D(const uint32_t width, const uint32_t height, const TextureFormat format, const uint32_t mipLevels);
      OpenGLTexture2D(const std::filesystem::path& path, const bool isSRGB);

      virtual TextureType GetType() const override;

      virtual uint32_t GetLayers() const override;

      virtual void SetData(void* data, const uint32_t size) override;

   private:
      std::filesystem::path m_Path;
   };


   class OpenGLTexture2DArray : public OpenGLTexture {
   public:
      OpenGLTexture2DArray(const uint32_t width, const uint32_t height, const uint32_t layers, const TextureFormat format, const uint32_t mipLevels);

      virtual TextureType GetType() const override;

      virtual uint32_t GetLayers() const override;

      virtual void SetData(void* data, const uint32_t size) override;

   private:
      uint32_t m_Layers = {};
   };


   class OpenGLTextureCube : public OpenGLTexture {
   public:

      OpenGLTextureCube(const uint32_t size, TextureFormat format, const uint32_t mipLevels);
      OpenGLTextureCube(const std::filesystem::path& path, const bool isSRGB);

      virtual TextureType GetType() const override;

      virtual uint32_t GetLayers() const override;

      virtual void SetData(void* data, const uint32_t size) override;

   private:
      void AllocateStorage(const uint32_t mipLevels);

   private:
      std::filesystem::path m_Path;
      TextureFormat m_DataFormat = {}; // format of the data texture was originally loaded from
   };


   class OpenGLTextureCubeArray : public OpenGLTexture {
   public:
      OpenGLTextureCubeArray(const uint32_t size, const uint32_t layers, const TextureFormat format, const uint32_t mipLevels);

      virtual TextureType GetType() const override;

      virtual uint32_t GetLayers() const override;

      virtual void SetData(void* data, const uint32_t size) override;

   private:
      void AllocateStorage(const uint32_t mipLevels);

   private:
      uint32_t m_Layers = {};
   };

}
