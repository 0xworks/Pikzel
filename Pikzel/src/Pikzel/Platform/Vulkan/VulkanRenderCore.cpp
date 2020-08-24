#include "vkpch.h"
#include "VulkanRenderCore.h"

#include "VulkanBuffer.h"
#include "VulkanImageGC.h"
#include "VulkanUtility.h"
#include "VulkanWindowGC.h"

#include "Pikzel/Core/Window.h"

#include <GLFW/glfw3.h>

#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_vulkan.h>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace Pikzel {

   std::unique_ptr<RenderCore> RenderCore::Create() {
      return std::make_unique<VulkanRenderCore>();
   }


   VulkanRenderCore::VulkanRenderCore() {
      if (!glfwInit()) {
         throw std::runtime_error("Could not initialize GLFW!");
      }
      glfwSetErrorCallback([] (int error, const char* description) {
         PKZL_CORE_LOG_ERROR("GLFW Error ({0}): {1}", error, description);
      });
      if (!glfwVulkanSupported()) {
         throw std::runtime_error("GLFW detects no support for Vulkan!");
      }
      CreateInstance();
      m_Device = std::make_shared<VulkanDevice>(m_Instance, nullptr);
   }


   VulkanRenderCore::~VulkanRenderCore() {
      m_Device = nullptr;
      DestroyInstance();
      glfwTerminate();
   }


   RendererAPI VulkanRenderCore::GetAPI() const {
      return RendererAPI::Vulkan;
   }


   std::unique_ptr<Pikzel::Buffer> VulkanRenderCore::CreateBuffer(const uint64_t size) {
      return std::make_unique<VulkanBuffer>(m_Device, size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
   }


   std::unique_ptr<Image> VulkanRenderCore::CreateImage(const ImageSettings& settings /*= ImageSettings()*/) {
      std::unique_ptr<VulkanImage> image = std::make_unique<VulkanImage>(m_Device, settings.Width, settings.Height, 1, vk::SampleCountFlagBits::e1, vk::Format::eB8G8R8A8Unorm, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eColorAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal);
      image->CreateImageView(image->GetFormat(), vk::ImageAspectFlagBits::eColor, image->GetMIPLevels());
      image->CreateSampler();
      return image;
   }


   std::unique_ptr<GraphicsContext> VulkanRenderCore::CreateGraphicsContext(Window& window) {
      //
      // TODO: here you somehow need to throw everything away and start again
      //       e.g. the device you currently have doesnt necessarily support windows surface...
      return std::make_unique<VulkanWindowGC>(m_Device, (GLFWwindow*)window.GetNativeWindow());
   }


   std::unique_ptr<Pikzel::GraphicsContext> VulkanRenderCore::CreateGraphicsContext(Image& image) {
      return std::make_unique<VulkanImageGC>(m_Device, dynamic_cast<VulkanImage*>(&image));
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