// HelloTriangle: the smallest program that draws something with the engine's renderer.
//
// It opens a window, creates a GPU device and a swap chain for the window, uploads one
// triangle's vertices to a GPU buffer, builds a graphics pipeline from Slang shaders
// (compiled to SPIR-V at build time), and then draws the triangle every frame until the
// window is closed. The window can be resized and minimised.

#include "Pikzel/Core/Log.h"
#include "Pikzel/Platform/Window.h"
#include "Pikzel/Renderer/Buffer.h"
#include "Pikzel/Renderer/Color.h"
#include "Pikzel/Renderer/CommandList.h"
#include "Pikzel/Renderer/GraphicsDevice.h"
#include "Pikzel/Renderer/Pipeline.h"
#include "Pikzel/Renderer/Shader.h"
#include "Pikzel/Renderer/SwapChain.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <span>

namespace pk = Pikzel;

namespace {

// The CPU-side vertex layout. The pipeline's vertex attributes (below) tell the GPU how
// to read this struct, so the two must agree on offsets, formats and stride.
struct Vertex {
   float position[2];
   pk::Color color; // four linear floats, read by the shader as a float4
};


void Run() {
   pk::Window window({.title = "Hello Triangle"});
   pk::GraphicsDevice device;
   pk::SwapChain swapChain(device, window);

   // Colors are written as sRGB hex codes, as a color picker would give them, and
   // converted to linear values for the GPU. The rasteriser interpolates them across the
   // triangle in linear space, which is what makes the blends between corners look right.
   const std::array<Vertex, 3> vertices = {{
      {{0.0f, 0.5f}, pk::Color::FromSrgbHex(0xFF0000)},
      {{0.5f, -0.5f}, pk::Color::FromSrgbHex(0x00FF00)},
      {{-0.5f, -0.5f}, pk::Color::FromSrgbHex(0x0000FF)},
   }};
   const auto clearColor = pk::Color::FromSrgbHex(0x1A1A1F);

   const pk::Buffer vertexBuffer(device, pk::BufferUsage::Vertex, std::as_bytes(std::span(vertices)));

   // Triangle.slang is compiled to Triangle.spv at build time; it holds both entry points.
   const pk::Shader shader("Triangle");

   // Attribute locations match [[vk::location(n)]] in Triangle.slang.
   const pk::Pipeline pipeline(
      device,
      {
         .shader = &shader,
         .vertexEntryPoint = "VertexMain",
         .fragmentEntryPoint = "FragmentMain",
         .vertexAttributes = {
            {.location = 0, .format = pk::Format::RG32Float, .offset = offsetof(Vertex, position)},
            {.location = 1, .format = pk::Format::RGBA32Float, .offset = offsetof(Vertex, color)},
         },
         .vertexStride = sizeof(Vertex),
         .cullMode = pk::CullMode::None,
         .colorFormat = swapChain.GetFormat(),
      }
   );

   while (!window.ShouldClose()) {
      pk::Window::PollEvents();

      if (window.IsMinimized()) {
         pk::Window::WaitEvents();
         continue;
      }

      auto frame = swapChain.BeginFrame();
      if (!frame) {
         continue;
      }

      pk::CommandList& commandList = frame->commandList;
      commandList.BeginRendering(frame->backBuffer, clearColor);
      commandList.BindPipeline(pipeline);
      commandList.BindVertexBuffer(vertexBuffer);
      commandList.Draw(static_cast<uint32_t>(vertices.size()));
      commandList.EndRendering();

      swapChain.EndFrame();
   }

   // The GPU may still be drawing the last frames. Wait for it before the pipeline and
   // vertex buffer (destroyed first, as the most recently created locals) go away.
   device.WaitIdle();
}

}


int main() {
   pk::Log::Init();

   int exitCode = 0;
   try {
      Run();
   } catch (const std::exception& e) {
      PKZL_LOG_FATAL("{}", e.what());
      exitCode = 1;
   }

   pk::Log::Shutdown();
   return exitCode;
}
