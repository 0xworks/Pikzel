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
      virtual TextureType GetType() const override;

      virtual uint32_t GetWidth() const override;
      virtual uint32_t GetHeight() const override;
      virtual uint32_t GetDepth() const override;
      virtual uint32_t GetLayers() const override;
      virtual uint32_t GetMIPLevels() const override;

      virtual void CopyFrom(const Texture& srcTexture, const TextureCopySettings& settings = {}) override;

      virtual void Commit(const uint32_t generateMipmapAfterLevel) override;

      bool operator==(const Texture& that) override;

   public:
      uint32_t GetRendererId() const;

   protected:
      void Init(const TextureSettings& settings);
      void SetTextureParameters(const TextureSettings& settings);

      virtual void SetDepth(const uint32_t depth) = 0;
      virtual void SetLayers(const uint32_t layers) = 0;

      virtual void GLTextureStorage() = 0;
      virtual void GLCompressedTextureSubImage(const uint32_t layer, const uint32_t slice, const int mipLevel, const int xOffset, const int yOffset, const uint32_t size, const void* data) = 0;
      virtual void GLTextureSubImage(const uint32_t layer, const uint32_t slice, const int mipLevel, const int xOffset, const int yOffset, const void* data) = 0;

   protected:
      std::filesystem::path m_Path;
      TextureFormat m_Format = {};
      TextureFormat m_DataFormat = {}; // temporary, only used during upload of cubemap texture from a 2D texture (this is the 2D texture's format)
      uint32_t m_Width = {};
      uint32_t m_Height = {};
      uint32_t m_Depth = {};
      uint32_t m_Layers = {};
      uint32_t m_MIPLevels = {};
      uint32_t m_RendererId = {};
   };


   class OpenGLTexture2D : public OpenGLTexture {
   public:
      OpenGLTexture2D(const TextureSettings& settings);

      virtual TextureType GetType() const override;

      virtual void SetData(const void* data, const uint32_t size) override;

   protected:
      virtual void SetDepth(const uint32_t depth) override;
      virtual void SetLayers(const uint32_t layers) override;

      virtual void GLTextureStorage() override;
      virtual void GLCompressedTextureSubImage(const uint32_t layer, const uint32_t slice, const int mipLevel, const int xOffset, const int yOffset, const uint32_t size, const void* data) override;
      virtual void GLTextureSubImage(const uint32_t layer, const uint32_t slice, const int mipLevel, const int xOffset, const int yOffset, const void* data) override;
   };


   class OpenGLTexture2DArray : public OpenGLTexture {
   public:
      OpenGLTexture2DArray(const TextureSettings& settings);

      virtual TextureType GetType() const override;

      virtual void SetData(const void* data, const uint32_t size) override;

   protected:
      virtual void SetDepth(const uint32_t depth) override;
      virtual void SetLayers(const uint32_t layers) override;

      virtual void GLTextureStorage() override;
      virtual void GLCompressedTextureSubImage(const uint32_t layer, const uint32_t slice, const int mipLevel, const int xOffset, const int yOffset, const uint32_t size, const void* data) override;
      virtual void GLTextureSubImage(const uint32_t layer, const uint32_t slice, const int mipLevel, const int xOffset, const int yOffset, const void* data) override;
   };


   class OpenGLTextureCube : public OpenGLTexture {
   public:

      OpenGLTextureCube(const TextureSettings& settings);

      virtual TextureType GetType() const override;

      virtual void SetData(const void* data, const uint32_t size) override;

   protected:
      virtual void SetDepth(const uint32_t depth) override;
      virtual void SetLayers(const uint32_t layers) override;

      virtual void GLTextureStorage() override;
      virtual void GLCompressedTextureSubImage(const uint32_t layer, const uint32_t slice, const int mipLevel, const int xOffset, const int yOffset, const uint32_t size, const void* data) override;
      virtual void GLTextureSubImage(const uint32_t layer, const uint32_t slice, const int mipLevel, const int xOffset, const int yOffset, const void* data) override;
   };


   class OpenGLTextureCubeArray : public OpenGLTexture {
   public:
      OpenGLTextureCubeArray(const TextureSettings& settings);

      virtual TextureType GetType() const override;

      virtual void SetData(const void* data, const uint32_t size) override;

   protected:
      virtual void SetDepth(const uint32_t depth) override;
      virtual void SetLayers(const uint32_t layers) override;

      virtual void GLTextureStorage() override;
      virtual void GLCompressedTextureSubImage(const uint32_t layer, const uint32_t slice, const int mipLevel, const int xOffset, const int yOffset, const uint32_t size, const void* data) override;
      virtual void GLTextureSubImage(const uint32_t layer, const uint32_t slice, const int mipLevel, const int xOffset, const int yOffset, const void* data) override;
   };

}
