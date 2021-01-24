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
      virtual uint32_t GetMIPLevels() const override;

      virtual void Commit(const bool generateMipmap = true) override;

      bool operator==(const Texture& that) override;

   public:
      uint32_t GetRendererId() const;

   protected:
      void SetTextureParameters(const TextureSettings& settings);

   protected:
      TextureFormat m_Format = {};
      uint32_t m_Width = {};
      uint32_t m_Height = {};
      uint32_t m_MIPLevels = {};
      uint32_t m_RendererId = {};
   };


   class OpenGLTexture2D : public OpenGLTexture {
   public:
      OpenGLTexture2D(const TextureSettings& settings);

      virtual TextureType GetType() const override;

      virtual uint32_t GetLayers() const override;

      virtual void SetData(void* data, const uint32_t size) override;

      void CopyFrom(const Texture& srcTexture, const TextureCopySettings& settings = {}) override;

   private:
      std::filesystem::path m_Path;
   };


   class OpenGLTexture2DArray : public OpenGLTexture {
   public:
      OpenGLTexture2DArray(const TextureSettings& settings);

      virtual TextureType GetType() const override;

      virtual uint32_t GetLayers() const override;

      virtual void SetData(void* data, const uint32_t size) override;

      void CopyFrom(const Texture& srcTexture, const TextureCopySettings& settings = {}) override;

   private:
      uint32_t m_Layers = {};
   };


   class OpenGLTextureCube : public OpenGLTexture {
   public:

      OpenGLTextureCube(const TextureSettings& settings);

      virtual TextureType GetType() const override;

      virtual uint32_t GetLayers() const override;

      virtual void SetData(void* data, const uint32_t size) override;

      void CopyFrom(const Texture& srcTexture, const TextureCopySettings& settings = {}) override;

   private:
      void AllocateStorage();

   private:
      std::filesystem::path m_Path;
      TextureFormat m_DataFormat = {}; // format of the data texture was originally loaded from
   };


   class OpenGLTextureCubeArray : public OpenGLTexture {
   public:
      OpenGLTextureCubeArray(const TextureSettings& settings);

      virtual TextureType GetType() const override;

      virtual uint32_t GetLayers() const override;

      virtual void SetData(void* data, const uint32_t size) override;

      void CopyFrom(const Texture& srcTexture, const TextureCopySettings& settings = {}) override;

   private:
      uint32_t m_Layers = {};
   };

}
