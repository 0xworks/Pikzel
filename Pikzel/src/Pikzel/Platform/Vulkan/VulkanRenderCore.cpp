#include "vkpch.h"
#include "VulkanRenderCore.h"

#include "VulkanBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanUtility.h"
#include "VulkanWindowGC.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace Pikzel {

   RenderCore::API RenderCore::s_API = RenderCore::API::Vulkan;

   std::unique_ptr<IRenderCore> CreateRenderCore(const Window& window) {
      return std::make_unique<VulkanRenderCore>(window);
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
         throw std::runtime_error("failed to create window surface!");
      }
      m_Device = std::make_shared<VulkanDevice>(m_Instance, surface);
      m_Instance.destroy(surface);
   }


   VulkanRenderCore::~VulkanRenderCore() {
      m_Device = nullptr;
      DestroyInstance();
   }


   void VulkanRenderCore::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
      // TODO: This needs to just set something for later use (e.g. in cmdBuffer.setViewport(...)? )
   }


   void VulkanRenderCore::SetClearColor(const glm::vec4& color) {
      // TODO: handle this.  Isn't clear color set when you create the render pass?
   }


   void VulkanRenderCore::Clear() {
      // TODO: something like get a command buffer from somewhere, and then cmdBuffer.clear(something)?
   }


   std::unique_ptr<GraphicsContext> VulkanRenderCore::CreateGraphicsContext(const Window& window) {
      return std::make_unique<VulkanWindowGC>(m_Device, static_cast<GLFWwindow*>(window.GetNativeWindow()));
   }


   std::unique_ptr<VertexBuffer> VulkanRenderCore::CreateVertexBuffer(uint32_t size) {
      return std::make_unique<VulkanVertexBuffer>(m_Device, size);
   }


   std::unique_ptr<VertexBuffer> VulkanRenderCore::CreateVertexBuffer(float* vertices, uint32_t size) {
      return std::make_unique<VulkanVertexBuffer>(m_Device, vertices, size);
   }


   std::unique_ptr<IndexBuffer> VulkanRenderCore::CreateIndexBuffer(uint32_t* indices, uint32_t count) {
      return std::make_unique<VulkanIndexBuffer>(m_Device, indices, count);
   }


   std::unique_ptr<Pipeline> VulkanRenderCore::CreatePipeline(const Window& window, const PipelineSettings& settings) {
      return std::make_unique<VulkanPipeline>(m_Device, window, settings);
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
         VK_MAKE_VERSION(1, 0, 0),
         "Pikzel",
         VK_MAKE_VERSION(1, 0, 0),
         VK_API_VERSION_1_2
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
