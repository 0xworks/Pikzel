#include "VulkanDevice.h"
#include "VulkanUtility.h"

#include <set>

namespace Pikzel {

   VulkanDevice::VulkanDevice(vk::Instance instance, vk::SurfaceKHR surface)
   : m_Instance {instance}
   {
      SelectPhysicalDevice(surface);
      CreateDevice();
      CreateCommandPool();
   }


   VulkanDevice::~VulkanDevice() {
      DestroyCommandPool();
      DestroyDevice();
   }


   bool VulkanDevice::IsPhysicalDeviceSuitable(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface) {
      bool queueAdequate = false;
      bool extensionsSupported = false;
      bool swapChainAdequate = false;
      QueueFamilyIndices indices = FindQueueFamilies(physicalDevice, surface);
      if (indices.GraphicsFamily.has_value() && (!surface || indices.PresentFamily.has_value()) && indices.ComputeFamily.has_value() && indices.TransferFamily.has_value()) {
         queueAdequate = true;
         extensionsSupported = CheckDeviceExtensionSupport(physicalDevice, GetRequiredDeviceExtensions());
         if (extensionsSupported) {
            if (surface) {
               SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(physicalDevice, surface);
               swapChainAdequate = !swapChainSupport.Formats.empty() && !swapChainSupport.PresentModes.empty();
            } else {
               swapChainAdequate = true;
            }
         }
      }
      return queueAdequate && extensionsSupported && swapChainAdequate;
   }


   std::vector<const char*> VulkanDevice::GetRequiredDeviceExtensions() const {
      return {VK_KHR_SWAPCHAIN_EXTENSION_NAME /*, VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME*/};   // At time of writing VK_EXT_extended_dynamic_state is not available in the nvidia general release drivers.
   }


   void VulkanDevice::EnablePhysicalDeviceFeatures() {
      auto result = m_PhysicalDevice.getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features>();
      auto availableFeatures = result.get<vk::PhysicalDeviceFeatures2>().features;
      auto availableVulkan13Features = result.get<vk::PhysicalDeviceVulkan13Features>();


      if (availableFeatures.samplerAnisotropy) {
         m_EnabledPhysicalDeviceFeatures.features.setSamplerAnisotropy(true);
      }
      if (availableFeatures.geometryShader) {
         m_EnabledPhysicalDeviceFeatures.features.setGeometryShader(true);
      }
      if (availableFeatures.imageCubeArray) {
         m_EnabledPhysicalDeviceFeatures.features.setImageCubeArray(true);
      }
      if (availableVulkan13Features.maintenance4) {
         m_EnabledPhysicalDeviceVulkan13Features.setMaintenance4(true);
      }
   }


   void VulkanDevice::SelectPhysicalDevice(vk::SurfaceKHR surface) {
      uint32_t deviceCount = 0;
      std::vector<vk::PhysicalDevice> physicalDevices = m_Instance.enumeratePhysicalDevices();
      for (const auto& physicalDevice : physicalDevices) {
         if (IsPhysicalDeviceSuitable(physicalDevice, surface)) {
            m_PhysicalDevice = physicalDevice;
            m_PhysicalDeviceProperties = m_PhysicalDevice.getProperties();
            m_QueueFamilyIndices = FindQueueFamilies(m_PhysicalDevice, surface);
            break;
         }
      }
      if (!m_PhysicalDevice) {
         throw std::runtime_error {"failed to find a suitable GPU!"};
      }
   }


   void VulkanDevice::CreateDevice() {
      constexpr float queuePriority = 1.0f;

      DestroyDevice();

      std::vector<vk::DeviceQueueCreateInfo> deviceQueueCIs;
      std::set<uint32_t> uniqueQueueFamilies;
      if (m_QueueFamilyIndices.GraphicsFamily.has_value()) {
         uniqueQueueFamilies.insert(m_QueueFamilyIndices.GraphicsFamily.value());
      }
      if (m_QueueFamilyIndices.PresentFamily.has_value()) {
         uniqueQueueFamilies.insert(m_QueueFamilyIndices.PresentFamily.value());
      }
      if (m_QueueFamilyIndices.ComputeFamily.has_value()) {
         uniqueQueueFamilies.insert(m_QueueFamilyIndices.ComputeFamily.value());
      }

      for (uint32_t queueFamily : uniqueQueueFamilies) {
         deviceQueueCIs.emplace_back(
            vk::DeviceQueueCreateFlags {},
            queueFamily,
            1,
            &queuePriority
         );
      }

      std::vector<const char*> deviceExtensions = GetRequiredDeviceExtensions();

      EnablePhysicalDeviceFeatures();

      vk::DeviceCreateInfo ci = {
         {},
         static_cast<uint32_t>(deviceQueueCIs.size())     /*queueCreateInfoCount*/,
         deviceQueueCIs.data()                            /*pQueueCreateInfos*/,
         0                                                /*enabledLayerCount*/,
         nullptr                                          /*ppEnabledLayerNames*/,
         static_cast<uint32_t>(deviceExtensions.size())   /*enabledExtensionCount*/,
         deviceExtensions.data()                          /*ppEnabledExtensionNames*/,
         nullptr                                          /*pEnabledFeatures*/
      };
      ci.pNext = &vk::StructureChain(m_EnabledPhysicalDeviceFeatures, m_EnabledPhysicalDeviceVulkan13Features).get<vk::PhysicalDeviceFeatures2>();

#ifdef PKZL_DEBUG
      std::vector<const char*> layers = {"VK_LAYER_KHRONOS_validation"};
      ci.enabledLayerCount = static_cast<uint32_t>(layers.size());
      ci.ppEnabledLayerNames = layers.data();
#endif

      m_Device = m_PhysicalDevice.createDevice(ci);

      if (m_QueueFamilyIndices.GraphicsFamily.has_value()) {
         m_GraphicsQueue = m_Device.getQueue(m_QueueFamilyIndices.GraphicsFamily.value(), 0);
      }
      if (m_QueueFamilyIndices.PresentFamily.has_value()) {
         m_PresentQueue = m_Device.getQueue(m_QueueFamilyIndices.PresentFamily.value(), 0);
      }
      if (m_QueueFamilyIndices.ComputeFamily.has_value()) {
         m_ComputeQueue = m_Device.getQueue(m_QueueFamilyIndices.ComputeFamily.value(), 0);
      }
      if (m_QueueFamilyIndices.TransferFamily.has_value()) {
         m_TransferQueue = m_Device.getQueue(m_QueueFamilyIndices.ComputeFamily.value(), 0);
      }
   }


