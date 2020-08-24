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

      vk::Queue GetGraphicsQueue() const;
      vk::Queue GetPresentQueue() const;

   private:
      bool IsPhysicalDeviceSuitable(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface);
      std::vector<const char*> GetRequiredDeviceExtensions() const;
      vk::PhysicalDeviceFeatures GetRequiredPhysicalDeviceFeatures(vk::PhysicalDeviceFeatures availableFeatures) const;
      void* GetRequiredPhysicalDeviceFeaturesEXT() const;
      void SelectPhysicalDevice(vk::SurfaceKHR surface);

      void CreateDevice();
      void DestroyDevice();

   private:
      vk::Instance m_Instance;
      vk::PhysicalDevice m_PhysicalDevice;
      vk::PhysicalDeviceProperties m_PhysicalDeviceProperties;
      vk::PhysicalDeviceFeatures m_PhysicalDeviceFeatures;                 // features that are available on the selected physical device
      vk::PhysicalDeviceFeatures m_EnabledPhysicalDeviceFeatures;          // features that have been enabled
      vk::PhysicalDeviceMemoryProperties m_PhysicalDeviceMemoryProperties;
      QueueFamilyIndices m_QueueFamilyIndices;

      vk::Device m_Device;

      vk::Queue m_GraphicsQueue;
      vk::Queue m_PresentQueue;
   };

}

