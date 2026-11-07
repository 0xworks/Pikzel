#include "Pikzel/Renderer/GraphicsDevice.h"
#include "Pikzel/Core/Log.h"
#include "Pikzel/Renderer/Nri.h"

#include <format>
#include <stdexcept>

namespace Pikzel {

namespace {

nri::GraphicsAPI ToNri(const GraphicsApi api) {
   switch (api) {
      case GraphicsApi::D3D12:
         return nri::GraphicsAPI::D3D12;
      case GraphicsApi::Vulkan:
      default:
         return nri::GraphicsAPI::VK;
   }
}


// Forwards NRI's diagnostics (including validation messages) to the engine log.
void NriMessageCallback(const nri::Message messageType, const char* file, const uint32_t line, const char* message, void*) {
   switch (messageType) {
      case nri::Message::INFO:
         PKZL_CORE_LOG_DEBUG("NRI: {}", message);
         break;
      case nri::Message::WARNING:
         PKZL_CORE_LOG_WARN("NRI: {}", message);
         break;
      case nri::Message::ERROR:
      default:
         PKZL_CORE_LOG_ERROR("NRI: {} ({}:{})", message, file, line);
         break;
   }
}

}


std::string_view ToString(const GraphicsApi api) {
   switch (api) {
      case GraphicsApi::Vulkan:
         return "Vulkan";
      case GraphicsApi::D3D12:
         return "D3D12";
   }
   return "Unknown";
}


bool IsGraphicsApiAvailable(const GraphicsApi api) {
   switch (api) {
      case GraphicsApi::Vulkan:
         return true;
      case GraphicsApi::D3D12:
#ifdef PKZL_HAS_D3D12
         return true;
#else
         return false;
#endif
   }
   return false;
}


GraphicsDevice::GraphicsDevice(const GraphicsDeviceSettings& settings)
: m_Nri(std::make_unique<NriInterface>()), m_Api(settings.api) {
   // NRI would also fail for a backend it was built without, but with a less helpful message.
   if (!IsGraphicsApiAvailable(settings.api)) {
      throw std::runtime_error(std::format("{} support was not compiled into the engine (configure with PKZL_ENABLE_D3D12=ON)", ToString(settings.api)));
   }

   // NRI returns adapters sorted with the most capable first (discrete GPUs, then by
   // video memory), so the first adapter that supports the API is a sensible default.
   nri::AdapterDesc adapterDesc = {};
   uint32_t adapterCount = 1;
   CheckNri(nri::nriEnumerateAdapters(&adapterDesc, adapterCount, ToNri(settings.api)), "nriEnumerateAdapters");
   if (adapterCount == 0) {
      throw std::runtime_error(std::format("No GPU with {} support was found", ToString(settings.api)));
   }

   nri::DeviceCreationDesc deviceCreationDesc = {};
   deviceCreationDesc.graphicsAPI = ToNri(settings.api);
   deviceCreationDesc.adapterDesc = &adapterDesc;
   deviceCreationDesc.callbackInterface.MessageCallback = NriMessageCallback;
   deviceCreationDesc.enableGraphicsAPIValidation = settings.isApiValidationEnabled;
   deviceCreationDesc.enableNRIValidation = settings.isNriValidationEnabled;

   // Must match the register shifts given to the shader compiler (see cmake/Shaders.cmake).
   // Only the Vulkan backend reads these; D3D12 has a separate namespace per register kind.
   deviceCreationDesc.vkBindingOffsets = {
      .sRegister = PKZL_VK_BINDING_OFFSET_SAMPLER,
      .tRegister = PKZL_VK_BINDING_OFFSET_SHADER_RESOURCE,
      .bRegister = PKZL_VK_BINDING_OFFSET_CONSTANT_BUFFER,
      .uRegister = PKZL_VK_BINDING_OFFSET_READ_WRITE,
   };

   CheckNri(nri::nriCreateDevice(deviceCreationDesc, m_Device), "nriCreateDevice");

   // From here on the constructor can still throw, and a throwing constructor does not
   // run the destructor, so the device is released explicitly on failure.
   try {
      // NriInterface inherits from every interface table the engine uses, so each table is
      // written into the matching base-class part of m_Nri. NRI_INTERFACE expands to the
      // interface's name and size; the size guards against mismatched headers.
      CheckNri(nri::nriGetInterface(*m_Device, NRI_INTERFACE(nri::CoreInterface), static_cast<nri::CoreInterface*>(m_Nri.get())), "Get CoreInterface");
      CheckNri(nri::nriGetInterface(*m_Device, NRI_INTERFACE(nri::HelperInterface), static_cast<nri::HelperInterface*>(m_Nri.get())), "Get HelperInterface");
      CheckNri(nri::nriGetInterface(*m_Device, NRI_INTERFACE(nri::SwapChainInterface), static_cast<nri::SwapChainInterface*>(m_Nri.get())), "Get SwapChainInterface");

      // NRI creates one graphics queue by default. It can also present to a window.
      CheckNri(m_Nri->GetQueue(*m_Device, nri::QueueType::GRAPHICS, 0, m_GraphicsQueue), "GetQueue");
   } catch (...) {
      nri::nriDestroyDevice(m_Device);
      throw;
   }

   const auto& desc = m_Nri->GetDeviceDesc(*m_Device);
   PKZL_CORE_LOG_INFO("Graphics device: {} ({}, {} MiB video memory)", desc.adapterDesc.name, nri::nriGetGraphicsAPIString(desc.graphicsAPI), desc.adapterDesc.videoMemorySize / (1024 * 1024));
}


GraphicsDevice::~GraphicsDevice() {
   // Destructors must not throw, so the result is logged rather than passed to CheckNri.
   if (const auto result = m_Nri->DeviceWaitIdle(m_Device); result != nri::Result::SUCCESS) {
      PKZL_CORE_LOG_ERROR("DeviceWaitIdle failed during shutdown: {}", ToString(result));
   }
   nri::nriDestroyDevice(m_Device);
}


const NriInterface& GraphicsDevice::GetNri() const {
   return *m_Nri;
}


nri::Device& GraphicsDevice::GetDevice() const {
   return *m_Device;
}


nri::Queue& GraphicsDevice::GetGraphicsQueue() const {
   return *m_GraphicsQueue;
}


void GraphicsDevice::WaitIdle() const {
   CheckNri(m_Nri->DeviceWaitIdle(m_Device), "DeviceWaitIdle");
}


GraphicsApi GraphicsDevice::GetApi() const {
   return m_Api;
}

}
