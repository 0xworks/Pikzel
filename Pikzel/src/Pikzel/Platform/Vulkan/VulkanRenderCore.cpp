#include "vkpch.h"
#include "VulkanRenderCore.h"
#include "VulkanGraphicsContext.h"

#include "Pikzel/Core/Window.h"

#include <GLFW/glfw3.h>

#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_vulkan.h>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace Pikzel {

   std::unique_ptr<RenderCore> RenderCore::Create() {
      return std::make_unique<VulkanRenderCore>();
   }


   static void GLFWErrorCallback(int error, const char* description) {
      PKZL_CORE_LOG_ERROR("GLFW Error ({0}): {1}", error, description);
   }


   VKAPI_ATTR VkBool32 VKAPI_CALL VulkanErrorCallback(
      VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
      VkDebugUtilsMessageTypeFlagsEXT messageType,
      const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
      void* pUserData
   ) {
      switch (messageSeverity) {
         case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            PKZL_CORE_LOG_TRACE("VULKAN VALIDATION: {0}", pCallbackData->pMessage);
            break;
         case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            PKZL_CORE_LOG_INFO("VULKAN VALIDATION: {0}", pCallbackData->pMessage);
            break;
         case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            PKZL_CORE_LOG_WARN("VULKAN VALIDATION: {0}", pCallbackData->pMessage);
            break;
         case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            PKZL_CORE_LOG_ERROR("VULKAN VALIDATION: {0}", pCallbackData->pMessage);
            break;
      }
      return VK_FALSE;
   }


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
            throw std::runtime_error(msg);
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
            throw std::runtime_error(msg);
         }
      }
   }


   VulkanRenderCore::VulkanRenderCore() {
      PKZL_CORE_LOG_INFO("Vulkan RenderCore");
      if (!glfwInit()) {
         throw std::runtime_error("Could not initialize GLFW!");
      }
      glfwSetErrorCallback(GLFWErrorCallback);
      if (!glfwVulkanSupported()) {
         throw std::runtime_error("GLFW detects no support for Vulkan!");
      }
      CreateInstance();
   }


   VulkanRenderCore::~VulkanRenderCore() {
      DestroyInstance();
      glfwTerminate();
   }


   std::unique_ptr<GraphicsContext> VulkanRenderCore::CreateGraphicsContext(const Window& window) {
      return std::make_unique<VulkanGraphicsContext>(m_Instance, (GLFWwindow*)window.GetNativeWindow());
   }


   RendererAPI VulkanRenderCore::GetAPI() const {
      return RendererAPI::Vulkan;
   }


   std::vector<const char*> VulkanRenderCore::GetRequiredInstanceExtensions() {
      // No instance extensions required (yet...)
      return {};
   }


   void VulkanRenderCore::CreateInstance() {
      vk::DynamicLoader dl;
      PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
      VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

      std::vector<const char*> layers = {"VK_LAYER_LUNARG_monitor"};
#ifdef PKZL_DEBUG
      layers.push_back("VK_LAYER_KHRONOS_validation");
#endif
      CheckLayerSupport(layers);

      std::vector<const char*> extensions = GetRequiredInstanceExtensions();

      uint32_t glfwExtensionCount = 0;
      const char** glfwExtensions;
      glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

      extensions.insert(extensions.end(), glfwExtensions, glfwExtensions + glfwExtensionCount);

#ifdef PKZL_DEBUG
      extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

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