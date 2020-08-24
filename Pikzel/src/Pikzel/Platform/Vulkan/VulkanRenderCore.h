#pragma once

#include "Pikzel/Renderer/RenderCore.h"
#include "QueueFamilyIndices.h"
#include "VulkanDevice.h"

namespace Pikzel {

   class VulkanRenderCore : public RenderCore {
   public:
      VulkanRenderCore();
      virtual ~VulkanRenderCore();

      virtual RendererAPI GetAPI() const override;

      virtual std::unique_ptr<Buffer> CreateBuffer(const uint64_t size) override;
      virtual std::unique_ptr<Image> CreateImage(const ImageSettings& settings = ImageSettings()) override;

      virtual std::unique_ptr<GraphicsContext> CreateGraphicsContext(Window& window) override;
      virtual std::unique_ptr<GraphicsContext> CreateGraphicsContext(Image& image) override;



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
