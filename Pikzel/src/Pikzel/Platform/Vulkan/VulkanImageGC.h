#pragma once
#include "VulkanDevice.h"
#include "VulkanGraphicsContext.h"

namespace Pikzel {

   class VulkanImageGC: public VulkanGraphicsContext {
   public:
      VulkanImageGC(std::shared_ptr<VulkanDevice> device, VulkanImage* image);
      virtual ~VulkanImageGC();

      virtual void BeginFrame() override;
      virtual void EndFrame() override;

      virtual void SwapBuffers() override;

   private:

      void CreateFrameBuffer();
      void DestroyFrameBuffer();

      void CreateSyncObjects();
      void DestroySyncObjects();

   private:
      VulkanImage* m_Image;
      vk::Fence m_ImageFence;
      vk::Framebuffer m_FrameBuffer;
   };

}
