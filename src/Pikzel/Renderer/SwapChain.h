#pragma once

#include "Pikzel/Renderer/CommandList.h"
#include "Pikzel/Renderer/Format.h"
#include "Pikzel/Renderer/Texture.h"

#include <cstdint>
#include <optional>
#include <vector>

namespace nri {
struct CommandAllocator;
struct CommandBuffer;
struct Descriptor;
struct Fence;
struct SwapChain;
struct Texture;
}

namespace Pikzel {

class GraphicsDevice;
class Window;

struct SwapChainSettings {
   // Wait for the display's vertical blank before presenting. Prevents tearing and caps
   // the frame rate at the display's refresh rate.
   bool isVSyncEnabled = true;

   // How many frames the CPU may record ahead of the GPU. See SwapChain for details.
   uint32_t framesInFlight = 2;

   // If set, the swap chain also keeps a depth buffer the size of the window, recreated
   // with the back buffers when the window is resized. Unknown means no depth buffer.
   Format depthFormat = Format::Unknown;
};

// Everything needed to record one frame of rendering into the window. Valid from
// SwapChain::BeginFrame() until the matching SwapChain::EndFrame().
//
// The command list is already recording, and the back buffer (and depth buffer, if any)
// are ready to be rendered into: start with commandList.BeginRendering(...).
struct Frame {
   CommandList commandList;
   RenderTarget backBuffer;
   std::optional<RenderTarget> depthBuffer;

   // Which of the frames-in-flight slots this frame uses, from 0 to framesInFlight - 1.
   // Anything the application rewrites every frame, such as a host-writable buffer of
   // per-frame constants, needs one copy per slot, indexed by this. The swap chain
   // guarantees the GPU has finished the previous frame that used the same slot.
   uint32_t inFlightIndex = 0;
};

// Presents rendered frames to a window, and paces the CPU against the GPU while doing so.
//
// A swap chain is a small set of textures (back buffers) owned jointly with the operating
// system's compositor. Each frame the application acquires one, renders into it, and
// presents it, handing it back to be displayed.
//
// Frames in flight
// ----------------
// The CPU and GPU run in parallel: while the GPU executes frame N, the CPU records frame
// N+1. To allow this, everything the CPU writes during a frame (here, the command
// allocator and command buffer) exists once per frame in flight, and the CPU only reuses
// a set once the GPU has finished with it. A timeline fence, signalled with the frame
// number at the end of each frame's GPU work, tells the CPU when that has happened.
//
// More frames in flight hide more variation in frame times, but add latency between input
// and display. Two is the usual choice.
//
// Frame pacing
// ------------
// The fence only stops the CPU getting too far ahead of the GPU. It does not tie the CPU to
// the display: the presentation engine can release back buffers in bursts, letting the CPU
// record several frames in quick succession and then blocking it for several refreshes.
// Each frame is still displayed one refresh apart, but anything the CPU sampled at record
// time (animation time, input) is then unevenly spaced, and motion visibly jitters.
//
// BeginFrame() therefore first waits until frame (N - framesInFlight) has actually been
// presented, using a waitable swap chain (VK_KHR_present_wait on Vulkan, the frame latency
// waitable object on D3D12). The CPU then starts each frame in step with the display, so
// the time between frames is steady, and latency stays at framesInFlight frames.
//
// GPU-side synchronisation
// ------------------------
// Two binary semaphores order the GPU work for each frame:
//  - the acquire semaphore is signalled when the presentation engine has finished reading
//    the back buffer, so rendering into it must wait for it;
//  - the release semaphore is signalled when rendering is finished, so presentation must
//    wait for it.
// Release semaphores belong to a back buffer, since presentation of that back buffer
// consumes them. Acquire semaphores are used before the back buffer index is known, so
// they are cycled through by frame number instead. This is the scheme recommended by NRI
// (see Extensions/NRISwapChain.h), and it is valid while the number of back buffers is
// greater than the number of frames in flight.
//
// Resizing
// --------
// The back buffers must match the window size. BeginFrame() recreates the swap chain when
// the window has been resized, or when the graphics API reports the swap chain as out of
// date. While the window is minimised there is nothing to render into, and BeginFrame()
// returns no frame.
//
// Depth buffer
// ------------
// Unlike the back buffers, there is a single depth buffer, shared by all frames in flight.
// That is safe because frames execute one after another on the GPU's single graphics
// queue, and each frame clears the depth buffer before use. A barrier at the start of each
// frame makes the GPU finish the previous frame's depth writes before the next frame's
// begin. The depth buffer lives here, rather than with the application, so that it is
// resized together with the back buffers.
//
// The swap chain keeps a reference to the window, which must outlive it.
class SwapChain {
public:
   SwapChain(GraphicsDevice& device, const Window& window, const SwapChainSettings& settings = {});
   ~SwapChain();

   SwapChain(const SwapChain&) = delete;
   SwapChain& operator=(const SwapChain&) = delete;
   SwapChain(SwapChain&&) = delete;
   SwapChain& operator=(SwapChain&&) = delete;

   // Waits until the resources for the next frame are free, acquires a back buffer, and
   // begins recording. Returns no frame if there is nothing to render into this time (the
   // window is minimised, or the swap chain had to be recreated); the caller should simply
   // skip rendering and try again next frame.
   std::optional<Frame> BeginFrame();

   // Finishes recording, submits the frame's commands to the GPU, and presents.
   void EndFrame();

   // The format pipelines render to the back buffers with, needed when creating them.
   // Always an sRGB format: shaders output linear colors, and the GPU encodes them for
   // display as it writes them.
   Format GetFormat() const;

   // SwapChainSettings::depthFormat: Unknown if there is no depth buffer.
   Format GetDepthFormat() const;

   uint32_t GetFramesInFlight() const;

private:
   struct FrameResources {
      nri::CommandAllocator* commandAllocator = nullptr;
      nri::CommandBuffer* commandBuffer = nullptr;
   };

   struct BackBuffer {
      nri::Texture* texture = nullptr;
      nri::Descriptor* colorAttachment = nullptr;
      nri::Fence* acquireSemaphore = nullptr;
      nri::Fence* releaseSemaphore = nullptr;
   };

   void CreateSwapChain(uint32_t width, uint32_t height);
   void DestroySwapChain();
   void RecreateSwapChain(uint32_t width, uint32_t height);
   void DestroyAll();

   GraphicsDevice& m_Device;
   const Window& m_Window;
   SwapChainSettings m_Settings;

   nri::SwapChain* m_SwapChain = nullptr;
   std::vector<BackBuffer> m_BackBuffers;
   Format m_Format = Format::Unknown;

   std::optional<Texture> m_DepthBuffer;
   // A newly created depth buffer has undefined layout, and needs a different barrier
   // in its first frame. See BeginFrame().
   bool m_IsDepthBufferNew = false;
   uint32_t m_Width = 0;
   uint32_t m_Height = 0;
   bool m_IsOutOfDate = false;

   std::vector<FrameResources> m_Frames;
   nri::Fence* m_FrameFence = nullptr;

   // Number of frames submitted so far. Frame N signals m_FrameFence with the value N + 1,
   // and is presented with present ID N + 1.
   uint64_t m_FrameIndex = 0;
   // The first frame presented by the current swap chain. Frames before it were presented
   // by a swap chain that has since been recreated, so there is no waiting for them.
   uint64_t m_FirstFrameIndex = 0;

   // State of the frame between BeginFrame() and EndFrame().
   bool m_IsFrameInProgress = false;
   uint32_t m_BackBufferIndex = 0;
   nri::Fence* m_AcquireSemaphore = nullptr;
};

}
