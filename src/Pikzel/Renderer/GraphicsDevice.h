#pragma once

#include <cstdint>
#include <memory>
#include <string_view>

namespace nri {
struct DescriptorPool;
struct Device;
struct Queue;
}

namespace Pikzel {

struct NriInterface;

// The low-level graphics API the renderer runs on. Vulkan is always available. D3D12 is
// available on Windows when the engine is built with PKZL_ENABLE_D3D12.
enum class GraphicsApi : uint8_t {
   Vulkan,
   D3D12,
};

std::string_view ToString(GraphicsApi api);

// Whether support for the given API was compiled into the engine. This says nothing about
// whether the computer's GPU and driver support it; creating a device finds that out.
bool IsGraphicsApiAvailable(GraphicsApi api);

struct GraphicsDeviceSettings {
   GraphicsApi api = GraphicsApi::Vulkan;

   // Validation catches incorrect API usage at a significant CPU cost, so it is on by
   // default only in debug builds. NRI validation is built into NRI. API validation needs
   // extra components: for Vulkan the validation layers, which come with the Vulkan SDK;
   // for D3D12 the debug layer, which comes with the Windows "Graphics Tools" optional
   // feature. Without them the device is still created, just without API validation.
#ifdef NDEBUG
   bool isApiValidationEnabled = false;
   bool isNriValidationEnabled = false;
#else
   bool isApiValidationEnabled = true;
   bool isNriValidationEnabled = true;
#endif
};

// The GPU device: the root object from which all other GPU objects are created.
//
// GraphicsDevice owns the NRI device, the interface tables used to call into NRI, and the
// graphics queue that command buffers are submitted to. It does not need a window, so it
// can also be used for off-screen work; presenting to a window is the job of SwapChain.
//
// Every GPU object created from a device must be destroyed before the device itself.
// Engine classes that own GPU objects keep a reference to their GraphicsDevice, which
// makes that ordering the natural one in C++ (members and locals are destroyed in reverse
// order of construction).
//
// The graphics API is chosen when the device is created and is fixed for its lifetime.
// Almost nothing above the renderer needs to know which one is in use; the exception is
// loading compiled shaders, which differ per API (see Shader).
//
// The device also owns the descriptor pool that every DescriptorSet is allocated from.
// There is a single pool because D3D12 can only have one shader-visible descriptor heap
// of each kind bound at a time, and NRI backs a pool with such heaps: descriptor sets used
// together in one command buffer must come from the same pool. See DescriptorSet.
class GraphicsDevice {
public:
   // Throws std::runtime_error if the requested API was not compiled in, or if no GPU
   // supports it.
   explicit GraphicsDevice(const GraphicsDeviceSettings& settings = {});
   ~GraphicsDevice();

   GraphicsDevice(const GraphicsDevice&) = delete;
   GraphicsDevice& operator=(const GraphicsDevice&) = delete;
   GraphicsDevice(GraphicsDevice&&) = delete;
   GraphicsDevice& operator=(GraphicsDevice&&) = delete;

   // Blocks until the GPU has finished all submitted work. Needed before destroying or
   // recreating objects the GPU may still be using; too slow to call every frame.
   void WaitIdle() const;

   GraphicsApi GetApi() const;

   // Renderer internal: used by the other renderer classes to call into NRI.
   const NriInterface& GetNri() const;
   nri::Device& GetDevice() const;
   nri::Queue& GetGraphicsQueue() const;
   nri::DescriptorPool& GetDescriptorPool() const;

private:
   // Held by pointer so that this header does not need NRI's definitions.
   std::unique_ptr<NriInterface> m_Nri;
   nri::Device* m_Device = nullptr;
   nri::Queue* m_GraphicsQueue = nullptr;
   nri::DescriptorPool* m_DescriptorPool = nullptr;
   GraphicsApi m_Api = GraphicsApi::Vulkan;
};

}
