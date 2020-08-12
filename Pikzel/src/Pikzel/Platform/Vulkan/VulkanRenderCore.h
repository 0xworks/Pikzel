#pragma once

#include "Pikzel/Renderer/RenderCore.h"
#include "QueueFamilyIndices.h"
#include <vulkan/vulkan.hpp>

namespace Pikzel {

   class VulkanRenderCore : public RenderCore {
   public:
      VulkanRenderCore();
      virtual ~VulkanRenderCore();

      std::unique_ptr<GraphicsContext> CreateGraphicsContext(const Window& window) override;

      virtual RendererAPI GetAPI() const override;

   private:
      std::vector<const char*> GetRequiredInstanceExtensions();

      void CreateInstance();
      void DestroyInstance();

   private:
      vk::Instance m_Instance;
      vk::DebugUtilsMessengerEXT m_DebugUtilsMessengerEXT;
   };

}
