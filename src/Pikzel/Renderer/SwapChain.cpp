#include "Pikzel/Renderer/SwapChain.h"
#include "Pikzel/Core/Log.h"
#include "Pikzel/Platform/Window.h"
#include "Pikzel/Renderer/GraphicsDevice.h"
#include "Pikzel/Renderer/Nri.h"

#include <cassert>
#include <format>
#include <stdexcept>
#include <type_traits>
#include <variant>

namespace Pikzel {

namespace {

nri::Window ToNriWindow(const NativeWindowHandle& handle) {
   nri::Window window = {};
   std::visit(
      [&window](const auto& native) {
         using T = std::decay_t<decltype(native)>;
         if constexpr (std::is_same_v<T, Win32WindowHandle>) {
            window.windows.hwnd = native.hwnd;
         } else if constexpr (std::is_same_v<T, X11WindowHandle>) {
            window.x11.dpy = native.display;
            window.x11.window = native.window;
         } else if constexpr (std::is_same_v<T, WaylandWindowHandle>) {
            window.wayland.display = native.display;
            window.wayland.surface = native.surface;
         }
      },
      handle
   );
   return window;
}


// The sRGB view format for a swap chain's UNORM format. See CreateSwapChain().
nri::Format GetSrgbViewFormat(const nri::Format format) {
   switch (format) {
      case nri::Format::RGBA8_UNORM:
      case nri::Format::RGBA8_SRGB:
         return nri::Format::RGBA8_SRGB;
      case nri::Format::BGRA8_UNORM:
      case nri::Format::BGRA8_SRGB:
         return nri::Format::BGRA8_SRGB;
      default:
         throw std::runtime_error(std::format("Swap chain format {} has no sRGB equivalent", nri::nriGetFormatProps(format)->name));
   }
}


}


SwapChain::SwapChain(GraphicsDevice& device, const Window& window, const SwapChainSettings& settings)
: m_Device(device), m_Window(window), m_Settings(settings) {
   assert(settings.framesInFlight > 0);
   if (settings.depthFormat != Format::Unknown && !IsDepthFormat(settings.depthFormat)) {
      throw std::invalid_argument("SwapChainSettings::depthFormat must be a depth format");
   }

   // Members are created one at a time, and any step can throw. A throwing constructor
   // does not run the destructor, so clean up whatever exists before rethrowing. NRI's
   // Destroy functions accept null, which keeps the clean-up code simple.
   try {
      const auto& nri = device.GetNri();

      // A timeline fence: its value only ever increases, and the CPU can wait for it to
      // reach a given value.
      CheckNri(nri.CreateFence(device.GetDevice(), {.initialValue = 0}, m_FrameFence), "CreateFence");

      m_Frames.resize(settings.framesInFlight);
      for (auto& frame : m_Frames) {
         CheckNri(nri.CreateCommandAllocator(device.GetGraphicsQueue(), frame.commandAllocator), "CreateCommandAllocator");
         CheckNri(nri.CreateCommandBuffer(*frame.commandAllocator, frame.commandBuffer), "CreateCommandBuffer");
      }

      const auto size = window.GetFramebufferSize();
      CreateSwapChain(size.width, size.height);
   } catch (...) {
      DestroyAll();
      throw;
   }
}


SwapChain::~SwapChain() {
   // The GPU may still be working on the last few frames, and their resources must not be
   // destroyed under it. Destructors must not throw, so failure is only logged.
   if (const auto result = m_Device.GetNri().QueueWaitIdle(&m_Device.GetGraphicsQueue()); result != nri::Result::SUCCESS) {
      PKZL_CORE_LOG_ERROR("QueueWaitIdle failed during swap chain destruction: {}", ToString(result));
   }
   DestroyAll();
}


std::optional<Frame> SwapChain::BeginFrame() {
   assert(!m_IsFrameInProgress && "BeginFrame() called twice without EndFrame()");

   const auto& nri = m_Device.GetNri();

   const auto windowSize = m_Window.GetFramebufferSize();
   if (windowSize.width == 0 || windowSize.height == 0) {
      return std::nullopt;
   }
   if (m_IsOutOfDate || windowSize.width != m_Width || windowSize.height != m_Height) {
      RecreateSwapChain(windowSize.width, windowSize.height);
   }

   const uint64_t framesInFlight = m_Frames.size();

   // Frame pacing: wait until frame (m_FrameIndex - framesInFlight) has been presented. See
   // "Frame pacing" in SwapChain.h. The two APIs wait differently:
   //  - Vulkan waits for a specific present ID (frame N is presented with ID N + 1), which
   //    must have been queued on this swap chain, so the first frames after creation skip
   //    the wait.
   //  - D3D12 ignores the ID and waits on a frame latency object, which starts out allowing
   //    framesInFlight frames and must be waited on every frame, from the first. Skipping
   //    waits would let extra frames queue up.
   const uint64_t framesSinceCreation = m_FrameIndex - m_FirstFrameIndex;
   const bool isPresentQueued = framesSinceCreation >= framesInFlight;
   if (isPresentQueued || m_Device.GetApi() == GraphicsApi::D3D12) {
      const uint64_t presentId = isPresentQueued ? m_FrameIndex - framesInFlight + 1 : 1; // any non-zero ID for D3D12
      const auto presentWaitResult = nri.WaitForPresent(*m_SwapChain, presentId);
      if (presentWaitResult == nri::Result::OUT_OF_DATE) {
         m_IsOutOfDate = true;
         return std::nullopt;
      }
      // UNSUPPORTED means the device has no waitable swap chain; frames are then paced by
      // the fence and acquire alone.
      if (presentWaitResult != nri::Result::UNSUPPORTED) {
         CheckNri(presentWaitResult, "WaitForPresent");
      }
   }

   // Wait until the GPU has finished the frame that last used this frame's resources:
   // frame (m_FrameIndex - framesInFlight), which signalled the fence with one more than
   // its index.
   if (m_FrameIndex >= framesInFlight) {
      nri.Wait(*m_FrameFence, m_FrameIndex - framesInFlight + 1);
   }
   const FrameResources& frame = m_Frames[m_FrameIndex % framesInFlight];

   m_AcquireSemaphore = m_BackBuffers[m_FrameIndex % m_BackBuffers.size()].acquireSemaphore;
   const auto acquireResult = nri.AcquireNextTexture(*m_SwapChain, *m_AcquireSemaphore, m_BackBufferIndex);
   if (acquireResult == nri::Result::OUT_OF_DATE) {
      m_IsOutOfDate = true;
      return std::nullopt;
   }
   CheckNri(acquireResult, "AcquireNextTexture");

   const BackBuffer& backBuffer = m_BackBuffers[m_BackBufferIndex];

   // The GPU has finished with this allocator's previous commands (see the wait above),
   // so its memory can be reused for this frame's commands. The device's descriptor pool
   // is attached to the command buffer, so that descriptor sets can be bound in it.
   nri.ResetCommandAllocator(*frame.commandAllocator);
   CheckNri(nri.BeginCommandBuffer(*frame.commandBuffer, &m_Device.GetDescriptorPool()), "BeginCommandBuffer");

   nri::TextureBarrierDesc barriers[2] = {};
   uint32_t barrierCount = 0;

   // Transition the back buffer from whatever the presentation engine left it in to a
   // color attachment. The old contents are not needed, so the "before" layout is left as
   // UNDEFINED, which lets the GPU discard them.
   nri::TextureBarrierDesc& toColorAttachment = barriers[barrierCount++];
   toColorAttachment.texture = backBuffer.texture;
   toColorAttachment.after = {nri::AccessBits::COLOR_ATTACHMENT, nri::Layout::COLOR_ATTACHMENT, nri::StageBits::COLOR_ATTACHMENT};
   toColorAttachment.mipNum = 1;
   toColorAttachment.layerNum = 1;

   // The depth buffer stays in the depth attachment layout from frame to frame, but this
   // frame's depth writes must not start until the previous frame's have finished (a
   // write-after-write hazard), so a barrier orders them. A new depth buffer has nothing to
   // wait for, and moves from UNDEFINED to the depth attachment layout instead.
   if (m_DepthBuffer) {
      constexpr nri::AccessLayoutStage depthAttachment = {nri::AccessBits::DEPTH_STENCIL_ATTACHMENT, nri::Layout::DEPTH_STENCIL_ATTACHMENT, nri::StageBits::DEPTH_STENCIL_ATTACHMENT};

      nri::TextureBarrierDesc& depthBarrier = barriers[barrierCount++];
      depthBarrier.texture = &m_DepthBuffer->GetNriTexture();
      depthBarrier.before = m_IsDepthBufferNew ? nri::AccessLayoutStage{nri::AccessBits::NONE, nri::Layout::UNDEFINED, nri::StageBits::NONE} : depthAttachment;
      depthBarrier.after = depthAttachment;
      depthBarrier.mipNum = 1;
      depthBarrier.layerNum = 1;
      m_IsDepthBufferNew = false;
   }

   nri::BarrierDesc barrierDesc = {};
   barrierDesc.textures = barriers;
   barrierDesc.textureNum = barrierCount;
   nri.CmdBarrier(*frame.commandBuffer, barrierDesc);

   m_IsFrameInProgress = true;
   return Frame{
      .commandList = CommandList(nri, *frame.commandBuffer),
      .backBuffer = RenderTarget(*backBuffer.colorAttachment, m_Width, m_Height),
      .depthBuffer = m_DepthBuffer ? std::optional(m_DepthBuffer->GetRenderTarget()) : std::nullopt,
      .inFlightIndex = static_cast<uint32_t>(m_FrameIndex % framesInFlight),
   };
}


void SwapChain::EndFrame() {
   assert(m_IsFrameInProgress && "EndFrame() called without a successful BeginFrame()");

   const auto& nri = m_Device.GetNri();
   const FrameResources& frame = m_Frames[m_FrameIndex % m_Frames.size()];
   const BackBuffer& backBuffer = m_BackBuffers[m_BackBufferIndex];

   // Transition the back buffer to the layout the presentation engine expects. Nothing
   // later in this command buffer uses it, hence no "after" access or stages.
   nri::TextureBarrierDesc toPresent = {};
   toPresent.texture = backBuffer.texture;
   toPresent.before = {nri::AccessBits::COLOR_ATTACHMENT, nri::Layout::COLOR_ATTACHMENT, nri::StageBits::COLOR_ATTACHMENT};
   toPresent.after = {nri::AccessBits::NONE, nri::Layout::PRESENT, nri::StageBits::NONE};
   toPresent.mipNum = 1;
   toPresent.layerNum = 1;

   nri::BarrierDesc barrierDesc = {};
   barrierDesc.textures = &toPresent;
   barrierDesc.textureNum = 1;
   nri.CmdBarrier(*frame.commandBuffer, barrierDesc);

   CheckNri(nri.EndCommandBuffer(*frame.commandBuffer), "EndCommandBuffer");

   // Rendering waits for the acquire semaphore, but only at the stage that writes color
   // attachments: vertex processing can start before the back buffer is available.
   const nri::FenceSubmitDesc waitFence = {
      .fence = m_AcquireSemaphore,
      .stages = nri::StageBits::COLOR_ATTACHMENT,
   };
   const nri::FenceSubmitDesc signalFences[] = {
      {.fence = backBuffer.releaseSemaphore},
      {.fence = m_FrameFence, .value = m_FrameIndex + 1},
   };

   nri::QueueSubmitDesc submitDesc = {};
   submitDesc.waitFences = &waitFence;
   submitDesc.waitFenceNum = 1;
   submitDesc.commandBuffers = &frame.commandBuffer;
   submitDesc.commandBufferNum = 1;
   submitDesc.signalFences = signalFences;
   submitDesc.signalFenceNum = static_cast<uint32_t>(std::size(signalFences));
   CheckNri(nri.QueueSubmit(m_Device.GetGraphicsQueue(), submitDesc), "QueueSubmit");

   // The present ID lets BeginFrame() wait for this frame to be presented (frame pacing).
   const auto presentResult = nri.QueuePresent(*m_SwapChain, *backBuffer.releaseSemaphore, m_FrameIndex + 1);
   if (presentResult == nri::Result::OUT_OF_DATE) {
      m_IsOutOfDate = true;
   } else {
      CheckNri(presentResult, "QueuePresent");
   }

   ++m_FrameIndex;
   m_IsFrameInProgress = false;
}


Format SwapChain::GetFormat() const {
   return m_Format;
}


Format SwapChain::GetDepthFormat() const {
   return m_Settings.depthFormat;
}


uint32_t SwapChain::GetFramesInFlight() const {
   return static_cast<uint32_t>(m_Frames.size());
}


void SwapChain::CreateSwapChain(const uint32_t width, const uint32_t height) {
   const auto& nri = m_Device.GetNri();

   nri::SwapChainDesc desc = {};
   desc.window = ToNriWindow(m_Window.GetNativeHandle());
   desc.queue = &m_Device.GetGraphicsQueue();
   desc.width = ToNriDim(width);
   desc.height = ToNriDim(height);
   // One more back buffer than frames in flight, so the CPU need not wait for the display
   // to release a back buffer while the GPU still has frames queued.
   desc.textureNum = static_cast<uint8_t>(m_Frames.size() + 1);
   // 8-bit, sRGB (gamma 2.2) output: standard dynamic range.
   desc.format = nri::SwapChainFormat::BT709_G22_8BIT;
   desc.flags = m_Settings.isVSyncEnabled ? nri::SwapChainBits::VSYNC : nri::SwapChainBits::ALLOW_TEARING;
   // WAITABLE enables WaitForPresent, used for frame pacing. NRI drops the flag if the
   // device does not support it, and WaitForPresent then reports UNSUPPORTED.
   desc.flags |= nri::SwapChainBits::WAITABLE;
   desc.queuedFrameNum = static_cast<uint8_t>(m_Frames.size());

   CheckNri(nri.CreateSwapChain(m_Device.GetDevice(), desc, m_SwapChain), "CreateSwapChain");
   m_Width = width;
   m_Height = height;
   m_IsOutOfDate = false;
   m_FirstFrameIndex = m_FrameIndex;

   // The graphics API may create more back buffers than requested, and picks the exact
   // format, so both are read back from the swap chain.
   uint32_t textureCount = 0;
   nri::Texture* const* textures = nri.GetSwapChainTextures(*m_SwapChain, textureCount);
   const nri::Format textureFormat = nri.GetTextureDesc(*textures[0]).format;

   // Shaders work with linear color values, because blending and interpolation are only
   // correct in linear space. Displays expect sRGB-encoded values. Rendering through an
   // sRGB view of the back buffer bridges the two: the GPU encodes each linear value to
   // sRGB as it writes it (and blending happens in linear space, before encoding).
   //
   // NRI creates 8-bit swap chains with a UNORM format but allows sRGB views of them. On
   // Vulkan this uses VK_KHR_swapchain_mutable_format, which all desktop drivers support.
   // D3D12 allows it natively: flip-model swap chains cannot have an sRGB format, but an
   // sRGB render target view of their UNORM back buffers is permitted.
   const nri::Format viewFormat = GetSrgbViewFormat(textureFormat);
   m_Format = FromNri(viewFormat);

   m_BackBuffers.resize(textureCount);
   for (uint32_t i = 0; i < textureCount; ++i) {
      BackBuffer& backBuffer = m_BackBuffers[i];
      backBuffer.texture = textures[i];

      const nri::TextureViewDesc viewDesc = {textures[i], nri::TextureView::COLOR_ATTACHMENT, viewFormat};
      CheckNri(nri.CreateTextureView(viewDesc, backBuffer.colorAttachment), "CreateTextureView");

      // SWAPCHAIN_SEMAPHORE as the initial value asks NRI for a binary semaphore rather
      // than a timeline fence.
      CheckNri(nri.CreateFence(m_Device.GetDevice(), {.initialValue = nri::SWAPCHAIN_SEMAPHORE}, backBuffer.acquireSemaphore), "CreateFence");
      CheckNri(nri.CreateFence(m_Device.GetDevice(), {.initialValue = nri::SWAPCHAIN_SEMAPHORE}, backBuffer.releaseSemaphore), "CreateFence");
   }

   if (m_Settings.depthFormat != Format::Unknown) {
      m_DepthBuffer.emplace(m_Device, TextureSettings{.width = width, .height = height, .format = m_Settings.depthFormat, .usage = TextureUsage::DepthAttachment});
      m_IsDepthBufferNew = true;
   }

   PKZL_CORE_LOG_DEBUG("Created swap chain: {}x{}, {} back buffers, format {} (rendered through {} views)", width, height, textureCount, nri::nriGetFormatProps(textureFormat)->name, nri::nriGetFormatProps(viewFormat)->name);
}


// Destroys the swap chain, the per-back-buffer objects and the depth buffer. The GPU must
// be idle.
void SwapChain::DestroySwapChain() {
   m_DepthBuffer.reset();

   const auto& nri = m_Device.GetNri();
   for (BackBuffer& backBuffer : m_BackBuffers) {
      nri.DestroyFence(backBuffer.acquireSemaphore);
      nri.DestroyFence(backBuffer.releaseSemaphore);
      nri.DestroyDescriptor(backBuffer.colorAttachment);
      // The textures themselves belong to the swap chain.
   }
   m_BackBuffers.clear();

   nri.DestroySwapChain(m_SwapChain);
   m_SwapChain = nullptr;
}


void SwapChain::RecreateSwapChain(const uint32_t width, const uint32_t height) {
   // Recreating is rare (it happens on resize), so simply waiting for the GPU to go idle
   // is acceptable, and it guarantees nothing still uses the old back buffers.
   CheckNri(m_Device.GetNri().QueueWaitIdle(&m_Device.GetGraphicsQueue()), "QueueWaitIdle");
   DestroySwapChain();
   CreateSwapChain(width, height);
}


void SwapChain::DestroyAll() {
   DestroySwapChain();

   const auto& nri = m_Device.GetNri();
   for (auto& frame : m_Frames) {
      nri.DestroyCommandBuffer(frame.commandBuffer);
      nri.DestroyCommandAllocator(frame.commandAllocator);
   }
   m_Frames.clear();

   nri.DestroyFence(m_FrameFence);
   m_FrameFence = nullptr;
}

}
