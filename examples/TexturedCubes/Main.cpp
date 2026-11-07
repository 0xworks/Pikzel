// TexturedCubes: 3D rendering with a camera, a depth buffer, textures and resource binding.
//
// A large cube spins in the middle of the window while four small cubes orbit it, passing
// through its corners. Every cube is drawn from the same indexed vertex buffer, with the
// same texture, and lit by one directional light. The example shows:
//
//  - view and projection matrices built with RTM, in the engine's right-handed, reversed-Z
//    conventions (see Renderer/Camera.h);
//  - a depth buffer, kept by the swap chain. The small cubes are drawn first, so without
//    depth testing the large cube would simply be painted over them; with it, they are
//    correctly hidden where they pass inside it;
//  - an sRGB texture and a sampler, reached through a descriptor set;
//  - per-frame constants (the camera and light) in a host-writable constant buffer, one
//    per frame in flight, each with its own descriptor set;
//  - per-draw constants (each cube's model matrix) in push constants;
//  - C++ structs that mirror the shader's data, with their layout checked at compile time.
//
// The texture is a checkerboard with an orange square in its top-left corner, so that
// every face shows which way up the texture is applied.
//
// It runs on Vulkan by default. Pass --d3d12 to run on D3D12 instead (only available when
// the engine is built with PKZL_ENABLE_D3D12). The result should look identical on both.

#include "Pikzel/Core/Log.h"
#include "Pikzel/Platform/Window.h"
#include "Pikzel/Renderer/Buffer.h"
#include "Pikzel/Renderer/Camera.h"
#include "Pikzel/Renderer/Color.h"
#include "Pikzel/Renderer/CommandList.h"
#include "Pikzel/Renderer/DescriptorSet.h"
#include "Pikzel/Renderer/GraphicsDevice.h"
#include "Pikzel/Renderer/Pipeline.h"
#include "Pikzel/Renderer/Sampler.h"
#include "Pikzel/Renderer/Shader.h"
#include "Pikzel/Renderer/ShaderTypes.h"
#include "Pikzel/Renderer/SwapChain.h"
#include "Pikzel/Renderer/Texture.h"

#include <rtm/matrix3x4f.h>
#include <rtm/matrix4x4f.h>
#include <rtm/quatf.h>
#include <rtm/vector4f.h>

#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <numbers>
#include <span>
#include <string_view>
#include <vector>

namespace pk = Pikzel;

