#pragma once

#include "Pikzel/Renderer/Texture.h"

#include <filesystem>

namespace Pikzel {

   class OpenGLTexture2D : public Texture2D {
   public:
      OpenGLTexture2D(uint32_t width, uint32_t height, TextureFormat format = TextureFormat::RGB8);
      OpenGLTexture2D(const std::filesystem::path& path);
      virtual ~OpenGLTexture2D();

      virtual TextureFormat GetFormat() const override;
      virtual uint32_t GetWidth() const override;
      virtual uint32_t GetHeight() const override;

      virtual void SetData(void* data, const uint32_t size) override;

      bool operator==(const Texture2D& that) override;

   public:
      uint32_t GetRendererId() const;

   private:
      std::filesystem::path m_Path;
      TextureFormat m_Format;
      uint32_t m_Width;
      uint32_t m_Height;
      uint32_t m_RendererId;
   };


   class OpenGLTextureCube : public TextureCube {
   public:

      OpenGLTextureCube(uint32_t size, TextureFormat format = TextureFormat::RGB8);
      OpenGLTextureCube(const std::filesystem::path& path);
      virtual ~OpenGLTextureCube();

      virtual TextureFormat GetFormat() const override;
      virtual uint32_t GetWidth() const override;
      virtual uint32_t GetHeight() const override;

      virtual void SetData(void* data, const uint32_t size) override;

      bool operator==(const TextureCube& that) override;

   public:
      uint32_t GetRendererId() const;

   private:
      void AllocateStorage();

   private:
      std::filesystem::path m_Path;
      TextureFormat m_Format;     // format of the cubemap texture (currently always RGBA8)
      TextureFormat m_DataFormat; // format of underlying data that was used to create the cubemap
      uint32_t m_Size;
      uint32_t m_RendererId;
   };
}
