#pragma once
#include <vulkan/vulkan.hpp>

#include "QueueFamilyIndices.h"
#include "SwapChainSupportDetails.h"

namespace Pikzel {

   void CheckLayerSupport(std::vector<const char*> layers);

   void CheckInstanceExtensionSupport(std::vector<const char*> extensions);

   bool CheckDeviceExtensionSupport(vk::PhysicalDevice physicalDevice, std::vector<const char*> extensions);

   QueueFamilyIndices FindQueueFamilies(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface);

   vk::Format FindSupportedFormat(vk::PhysicalDevice physicalDevice, const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);

   SwapChainSupportDetails QuerySwapChainSupport(vk::PhysicalDevice device, vk::SurfaceKHR surface);

}
