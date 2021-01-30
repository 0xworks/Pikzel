#include "VulkanUtility.h"

namespace Pikzel {

   void CheckLayerSupport(std::vector<const char*> layers) {
      std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();
      for (const auto& layer : layers) {
         bool layerFound = false;
         for (const auto& availableLayer : availableLayers) {
            if (strcmp(layer, availableLayer.layerName) == 0) {
               layerFound = true;
               break;
            }
         }
         if (!layerFound) {
            PKZL_CORE_LOG_INFO("available layers:");
            for (const auto& layer : availableLayers) {
               PKZL_CORE_LOG_INFO("\t{0}", layer.layerName);
            }
            std::string msg {"layer '"};
            msg += layer;
            msg += "' requested but is not available!";
            throw std::runtime_error {msg};
         }
      }
   }


   void CheckInstanceExtensionSupport(std::vector<const char*> extensions) {
      std::vector<vk::ExtensionProperties> availableExtensions = vk::enumerateInstanceExtensionProperties();
      for (const auto& extension : extensions) {
         bool extensionFound = false;
         for (const auto& availableExtension : availableExtensions) {
            if (strcmp(extension, availableExtension.extensionName) == 0) {
               extensionFound = true;
               break;
            }
         }
         if (!extensionFound) {
            PKZL_CORE_LOG_INFO("available extensions:");
            for (const auto& extension : availableExtensions) {
               PKZL_CORE_LOG_INFO("\t{0}", extension.extensionName);
            }
            std::string msg {"extension '"};
            msg += extension;
            msg += "' requested but is not available!";
            throw std::runtime_error {msg};
         }
      }
   }


   bool CheckDeviceExtensionSupport(vk::PhysicalDevice physicalDevice, std::vector<const char*> extensions) {
      bool allExtensionsFound = true;
      std::vector<vk::ExtensionProperties> availableExtensions = physicalDevice.enumerateDeviceExtensionProperties();
      for (const auto& extension : extensions) {
         bool extensionFound = false;
         for (const auto& availableExtension : availableExtensions) {
            if (strcmp(extension, availableExtension.extensionName) == 0) {
               extensionFound = true;
               break;
            }
         }
         if (!extensionFound) {
            allExtensionsFound = false;
            break;
         }
      }
      return allExtensionsFound;
   }


   QueueFamilyIndices FindQueueFamilies(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface) {
      QueueFamilyIndices indices;

      std::vector<vk::QueueFamilyProperties> queueFamilies = physicalDevice.getQueueFamilyProperties();
      int i = 0;
      for (const auto& queueFamily : queueFamilies) {
         if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
            indices.GraphicsFamily = i;
         }

         if (surface) {
            if (physicalDevice.getSurfaceSupportKHR(i, surface)) {
               indices.PresentFamily = i;
            }
         }

         if (queueFamily.queueFlags & vk::QueueFlagBits::eCompute) {
            indices.ComputeFamily = i;
         }

         if (queueFamily.queueFlags & vk::QueueFlagBits::eTransfer) {
            indices.TransferFamily = i;
         }

         if (indices.GraphicsFamily.has_value() && indices.PresentFamily.has_value() && indices.ComputeFamily.has_value() && indices.TransferFamily.has_value()) {
            break;
         }

         ++i;
      }

      return indices;
   }


   vk::Format FindSupportedFormat(vk::PhysicalDevice physicalDevice, const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) {
      for (auto format : candidates) {
         vk::FormatProperties props = physicalDevice.getFormatProperties(format);
         if ((tiling == vk::ImageTiling::eLinear) && ((props.linearTilingFeatures & features) == features)) {
            return format;
         } else if ((tiling == vk::ImageTiling::eOptimal) && ((props.optimalTilingFeatures & features) == features)) {
            return format;
         }
      }
      throw std::runtime_error {"failed to find supported format!"};
   }


   SwapChainSupportDetails QuerySwapChainSupport(vk::PhysicalDevice device, vk::SurfaceKHR surface) {
      return {
         device.getSurfaceCapabilitiesKHR(surface),
         device.getSurfaceFormatsKHR(surface),
         device.getSurfacePresentModesKHR(surface)
      };
   }

}
