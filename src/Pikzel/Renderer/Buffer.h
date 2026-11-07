#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace nri {
struct Buffer;
struct Descriptor;
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

struct BufferSettings {
   uint64_t size = 0;
   BufferUsage usage = BufferUsage::None;

   // Where the buffer's memory lives, which decides how the CPU fills it:
   //  - false: device-local memory, the fastest for the GPU to read but not writable by the
   //    CPU. Contents are copied in through a staging buffer, once, at creation.
   //  - true: memory the CPU writes directly with Write(), and the GPU reads from. Used for
   //    data that changes every frame, such as per-frame constants.
   bool isHostWritable = false;
};

// A GPU buffer together with the memory backing it.
//
// NRI separates creating a buffer from allocating memory for it, as Vulkan and D3D12 do,
// which lets an engine pack many resources into a few large allocations. Buffer keeps the
// two together, which is all the engine needs until it has many small resources to manage.
//
// Device-local buffers are filled at creation by NRI's UploadData helper, which copies
// through a temporary staging buffer and waits for the GPU to finish. That is fine at load
// time but too slow for data that changes every frame; those buffers are host-writable.
//
// Host-writable buffers use NRI's DEVICE_UPLOAD memory: on GPUs with resizable BAR this is
// video memory the CPU can write over PCIe, so the GPU reads it at full speed; otherwise
// NRI falls back to system memory, which the GPU reads over PCIe. Either way the CPU writes
// straight into memory the GPU may be reading. Write() does not synchronise with the GPU,
// so the caller must not overwrite data that frames still in flight are using. The usual
// pattern, used by the examples, is one buffer per frame in flight (see SwapChain).
//
// A buffer with BufferUsage::Constant also gets a constant buffer view, so it can be put
// into a DescriptorSet. D3D12 requires constant buffer views to be a multiple of 256 bytes,
// so the GPU allocation is rounded up to that; GetSize() still reports the requested size.
class Buffer {
public:
   // Creates a buffer with uninitialised contents. A host-writable buffer is filled with
   // Write(). A device-local one is only useful if the GPU itself will write to it.
   Buffer(GraphicsDevice& device, const BufferSettings& settings);

   // Creates a device-local buffer the size of data, and uploads data into it.
   Buffer(GraphicsDevice& device, BufferUsage usage, std::span<const std::byte> data);

   ~Buffer();

   Buffer(const Buffer&) = delete;
   Buffer& operator=(const Buffer&) = delete;
   Buffer(Buffer&& other) noexcept;
   Buffer& operator=(Buffer&& other) noexcept;

   // Copies data into a host-writable buffer, starting offset bytes in.
   void Write(std::span<const std::byte> data, uint64_t offset = 0);

   uint64_t GetSize() const;
   BufferUsage GetUsage() const;
   bool IsHostWritable() const;

   // Renderer internal.
   nri::Buffer& GetNriBuffer() const;
   nri::Descriptor& GetNriConstantBufferView() const;

private:
   void Create(const BufferSettings& settings);
   void Release();

   GraphicsDevice* m_Device = nullptr;
   nri::Buffer* m_Buffer = nullptr;
   nri::Descriptor* m_ConstantBufferView = nullptr;
   std::vector<nri::Memory*> m_Memory;
   uint64_t m_Size = 0;
   BufferUsage m_Usage = BufferUsage::None;
   bool m_IsHostWritable = false;
};

}
