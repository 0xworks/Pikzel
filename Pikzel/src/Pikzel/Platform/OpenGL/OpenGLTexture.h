#pragma once

#include "Pikzel/Renderer/Texture.h"

#include <filesystem>

namespace Pikzel {

   class OpenGLTexture2D : public Texture2D {
   public:
      OpenGLTexture2D(uint32_t width, uint32_t height);
      OpenGLTexture2D(const std::filesystem::path& path);
      virtual ~OpenGLTexture2D();

      virtual uint32_t GetWidth() const override { return m_Width; }
      virtual uint32_t GetHeight() const override { return m_Height; }
      virtual uint32_t GetRendererID() const override { return m_RendererID; }

      virtual void SetData(void* data, uint32_t size) override;

      virtual bool operator==(const Texture2D& other) const override {
         return m_RendererID == ((OpenGLTexture2D&)other).m_RendererID;
      }

   private:
      std::filesystem::path m_Path;
      uint32_t m_Width, m_Height;
      uint32_t m_RendererID;
      GLenum m_InternalFormat, m_DataFormat;
   };

}