namespace {

struct Vertex {
   float position[3];
   float normal[3];
   float uv[2];
};

// Mirrors DrawConstants in Cube.slang: the push constants.
struct DrawConstants {
   pk::Float4x4 model;
};

static_assert(sizeof(DrawConstants) == 64);

// Mirrors FrameConstants in Cube.slang. Following constant buffer packing rules, the float3
// and the float after it share one 16-byte row.
struct FrameConstants {
   pk::Float4x4 viewProjection;
   float directionToLight[3];
   float ambient;
};

static_assert(offsetof(FrameConstants, viewProjection) == 0);
static_assert(offsetof(FrameConstants, directionToLight) == 64);
static_assert(offsetof(FrameConstants, ambient) == 76);
static_assert(sizeof(FrameConstants) == 80);

// Positions of the descriptor sets in the pipeline layout (see the pipeline settings).
constexpr uint32_t s_FrameSetIndex = 0;
constexpr uint32_t s_MaterialSetIndex = 1;


struct Mesh {
   std::vector<Vertex> vertices;
   std::vector<uint16_t> indices;
};


// A unit cube centred on the origin. Each face has its own four vertices, because vertices
// at the same corner need different normals and texture coordinates on different faces.
Mesh BuildCube() {
   struct Face {
      float normal[3];
      // The direction that is "up" in the face's texture.
      float up[3];
   };
   constexpr Face faces[] = {
      {{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
      {{-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
      {{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
      {{0.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
      {{0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
      {{0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
   };

   Mesh mesh;
   for (const Face& face : faces) {
      const float* n = face.normal;
      const float* v = face.up;
      // "Right" in the face's texture, chosen so that right x up = normal. Going round the
      // corners as -right-up, +right-up, +right+up, -right+up is then counter-clockwise when
      // seen from outside the cube (looking against the normal), the engine's front face.
      const float u[3] = {v[1] * n[2] - v[2] * n[1], v[2] * n[0] - v[0] * n[2], v[0] * n[1] - v[1] * n[0]};

      // Corner offsets along (right, up), and their texture coordinates. Texture v runs
      // downwards from the top row of the image, so the face's top edge has v = 0.
      constexpr float corners[4][4] = {
         {-1.0f, -1.0f, 0.0f, 1.0f},
         {1.0f, -1.0f, 1.0f, 1.0f},
         {1.0f, 1.0f, 1.0f, 0.0f},
         {-1.0f, 1.0f, 0.0f, 0.0f},
      };

      const auto first = static_cast<uint16_t>(mesh.vertices.size());
      for (const auto& corner : corners) {
         Vertex vertex = {};
         for (int axis = 0; axis < 3; ++axis) {
            vertex.position[axis] = 0.5f * (n[axis] + corner[0] * u[axis] + corner[1] * v[axis]);
            vertex.normal[axis] = n[axis];
         }
         vertex.uv[0] = corner[2];
         vertex.uv[1] = corner[3];
         mesh.vertices.push_back(vertex);
      }

      // Two triangles per face, both counter-clockwise.
      constexpr uint16_t faceIndices[] = {0, 1, 2, 0, 2, 3};
      for (const uint16_t index : faceIndices) {
         mesh.indices.push_back(static_cast<uint16_t>(first + index));
      }
   }
   return mesh;
}


// RGBA8 texels for a checkerboard. The colors are sRGB hex codes, as a paint program would
// give them, and they are stored as they are: the texture has an sRGB format, so the GPU
// decodes them to linear values when the shader samples it.
std::vector<std::byte> BuildCheckerboard(const uint32_t size, const uint32_t squares) {
   constexpr uint32_t light = 0xF2E8D5;
   constexpr uint32_t dark = 0x2E6F95;
   constexpr uint32_t marker = 0xE07A2E;

   std::vector<std::byte> texels;
   texels.reserve(size * size * 4);
   const uint32_t squareSize = size / squares;
   for (uint32_t y = 0; y < size; ++y) {
      for (uint32_t x = 0; x < size; ++x) {
         const uint32_t squareX = x / squareSize;
         const uint32_t squareY = y / squareSize;
         uint32_t color = (squareX + squareY) % 2 == 0 ? light : dark;
         if (squareX == 0 && squareY == 0) {
            color = marker;
         }
         texels.push_back(static_cast<std::byte>((color >> 16) & 0xFF));
         texels.push_back(static_cast<std::byte>((color >> 8) & 0xFF));
         texels.push_back(static_cast<std::byte>(color & 0xFF));
         texels.push_back(std::byte{0xFF});
      }
   }
   return texels;
}


// A model matrix from a rotation, a position and a uniform scale.
rtm::matrix4x4f MakeModelMatrix(const rtm::quatf& rotation, const rtm::vector4f& position, const float scale) {
   return rtm::matrix_cast(rtm::matrix_from_vqv(rotation, position, rtm::vector_set(scale)));
}


pk::GraphicsApi ParseGraphicsApi(const std::span<char*> arguments) {
   for (const std::string_view argument : arguments) {
      if (argument == "--d3d12") {
         return pk::GraphicsApi::D3D12;
      }
   }
   return pk::GraphicsApi::Vulkan;
}


void Run(const pk::GraphicsApi api) {
   pk::Window window({.title = "Textured Cubes"});
   pk::GraphicsDevice device({.api = api});
   pk::SwapChain swapChain(device, window, {.depthFormat = pk::Format::D32Float});

   const Mesh cube = BuildCube();
   const pk::Buffer vertexBuffer(device, pk::BufferUsage::Vertex, std::as_bytes(std::span(cube.vertices)));
   const pk::Buffer indexBuffer(device, pk::BufferUsage::Index, std::as_bytes(std::span(cube.indices)));

   constexpr uint32_t textureSize = 256;
   const pk::Texture texture(device, {.width = textureSize, .height = textureSize, .format = pk::Format::RGBA8Srgb}, BuildCheckerboard(textureSize, 8));
   const pk::Sampler sampler(device, {.filter = pk::Filter::Linear, .addressMode = pk::AddressMode::Repeat, .maxAnisotropy = 8});

   const pk::Shader vertexShader(device, "Cube", "VertexMain");
   const pk::Shader fragmentShader(device, "Cube", "FragmentMain");

   // The layout matches the register spaces declared in Cube.slang.
   const pk::Pipeline pipeline(
      device,
      {
         .vertexShader = &vertexShader,
         .fragmentShader = &fragmentShader,
         .vertexAttributes = {
            {.location = 0, .semanticName = "POSITION", .format = pk::Format::RGB32Float, .offset = offsetof(Vertex, position)},
            {.location = 1, .semanticName = "NORMAL", .format = pk::Format::RGB32Float, .offset = offsetof(Vertex, normal)},
            {.location = 2, .semanticName = "TEXCOORD", .semanticIndex = 0, .format = pk::Format::RG32Float, .offset = offsetof(Vertex, uv)},
         },
         .vertexStride = sizeof(Vertex),
         .descriptorSets = {
            // s_FrameSetIndex: per-frame constants, b0 in space1.
            {.registerSpace = 1, .ranges = {{.type = pk::DescriptorType::ConstantBuffer, .baseRegister = 0}}},
            // s_MaterialSetIndex: texture t0 and sampler s0, in space2.
            {.registerSpace = 2, .ranges = {{.type = pk::DescriptorType::Texture, .baseRegister = 0}, {.type = pk::DescriptorType::Sampler, .baseRegister = 0}}},
         },
         .pushConstantsSize = sizeof(DrawConstants),
         .cullMode = pk::CullMode::Back,
         .colorFormat = swapChain.GetFormat(),
         .depthFormat = swapChain.GetDepthFormat(),
      }
   );

   // The material's resources never change, so one descriptor set serves every frame.
   pk::DescriptorSet materialSet(device, pipeline, s_MaterialSetIndex);
   materialSet.SetTexture(0, texture);
   materialSet.SetSampler(1, sampler);

   // The frame constants are rewritten every frame. While the CPU writes frame N's, the GPU
   // may still be reading frame N-1's, so each frame in flight gets its own buffer, and a
   // descriptor set pointing at it.
   std::vector<pk::Buffer> frameConstantBuffers;
   std::vector<pk::DescriptorSet> frameSets;
   frameConstantBuffers.reserve(swapChain.GetFramesInFlight());
   frameSets.reserve(swapChain.GetFramesInFlight());
   for (uint32_t i = 0; i < swapChain.GetFramesInFlight(); ++i) {
      const auto& buffer = frameConstantBuffers.emplace_back(device, pk::BufferSettings{.size = sizeof(FrameConstants), .usage = pk::BufferUsage::Constant, .isHostWritable = true});
      frameSets.emplace_back(device, pipeline, s_FrameSetIndex).SetConstantBuffer(0, buffer);
   }

   const rtm::vector4f eye = rtm::vector_set(0.0f, 1.4f, 3.2f);
   const rtm::vector4f target = rtm::vector_set(0.0f, 0.0f, 0.0f);
   const rtm::vector4f up = rtm::vector_set(0.0f, 1.0f, 0.0f);
   const rtm::matrix4x4f view = pk::CreateLookAtMatrix(eye, target, up);

   FrameConstants frameConstants = {};
   rtm::vector_store3(rtm::vector_normalize3(rtm::vector_set(0.4f, 0.8f, 0.45f)), frameConstants.directionToLight);
   frameConstants.ambient = 0.2f;

   const auto clearColor = pk::Color::FromSrgbHex(0x1A1A1F);
   const auto startTime = std::chrono::steady_clock::now();
   constexpr float pi = std::numbers::pi_v<float>;

   PKZL_LOG_INFO("Expect: a large spinning cube with four small cubes orbiting through its corners, each correctly hidden where it is inside the large cube.");

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

      const float time = std::chrono::duration<float>(std::chrono::steady_clock::now() - startTime).count();

      // The projection depends on the window's shape, which can change at any time.
      const float aspectRatio = static_cast<float>(frame->backBuffer.GetWidth()) / static_cast<float>(frame->backBuffer.GetHeight());
      const rtm::matrix4x4f projection = pk::CreatePerspectiveMatrix(pi / 3.0f, aspectRatio, 0.1f);

      // Row vectors: world -> view first, then view -> clip.
      frameConstants.viewProjection = pk::ToFloat4x4(rtm::matrix_mul(view, projection));
      frameConstantBuffers[frame->inFlightIndex].Write(std::as_bytes(std::span(&frameConstants, 1)));

      pk::CommandList& commandList = frame->commandList;
      commandList.BeginRendering({
         .colorTarget = &frame->backBuffer,
         .clearColor = clearColor,
         .depthTarget = &*frame->depthBuffer,
         .clearDepth = 0.0f, // reversed-Z: 0 is infinitely far away
      });
      commandList.BindPipeline(pipeline);
      commandList.BindVertexBuffer(vertexBuffer);
      commandList.BindIndexBuffer(indexBuffer, pk::IndexType::UInt16);
      commandList.BindDescriptorSet(s_FrameSetIndex, frameSets[frame->inFlightIndex]);
      commandList.BindDescriptorSet(s_MaterialSetIndex, materialSet);

      const auto indexCount = static_cast<uint32_t>(cube.indices.size());

      // The small cubes, drawn first so that the depth test (not draw order) decides what
      // is visible where they overlap the large one.
      const rtm::vector4f tumbleAxis = rtm::vector_normalize3(rtm::vector_set(1.0f, 1.0f, 0.0f));
      for (int i = 0; i < 4; ++i) {
         const float phase = static_cast<float>(i) * pi / 2.0f;
         const float angle = time * 0.7f + phase;
         const rtm::vector4f position = rtm::vector_set(0.95f * std::cos(angle), 0.3f * std::sin(time * 1.3f + phase), 0.95f * std::sin(angle));
         const rtm::quatf rotation = rtm::quat_from_axis_angle(tumbleAxis, time * 2.0f + phase);
         commandList.PushConstants(DrawConstants{pk::ToFloat4x4(MakeModelMatrix(rotation, position, 0.4f))});
         commandList.DrawIndexed(indexCount);
      }

      const rtm::vector4f spinAxis = rtm::vector_normalize3(rtm::vector_set(0.3f, 1.0f, 0.2f));
      const rtm::quatf rotation = rtm::quat_from_axis_angle(spinAxis, time * 0.6f);
      commandList.PushConstants(DrawConstants{pk::ToFloat4x4(MakeModelMatrix(rotation, rtm::vector_zero(), 1.0f))});
      commandList.DrawIndexed(indexCount);

      commandList.EndRendering();
      swapChain.EndFrame();
   }

   // The GPU may still be drawing the last frames. Wait for it before the resources above
   // are destroyed.
   device.WaitIdle();
}

}


int main(const int argc, char* argv[]) {
   pk::Log::Init();

   int exitCode = 0;
   try {
      Run(ParseGraphicsApi(std::span(argv, static_cast<std::size_t>(argc))));
   } catch (const std::exception& e) {
      PKZL_LOG_FATAL("{}", e.what());
      exitCode = 1;
   }

   pk::Log::Shutdown();
   return exitCode;
}
