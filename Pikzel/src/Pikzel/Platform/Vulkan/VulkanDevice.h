#pragma once

#include "QueueFamilyIndices.h"
#include <vulkan/vulkan.hpp>

namespace Pikzel {

   class VulkanDevice {
   public:
      VulkanDevice(vk::Instance instance, vk::SurfaceKHR surface);
      virtual ~VulkanDevice();

      vk::Instance GetVkInstance() const;
      vk::Device GetVkDevice() const;
      vk::PhysicalDevice GetVkPhysicalDevice() const;

      uint32_t GetGraphicsQueueFamilyIndex() const;
      uint32_t GetPresentQueueFamilyIndex() const;
      uint32_t GetComputeQueueFamilyIndex() const;
      uint32_t GetTransferQueueFamilyIndex() const;

      vk::Queue GetGraphicsQueue() const;
      vk::Queue GetPresentQueue() const;
      vk::Queue GetComputeQueue() const;
      vk::Queue GetTransferQueue() const;

      uint32_t GetMSAAMaxSamples() const;

      vk::PhysicalDeviceFeatures GetEnabledPhysicalDeviceFeatures() const {
         return m_EnabledPhysicalDeviceFeatures.features;
      }

      vk::PhysicalDeviceVulkan13Features GetEnabledPhysicalDeviceVulkan13Features() const {
         return m_EnabledPhysicalDeviceVulkan13Features;
      }

      void SubmitSingleTimeCommands(vk::Queue queue, const std::function<void(vk::CommandBuffer)>& action);

      void PipelineBarrier(vk::PipelineStageFlags srcStageMask, vk::PipelineStageFlags dstStageMask, const vk::ArrayProxy<const vk::ImageMemoryBarrier>& barriers);

   private:
      bool IsPhysicalDeviceSuitable(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface);
      std::vector<const char*> GetRequiredDeviceExtensions() const;
      void EnablePhysicalDeviceFeatures();
      void SelectPhysicalDevice(vk::SurfaceKHR surface);

      void CreateDevice();
      void DestroyDevice();

      void CreateCommandPool();
      void DestroyCommandPool();

   private:
      vk::Instance m_Instance;
      vk::PhysicalDevice m_PhysicalDevice;
      vk::PhysicalDeviceProperties m_PhysicalDeviceProperties;
      vk::PhysicalDeviceFeatures2 m_EnabledPhysicalDeviceFeatures;
      vk::PhysicalDeviceVulkan13Features m_EnabledPhysicalDeviceVulkan13Features;

      QueueFamilyIndices m_QueueFamilyIndices;

      vk::Device m_Device;

      vk::Queue m_GraphicsQueue;
      vk::Queue m_PresentQueue;
      vk::Queue m_ComputeQueue;
      vk::Queue m_TransferQueue;

      vk::CommandPool m_CommandPool;

   };

}
