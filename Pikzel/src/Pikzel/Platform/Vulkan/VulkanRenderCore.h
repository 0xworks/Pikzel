#pragma once

#include "Pikzel/Renderer/RenderCore.h"
#include "VulkanDevice.h"

namespace Pikzel {

   class VulkanRenderCore : public IRenderCore {
   public:
      VulkanRenderCore(const Window& window);
      virtual ~VulkanRenderCore();

      virtual void SetViewport(const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height) override;

      virtual std::unique_ptr<GraphicsContext> CreateGraphicsContext(const Window& window) override;

      virtual std::unique_ptr<VertexBuffer> CreateVertexBuffer(const uint32_t size) override;
      virtual std::unique_ptr<VertexBuffer> CreateVertexBuffer(const uint32_t size,const void* data) override;

      virtual std::unique_ptr<IndexBuffer> CreateIndexBuffer(const uint32_t count, const uint32_t* indices) override;

      std::unique_ptr<Texture2D> CreateTexture2D(const uint32_t width, const uint32_t height) override;
      std::unique_ptr<Texture2D> CreateTexture2D(const std::filesystem::path& path) override;

   private:
      std::vector<const char*> GetRequiredInstanceExtensions();

      void CreateInstance();
      void DestroyInstance();


   private:
      vk::Instance m_Instance;
      vk::DebugUtilsMessengerEXT m_DebugUtilsMessengerEXT;

      std::shared_ptr<VulkanDevice> m_Device;

   };

}
