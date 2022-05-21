#include "VulkanRenderCore.h"

#include "VulkanBuffer.h"
#include "VulkanComputeContext.h"
#include "VulkanGraphicsContext.h"
#include "VulkanMemoryAllocator.hpp"
#include "VulkanPipeline.h"
#include "VulkanTexture.h"
#include "VulkanUtility.h"

#include "imgui_impl_vulkan.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

#if defined(PKZL_PLATFORM_WINDOWS)
   #define PLATFORM_API __declspec(dllexport)
#else
   #define PLATFORM_API
#endif

namespace Pikzel {

   extern "C" PLATFORM_API IRenderCore* CDECL CreateRenderCore(const Window* window) {
      PKZL_CORE_ASSERT(window, "Window is null in call to CreateRenderCore!");
      return new VulkanRenderCore {*window};
   }


   VKAPI_ATTR VkBool32 VKAPI_CALL VulkanErrorCallback(
      VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
      VkDebugUtilsMessageTypeFlagsEXT messageType,
      const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
      void* pUserData
   ) {
      switch (messageSeverity) {
         case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            PKZL_CORE_LOG_TRACE("Vulkan Debug: {0}", pCallbackData->pMessage);
            break;
         case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            PKZL_CORE_LOG_INFO("Vulkan Debug: {0}", pCallbackData->pMessage);
            break;
         case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            PKZL_CORE_LOG_WARN("Vulkan Debug: {0}", pCallbackData->pMessage);
            break;
         case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            PKZL_CORE_LOG_ERROR("Vulkan Debug: {0}", pCallbackData->pMessage);
            break;
      }
      return VK_FALSE;
   }


   VulkanRenderCore::VulkanRenderCore(const Window& window) {
      CreateInstance();

      // temporary surface just to ensure that device is created with correct properties.
      // real surface is created later (in GraphicsContext)
      VkSurfaceKHR surface;
      if (glfwCreateWindowSurface(m_Instance, static_cast<GLFWwindow*>(window.GetNativeWindow()), nullptr, &surface) != VK_SUCCESS) {
         throw std::runtime_error {"failed to create window surface!"};
      }
      m_Device = std::make_shared<VulkanDevice>(m_Instance, surface);
      m_Instance.destroy(surface);

      VulkanMemoryAllocator::Init(m_Instance, m_Device->GetVkPhysicalDevice(), m_Device->GetVkDevice());
   }


   VulkanRenderCore::~VulkanRenderCore() {
      VulkanMemoryAllocator::Get().destroy();
      m_Device = nullptr;
      DestroyInstance();
   }


   void VulkanRenderCore::UploadImGuiFonts() {
      m_Device->SubmitSingleTimeCommands(m_Device->GetTransferQueue(), [] (vk::CommandBuffer commandBuffer) {
         if (!ImGui_ImplVulkan_CreateFontsTexture(commandBuffer)) {
            throw std::runtime_error {"failed to create ImGui font textures!"};
         }
      });
      ImGui_ImplVulkan_DestroyFontUploadObjects();
   }


   void VulkanRenderCore::SetViewport(const uint32_t, const uint32_t, const uint32_t, const uint32_t) {
      // Don't need to do anything. Vulkan swapchain resizing will deal with it
   }


   std::unique_ptr<ComputeContext> VulkanRenderCore::CreateComputeContext() {
      return std::make_unique<VulkanComputeContext>(m_Device);
   }


   std::unique_ptr<GraphicsContext> VulkanRenderCore::CreateGraphicsContext(const Window& window) {
      return std::make_unique<VulkanWindowGC>(m_Device, window);
   }


   std::unique_ptr<VertexBuffer> VulkanRenderCore::CreateVertexBuffer(const BufferLayout& layout, const uint32_t size) {
      return std::make_unique<VulkanVertexBuffer>(m_Device, layout, size);
   }


   std::unique_ptr<VertexBuffer> VulkanRenderCore::CreateVertexBuffer(const BufferLayout& layout, const uint32_t size, const void* data) {
      return std::make_unique<VulkanVertexBuffer>(m_Device, layout, size, data);
   }


   std::unique_ptr<IndexBuffer> VulkanRenderCore::CreateIndexBuffer(const uint32_t count, const uint32_t* indices) {
      return std::make_unique<VulkanIndexBuffer>(m_Device, count, indices);
   }