   void VulkanDevice::DestroyDevice() {
      if (m_Device) {
         m_Device.destroy();
         m_Device = nullptr;
      }
   }


   void VulkanDevice::CreateCommandPool() {
      m_CommandPool = m_Device.createCommandPool({
         vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
         m_QueueFamilyIndices.GraphicsFamily.value()
      });
   }


   void VulkanDevice::DestroyCommandPool() {
      if (m_Device && m_CommandPool) {
         m_Device.destroy(m_CommandPool);
         m_CommandPool = nullptr;
      }
   }


   vk::Instance VulkanDevice::GetVkInstance() const {
      return m_Instance;
   }


   vk::Device VulkanDevice::GetVkDevice() const {
      return m_Device;
   }


   vk::PhysicalDevice VulkanDevice::GetVkPhysicalDevice() const {
      return m_PhysicalDevice;
   }


   uint32_t VulkanDevice::GetGraphicsQueueFamilyIndex() const {
      PKZL_CORE_ASSERT(m_QueueFamilyIndices.GraphicsFamily.has_value(), "no graphics queue family index");
      return m_QueueFamilyIndices.GraphicsFamily.value();
   }


   uint32_t VulkanDevice::GetPresentQueueFamilyIndex() const {
      PKZL_CORE_ASSERT(m_QueueFamilyIndices.GraphicsFamily.has_value(), "no present queue family index");
      return m_QueueFamilyIndices.PresentFamily.value();
   }


   uint32_t VulkanDevice::GetComputeQueueFamilyIndex() const {
      PKZL_CORE_ASSERT(m_QueueFamilyIndices.ComputeFamily.has_value(), "no compute queue family index");
      return m_QueueFamilyIndices.ComputeFamily.value();
   }


   uint32_t VulkanDevice::GetTransferQueueFamilyIndex() const {
      PKZL_CORE_ASSERT(m_QueueFamilyIndices.TransferFamily.has_value(), "no transfer queue family index");
      return m_QueueFamilyIndices.TransferFamily.value();
   }


   vk::Queue VulkanDevice::GetGraphicsQueue() const {
      return m_GraphicsQueue;
   }


   vk::Queue VulkanDevice::GetPresentQueue() const {
      return m_PresentQueue;
   }


   vk::Queue VulkanDevice::GetComputeQueue() const {
      return m_ComputeQueue;
   }


   vk::Queue VulkanDevice::GetTransferQueue() const {
      return m_TransferQueue;
   }


   uint32_t VulkanDevice::GetMSAAMaxSamples() const {
      vk::SampleCountFlags counts = m_PhysicalDeviceProperties.limits.framebufferColorSampleCounts & m_PhysicalDeviceProperties.limits.framebufferDepthSampleCounts;
      if (counts & vk::SampleCountFlagBits::e64) return 64;
      if (counts & vk::SampleCountFlagBits::e32) return 32;
      if (counts & vk::SampleCountFlagBits::e16) return 16;
      if (counts & vk::SampleCountFlagBits::e8)  return 8;
      if (counts & vk::SampleCountFlagBits::e4)  return 4;
      if (counts & vk::SampleCountFlagBits::e2)  return 2;
      return 1;
   }


   void VulkanDevice::SubmitSingleTimeCommands(vk::Queue queue, const std::function<void(vk::CommandBuffer)>& action) {
      std::vector<vk::CommandBuffer> commandBuffers = m_Device.allocateCommandBuffers({
         m_CommandPool                    /*commandPool*/,
         vk::CommandBufferLevel::ePrimary /*level*/,
         1                                /*commandBufferCount*/
      });

      commandBuffers[0].begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
      action(commandBuffers[0]);
      commandBuffers[0].end();

      vk::SubmitInfo si;
      si.commandBufferCount = 1;
      si.pCommandBuffers = commandBuffers.data();
      queue.submit(si, nullptr);
      queue.waitIdle();
      m_Device.freeCommandBuffers(m_CommandPool, commandBuffers);
   }


   void VulkanDevice::PipelineBarrier(vk::PipelineStageFlags srcStageMask, vk::PipelineStageFlags dstStageMask, const vk::ArrayProxy<const vk::ImageMemoryBarrier>& barriers) {
      // TODO: which queue should this be?
      SubmitSingleTimeCommands(m_GraphicsQueue, [this, srcStageMask, dstStageMask, &barriers] (vk::CommandBuffer cmd) {
         cmd.pipelineBarrier(srcStageMask, dstStageMask, {}, nullptr, nullptr, barriers);
      });
   }
}
