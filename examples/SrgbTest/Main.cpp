// SrgbTest: a visual check that the renderer handles color spaces correctly.
//
// The pattern is based on "What every coder should know about gamma" by John Novak
// (https://blog.johnnovak.net/2016/09/21/what-every-coder-should-know-about-gamma/).
// It is easy to judge by eye, and gets visibly wrong if colors are blended or interpolated
// in sRGB space instead of linear space:
//
//  1. A row of 21 grey squares, from sRGB 0.0 (black) to sRGB 1.0 (white) in equal sRGB
//     steps. sRGB is designed to be perceptually uniform, so the row should look like an
//     even ramp, with mid-grey (sRGB 0.5, i.e. 128) in the middle. Treating the values as
//     linear instead would make most of the row look washed out.
//
//  2. An opaque green vertical bar, crossed by two red horizontal bars: one opaque, and one
//     50% transparent. When blending is done in linear space, the transparent bar has the
//     same perceived brightness along its whole length. Blending in sRGB space makes it
//     noticeably darker where it crosses the green bar.
//
// Everything is drawn in a single draw call from one vertex buffer, with alpha blending.
// Shapes are laid out in a 32 x 18 unit space (matching the window's 16:9 shape) and
// stretch with the window if it is resized.

#include "Pikzel/Core/Log.h"
#include "Pikzel/Platform/Window.h"
#include "Pikzel/Renderer/Buffer.h"
#include "Pikzel/Renderer/Color.h"
#include "Pikzel/Renderer/CommandList.h"
#include "Pikzel/Renderer/GraphicsDevice.h"
#include "Pikzel/Renderer/Pipeline.h"
#include "Pikzel/Renderer/Shader.h"
#include "Pikzel/Renderer/SwapChain.h"

#include <cstddef>
#include <cstdint>
#include <exception>
#include <span>
#include <vector>

namespace pk = Pikzel;

namespace {

struct Vertex {
   float position[2];
   pk::Color color;
};

// Half the width and height of the layout space. Layout coordinates are divided by these
// to get clip-space coordinates (-1 to +1).
constexpr float s_HalfWidth = 16.0f;
constexpr float s_HalfHeight = 9.0f;


// Appends an axis-aligned rectangle, as two triangles, given its center and size in
// layout units. Vertices are in clockwise order, the engine's front-facing winding.
void AddRect(std::vector<Vertex>& vertices, const float centerX, const float centerY, const float width, const float height, const pk::Color& color) {
   const float left = (centerX - width / 2.0f) / s_HalfWidth;
   const float right = (centerX + width / 2.0f) / s_HalfWidth;
   const float bottom = (centerY - height / 2.0f) / s_HalfHeight;
   const float top = (centerY + height / 2.0f) / s_HalfHeight;

   vertices.push_back({{left, top}, color});
   vertices.push_back({{right, top}, color});
   vertices.push_back({{right, bottom}, color});

   vertices.push_back({{left, top}, color});
   vertices.push_back({{right, bottom}, color});
   vertices.push_back({{left, bottom}, color});
}


std::vector<Vertex> BuildTestPattern() {
   std::vector<Vertex> vertices;

   // Blending test. Later shapes are drawn over earlier ones, so the green bar comes first.
   AddRect(vertices, 0.0f, 0.0f, 2.0f, 8.0f, pk::Color::FromSrgb(0.0f, 1.0f, 0.0f));
   AddRect(vertices, 0.0f, 0.0f, 14.0f, 0.5f, pk::Color::FromSrgb(1.0f, 0.0f, 0.0f));
   AddRect(vertices, 0.0f, -1.0f, 14.0f, 0.5f, pk::Color::FromSrgb(1.0f, 0.0f, 0.0f, 0.5f));

   // Grey ramp. The value is computed as i / 20 rather than by adding 0.05 repeatedly:
   // twenty float additions of 0.05f reach 1.0000001, which would trip Color's range check.
   constexpr int stepCount = 20;
   for (int i = 0; i <= stepCount; ++i) {
      const float value = static_cast<float>(i) / stepCount;
      AddRect(vertices, -14.0f + 1.4f * static_cast<float>(i), -6.0f, 0.7f, 0.7f, pk::Color::FromSrgb(value, value, value));
   }

   return vertices;
}


void Run() {
   pk::Window window({.title = "sRGB Test"});
   pk::GraphicsDevice device;
   pk::SwapChain swapChain(device, window);

   const std::vector<Vertex> vertices = BuildTestPattern();
   const pk::Buffer vertexBuffer(device, pk::BufferUsage::Vertex, std::as_bytes(std::span(vertices)));

   const pk::Shader shader("SrgbTest");
   const pk::Pipeline pipeline(
      device,
      {
         .shader = &shader,
         .vertexAttributes = {
            {.location = 0, .format = pk::Format::RG32Float, .offset = offsetof(Vertex, position)},
            {.location = 1, .format = pk::Format::RGBA32Float, .offset = offsetof(Vertex, color)},
         },
         .vertexStride = sizeof(Vertex),
         .cullMode = pk::CullMode::None,
         .blendMode = pk::BlendMode::Alpha,
         .colorFormat = swapChain.GetFormat(),
      }
   );

   PKZL_LOG_INFO("Expect: an evenly spaced grey ramp with mid-grey in the middle, and a translucent red bar of even brightness, including where it crosses the green bar.");

   const auto clearColor = pk::Color::FromSrgb(1.0f, 1.0f, 1.0f);

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