   std::unique_ptr<UniformBuffer> VulkanRenderCore::CreateUniformBuffer(const uint32_t size) {
      return std::make_unique<VulkanUniformBuffer>(m_Device, size);
   }


   std::unique_ptr<UniformBuffer> VulkanRenderCore::CreateUniformBuffer(const uint32_t size, const void* data) {
      return std::make_unique<VulkanUniformBuffer>(m_Device, size, data);
   }


   std::unique_ptr<Framebuffer> VulkanRenderCore::CreateFramebuffer(const FramebufferSettings& settings) {
      return std::make_unique<VulkanFramebuffer>(m_Device, settings);
   }


   std::unique_ptr<Texture> VulkanRenderCore::CreateTexture(const TextureSettings& settings) {
      vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment;
      vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor;
      if (IsDepthFormat(settings.format)) {
         usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
         aspect = vk::ImageAspectFlagBits::eDepth;
      }
      if (settings.imageStorage) {
         usage |= vk::ImageUsageFlagBits::eStorage;
      }

      switch (settings.textureType) {
         case TextureType::Texture2D:        return std::make_unique<VulkanTexture2D>(m_Device, settings, usage, aspect);
         case TextureType::Texture2DArray:   return std::make_unique<VulkanTexture2DArray>(m_Device, settings, usage, aspect);
         case TextureType::TextureCube:      return std::make_unique<VulkanTextureCube>(m_Device, settings, usage, aspect);
         case TextureType::TextureCubeArray: return std::make_unique<VulkanTextureCubeArray>(m_Device, settings, usage, aspect);
      }
      PKZL_CORE_ASSERT(false, "TextureType not supported!");
      return nullptr;
   }


   std::vector<const char*> VulkanRenderCore::GetRequiredInstanceExtensions() {
      std::vector<const char*> extensions;
      uint32_t glfwExtensionCount = 0;
      const char** glfwExtensions;
      glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

      extensions.reserve(extensions.size() + glfwExtensionCount + 1);
      extensions.insert(extensions.end(), glfwExtensions, glfwExtensions + glfwExtensionCount);

#ifdef PKZL_DEBUG
      extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

      return extensions;
   }


   void VulkanRenderCore::CreateInstance() {
      vk::DynamicLoader dl;
      PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
      VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

      std::vector<const char*> layers = {/*"VK_LAYER_LUNARG_monitor"*/};   // Note: ImGui viewports seem to knobble VK_LAYER_LUNARG_monitor
#ifdef PKZL_DEBUG
      layers.push_back("VK_LAYER_KHRONOS_validation");
#endif
      CheckLayerSupport(layers);

      std::vector<const char*> extensions = GetRequiredInstanceExtensions();

      CheckInstanceExtensionSupport(extensions);

      vk::ApplicationInfo appInfo = {
         "Pikzel Vulkan RenderCore",
         VK_MAKE_API_VERSION(0, 1, 0, 0),
         "Pikzel",
         VK_MAKE_API_VERSION(0, 1, 0, 0),
         VK_API_VERSION_1_3
      };

      vk::DebugUtilsMessengerCreateInfoEXT debugCI {
         {},
         {vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError},
         {vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance},
         VulkanErrorCallback,
         nullptr
      };

      vk::InstanceCreateInfo instanceCI = {
         {},
         &appInfo,
         static_cast<uint32_t>(layers.size()),
         layers.data(),
         static_cast<uint32_t>(extensions.size()),
         extensions.data()
      };

#ifdef PKZL_DEBUG
      instanceCI.pNext = &debugCI;
#endif

      m_Instance = vk::createInstance(instanceCI);
      VULKAN_HPP_DEFAULT_DISPATCHER.init(m_Instance);

#ifdef PKZL_DEBUG
      m_DebugUtilsMessengerEXT = m_Instance.createDebugUtilsMessengerEXT(debugCI);
#endif
   }


   void VulkanRenderCore::DestroyInstance() {
      if (m_Instance) {
         if (m_DebugUtilsMessengerEXT) {
            m_Instance.destroy(m_DebugUtilsMessengerEXT);
            m_DebugUtilsMessengerEXT = nullptr;
         }
         m_Instance.destroy();
         m_Instance = nullptr;
      }
   }

}
