#pragma once

#include "Pikzel/Renderer/Texture.h"

#include <filesystem>

namespace Pikzel {

   class OpenGLTexture2D : public Texture2D {
   public:
      OpenGLTexture2D(uint32_t width, uint32_t height);
      OpenGLTexture2D(const std::filesystem::path& path);
      virtual ~OpenGLTexture2D();

      virtual uint32_t GetWidth() const override;
      virtual uint32_t GetHeight() const override;

      virtual void SetData(void* data, uint32_t size) override;

   public:
      uint32_t GetRendererId() const;

   private:
      std::filesystem::path m_Path;
      uint32_t m_Width;
      uint32_t m_Height;
      uint32_t m_RendererId;
      GLenum m_InternalFormat;
      GLenum m_DataFormat;
   };

}
