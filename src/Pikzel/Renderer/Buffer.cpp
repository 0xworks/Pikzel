#include "Pikzel/Renderer/Buffer.h"
#include "Pikzel/Renderer/GraphicsDevice.h"
#include "Pikzel/Renderer/Nri.h"

#include <cassert>
#include <cstring>
#include <stdexcept>
#include <utility>

namespace Pikzel {

namespace {

nri::BufferUsageBits ToNri(const BufferUsage usage) {
   auto bits = nri::BufferUsageBits::NONE;
   if (HasFlag(usage, BufferUsage::Vertex)) {
      bits |= nri::BufferUsageBits::VERTEX;
   }
   if (HasFlag(usage, BufferUsage::Index)) {
      bits |= nri::BufferUsageBits::INDEX;
   }
   if (HasFlag(usage, BufferUsage::Constant)) {
      bits |= nri::BufferUsageBits::CONSTANT;
   }
   if (HasFlag(usage, BufferUsage::ShaderResource)) {
      bits |= nri::BufferUsageBits::SHADER_RESOURCE;
   }
   return bits;
}


// After the upload, the buffer is transitioned to the access its usage implies, so that
// it is ready to use without a further barrier.
nri::AccessBits GetAccessForUsage(const BufferUsage usage) {
   auto access = nri::AccessBits::NONE;
   if (HasFlag(usage, BufferUsage::Vertex)) {
      access |= nri::AccessBits::VERTEX_BUFFER;
   }
   if (HasFlag(usage, BufferUsage::Index)) {
      access |= nri::AccessBits::INDEX_BUFFER;
   }
   if (HasFlag(usage, BufferUsage::Constant)) {
      access |= nri::AccessBits::CONSTANT_BUFFER;
   }
   if (HasFlag(usage, BufferUsage::ShaderResource)) {
      access |= nri::AccessBits::SHADER_RESOURCE;
   }
   return access;
}


uint64_t AlignUp(const uint64_t value, const uint64_t alignment) {
   return (value + alignment - 1) / alignment * alignment;
}

}


Buffer::Buffer(GraphicsDevice& device, const BufferSettings& settings)
: m_Device(&device) {
   Create(settings);
}


Buffer::Buffer(GraphicsDevice& device, const BufferUsage usage, const std::span<const std::byte> data)
: m_Device(&device) {
   Create({.size = data.size(), .usage = usage, .isHostWritable = false});

   try {
      nri::BufferUploadDesc uploadDesc = {};
      uploadDesc.data = data.data();
      uploadDesc.buffer = m_Buffer;
      uploadDesc.after = {GetAccessForUsage(usage)};
      CheckNri(device.GetNri().UploadData(device.GetGraphicsQueue(), nullptr, 0, &uploadDesc, 1), "UploadData");
   } catch (...) {
      Release();
      throw;
   }
}


Buffer::~Buffer() {
   Release();
}


Buffer::Buffer(Buffer&& other) noexcept
: m_Device(std::exchange(other.m_Device, nullptr)), m_Buffer(std::exchange(other.m_Buffer, nullptr)), m_ConstantBufferView(std::exchange(other.m_ConstantBufferView, nullptr)), m_Memory(std::exchange(other.m_Memory, {})), m_Size(std::exchange(other.m_Size, 0)), m_Usage(std::exchange(other.m_Usage, BufferUsage::None)), m_IsHostWritable(std::exchange(other.m_IsHostWritable, false)) {}


Buffer& Buffer::operator=(Buffer&& other) noexcept {
   if (this != &other) {
      Release();
      m_Device = std::exchange(other.m_Device, nullptr);
      m_Buffer = std::exchange(other.m_Buffer, nullptr);
      m_ConstantBufferView = std::exchange(other.m_ConstantBufferView, nullptr);
      m_Memory = std::exchange(other.m_Memory, {});
      m_Size = std::exchange(other.m_Size, 0);
      m_Usage = std::exchange(other.m_Usage, BufferUsage::None);
      m_IsHostWritable = std::exchange(other.m_IsHostWritable, false);
   }
   return *this;
}


void Buffer::Write(const std::span<const std::byte> data, const uint64_t offset) {
   assert(m_IsHostWritable && "Write() needs a buffer created with isHostWritable");
   assert(offset + data.size() <= m_Size && "Write() past the end of the buffer");

   // NRI keeps host-visible memory persistently mapped, so Map is cheap. Unmap is still
   // called because on Vulkan it flushes CPU caches if the memory is not host-coherent.
   const auto& nri = m_Device->GetNri();
   void* destination = nri.MapBuffer(*m_Buffer, offset, data.size());
   if (!destination) {
      throw std::runtime_error("MapBuffer failed");
   }
   std::memcpy(destination, data.data(), data.size());
   nri.UnmapBuffer(*m_Buffer);
}


uint64_t Buffer::GetSize() const {
   return m_Size;
}


BufferUsage Buffer::GetUsage() const {
   return m_Usage;
}


bool Buffer::IsHostWritable() const {
   return m_IsHostWritable;
}


nri::Buffer& Buffer::GetNriBuffer() const {
   return *m_Buffer;
}


nri::Descriptor& Buffer::GetNriConstantBufferView() const {
   assert(m_ConstantBufferView && "Only buffers created with BufferUsage::Constant have a constant buffer view");
   return *m_ConstantBufferView;
}


void Buffer::Create(const BufferSettings& settings) {
   assert(settings.size > 0 && "A buffer cannot be empty");

   m_Size = settings.size;
   m_Usage = settings.usage;
   m_IsHostWritable = settings.isHostWritable;

   const auto& nri = m_Device->GetNri();
   const auto& deviceDesc = nri.GetDeviceDesc(m_Device->GetDevice());
   const bool isConstantBuffer = HasFlag(settings.usage, BufferUsage::Constant);

   nri::BufferDesc desc = {};
   desc.size = isConstantBuffer ? AlignUp(settings.size, deviceDesc.memoryAlignment.constantBufferOffset) : settings.size;
   desc.usage = ToNri(settings.usage);
   CheckNri(nri.CreateBuffer(m_Device->GetDevice(), desc, m_Buffer), "CreateBuffer");

   // The constructor can still throw from here on, and the destructor does not run for an
   // object whose constructor threw, so release what has been created so far on failure.
   try {
      nri::ResourceGroupDesc resourceGroupDesc = {};
      resourceGroupDesc.memoryLocation = settings.isHostWritable ? nri::MemoryLocation::DEVICE_UPLOAD : nri::MemoryLocation::DEVICE;
      resourceGroupDesc.buffers = &m_Buffer;
      resourceGroupDesc.bufferNum = 1;

      m_Memory.resize(nri.CalculateAllocationNumber(m_Device->GetDevice(), resourceGroupDesc), nullptr);
      CheckNri(nri.AllocateAndBindMemory(m_Device->GetDevice(), resourceGroupDesc, m_Memory.data()), "AllocateAndBindMemory");

      if (isConstantBuffer) {
         const nri::BufferViewDesc viewDesc = {
            .buffer = m_Buffer,
            .type = nri::BufferView::CONSTANT_BUFFER,
            .offset = 0,
            .size = desc.size,
         };
         CheckNri(nri.CreateBufferView(viewDesc, m_ConstantBufferView), "CreateBufferView");
      }
   } catch (...) {
      Release();
      throw;
   }
}


// The caller is responsible for making sure the GPU is no longer using the buffer.
void Buffer::Release() {
   if (!m_Device) {
      return;
   }
   const auto& nri = m_Device->GetNri();
   nri.DestroyDescriptor(m_ConstantBufferView);
   nri.DestroyBuffer(m_Buffer);
   for (nri::Memory* memory : m_Memory) {
      nri.FreeMemory(memory);
   }
   m_ConstantBufferView = nullptr;
   m_Buffer = nullptr;
   m_Memory.clear();
   m_Device = nullptr;
}

}
