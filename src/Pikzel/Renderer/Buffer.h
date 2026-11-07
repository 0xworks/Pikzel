#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace nri {
struct Buffer;
struct Memory;
}

namespace Pikzel {

class GraphicsDevice;

// How a buffer will be used. Flags can be combined, e.g. Vertex | Index for a buffer
// holding both vertices and indices.
enum class BufferUsage : uint32_t {
   None = 0,
   Vertex = 1 << 0,
   Index = 1 << 1,
   Constant = 1 << 2,
   ShaderResource = 1 << 3,
};

constexpr BufferUsage operator|(const BufferUsage lhs, const BufferUsage rhs) {
   return static_cast<BufferUsage>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

constexpr bool HasFlag(const BufferUsage usage, const BufferUsage flag) {
   return (static_cast<uint32_t>(usage) & static_cast<uint32_t>(flag)) != 0;
}

// A GPU buffer together with the memory backing it.
//
// NRI separates creating a buffer from allocating memory for it, as Vulkan and D3D12 do,
// which lets an engine pack many resources into a few large allocations. Buffer keeps the
// two together, which is all the engine needs until it has many small resources to manage.
//
// The buffer lives in device-local memory: the fastest memory for the GPU to read, but
// not directly writable by the CPU. Its contents are therefore copied in through a
// temporary staging buffer by NRI's UploadData helper. That upload waits for the GPU to
// finish, which is fine at load time but too slow for data that changes every frame.
class Buffer {
public:
   // Creates a buffer the size of data, and uploads data into it.
   Buffer(GraphicsDevice& device, BufferUsage usage, std::span<const std::byte> data);
   ~Buffer();

   Buffer(const Buffer&) = delete;
   Buffer& operator=(const Buffer&) = delete;
   Buffer(Buffer&& other) noexcept;
   Buffer& operator=(Buffer&& other) noexcept;

   uint64_t GetSize() const;

   // Renderer internal.
   nri::Buffer& GetNriBuffer() const;

private:
   void Release();

   GraphicsDevice* m_Device = nullptr;
   nri::Buffer* m_Buffer = nullptr;
   std::vector<nri::Memory*> m_Memory;
   uint64_t m_Size = 0;
};

}
