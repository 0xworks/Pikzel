#include "vkpch.h"
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
      if (indices.GraphicsFamily.has_value() && (!surface || indices.PresentFamily.has_value())) {
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
      return {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
   }


   vk::PhysicalDeviceFeatures VulkanDevice::GetRequiredPhysicalDeviceFeatures(vk::PhysicalDeviceFeatures availableFeatures) const {
      vk::PhysicalDeviceFeatures features;
      if (availableFeatures.samplerAnisotropy) {
         features.setSamplerAnisotropy(true);
      } else {
         // TODO: just don't use this feature if it isn't available, rather than fatal error
         throw std::runtime_error("Sampler Anisotropy is not supported on this device!");
      }
      return features;
   }


   void* VulkanDevice::GetRequiredPhysicalDeviceFeaturesEXT() const {
      return nullptr;
   }


   void VulkanDevice::SelectPhysicalDevice(vk::SurfaceKHR surface) {
      uint32_t deviceCount = 0;
      std::vector<vk::PhysicalDevice> physicalDevices = m_Instance.enumeratePhysicalDevices();
      for (const auto& physicalDevice : physicalDevices) {
         if (IsPhysicalDeviceSuitable(physicalDevice, surface)) {
            m_PhysicalDevice = physicalDevice;
            m_PhysicalDeviceProperties = m_PhysicalDevice.getProperties();
            m_PhysicalDeviceFeatures = m_PhysicalDevice.getFeatures();
            m_PhysicalDeviceMemoryProperties = m_PhysicalDevice.getMemoryProperties();
            m_QueueFamilyIndices = FindQueueFamilies(m_PhysicalDevice, surface);
            break;
         }
      }
      if (!m_PhysicalDevice) {
         throw std::runtime_error("failed to find a suitable GPU!");
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

      for (uint32_t queueFamily : uniqueQueueFamilies) {
         deviceQueueCIs.emplace_back(
            vk::DeviceQueueCreateFlags {},
            queueFamily,
            1,
            &queuePriority
         );
      }

      std::vector<const char*> deviceExtensions = GetRequiredDeviceExtensions();

      m_EnabledPhysicalDeviceFeatures = GetRequiredPhysicalDeviceFeatures(m_PhysicalDeviceFeatures);

      vk::DeviceCreateInfo ci = {
         {},
         static_cast<uint32_t>(deviceQueueCIs.size())     /*queueCreateInfoCount*/,
         deviceQueueCIs.data()                            /*pQueueCreateInfos*/,
         0                                                /*enabledLayerCount*/,
         nullptr                                          /*ppEnabledLayerNames*/,
         static_cast<uint32_t>(deviceExtensions.size())   /*enabledExtensionCount*/,
         deviceExtensions.data()                          /*ppEnabledExtensionNames*/,
         &m_EnabledPhysicalDeviceFeatures                 /*pEnabledFeatures*/
      };
      ci.pNext = GetRequiredPhysicalDeviceFeaturesEXT();

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


   vk::Queue VulkanDevice::GetGraphicsQueue() const {
      return m_GraphicsQueue;
   }


   vk::Queue VulkanDevice::GetPresentQueue() const {
      return m_PresentQueue;
   }


   void VulkanDevice::SubmitSingleTimeCommands(const std::function<void(vk::CommandBuffer)>& action) {
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
      GetGraphicsQueue().submit(si, nullptr);
      GetGraphicsQueue().waitIdle();
      m_Device.freeCommandBuffers(m_CommandPool, commandBuffers);
   }


   vk::ShaderModule VulkanDevice::CreateShaderModule(std::vector<char> code) {
      PKZL_CORE_ASSERT(m_Device, "Attempted to use null device!");
      vk::ShaderModuleCreateInfo ci = {
         {},
         code.size(),
         reinterpret_cast<const uint32_t*>(code.data())
      };

      return m_Device.createShaderModule(ci);
   }


   void VulkanDevice::DestroyShaderModule(vk::ShaderModule& module) {
      if (m_Device && module) {
         m_Device.destroy(module);
         module = nullptr;
      }
   }

}
