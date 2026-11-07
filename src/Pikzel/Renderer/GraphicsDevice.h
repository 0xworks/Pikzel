#pragma once

#include <memory>

namespace nri {
struct Device;
struct Queue;
}

namespace Pikzel {

struct NriInterface;

struct GraphicsDeviceSettings {
   // Validation catches incorrect API usage at a significant CPU cost, so it is on by
   // default only in debug builds. API validation needs the Vulkan validation layers,
   // which come with the Vulkan SDK; NRI validation is built into NRI.
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
class GraphicsDevice {
public:
   explicit GraphicsDevice(const GraphicsDeviceSettings& settings = {});
   ~GraphicsDevice();

   GraphicsDevice(const GraphicsDevice&) = delete;
   GraphicsDevice& operator=(const GraphicsDevice&) = delete;
   GraphicsDevice(GraphicsDevice&&) = delete;
   GraphicsDevice& operator=(GraphicsDevice&&) = delete;

   // Blocks until the GPU has finished all submitted work. Needed before destroying or
   // recreating objects the GPU may still be using; too slow to call every frame.
   void WaitIdle() const;

   // Renderer internal: used by the other renderer classes to call into NRI.
   const NriInterface& GetNri() const;
   nri::Device& GetDevice() const;
   nri::Queue& GetGraphicsQueue() const;

private:
   // Held by pointer so that this header does not need NRI's definitions.
   std::unique_ptr<NriInterface> m_Nri;
   nri::Device* m_Device = nullptr;
   nri::Queue* m_GraphicsQueue = nullptr;
};

}
