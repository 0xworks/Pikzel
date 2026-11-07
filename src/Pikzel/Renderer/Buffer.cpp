#include "Pikzel/Renderer/Buffer.h"
#include "Pikzel/Renderer/GraphicsDevice.h"
#include "Pikzel/Renderer/Nri.h"

#include <cassert>
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

}


Buffer::Buffer(GraphicsDevice& device, const BufferUsage usage, const std::span<const std::byte> data)
: m_Device(&device), m_Size(data.size()) {
   assert(!data.empty() && "A buffer cannot be empty");

   nri::BufferDesc desc = {};
   desc.size = data.size();
   desc.usage = ToNri(usage);

   const auto& nri = device.GetNri();
   CheckNri(nri.CreateBuffer(device.GetDevice(), desc, m_Buffer), "CreateBuffer");

   // The constructor can still throw from here on, and the destructor does not run for an
   // object whose constructor threw, so release what has been created so far on failure.
   try {
      nri::ResourceGroupDesc resourceGroupDesc = {};
      resourceGroupDesc.memoryLocation = nri::MemoryLocation::DEVICE;
      resourceGroupDesc.buffers = &m_Buffer;
      resourceGroupDesc.bufferNum = 1;

      m_Memory.resize(nri.CalculateAllocationNumber(device.GetDevice(), resourceGroupDesc), nullptr);
      CheckNri(nri.AllocateAndBindMemory(device.GetDevice(), resourceGroupDesc, m_Memory.data()), "AllocateAndBindMemory");

      nri::BufferUploadDesc uploadDesc = {};
      uploadDesc.data = data.data();
      uploadDesc.buffer = m_Buffer;
      uploadDesc.after = {GetAccessForUsage(usage)};
      CheckNri(nri.UploadData(device.GetGraphicsQueue(), nullptr, 0, &uploadDesc, 1), "UploadData");
   } catch (...) {
      Release();
      throw;
   }
}


Buffer::~Buffer() {
   Release();
}


Buffer::Buffer(Buffer&& other) noexcept
: m_Device(std::exchange(other.m_Device, nullptr)), m_Buffer(std::exchange(other.m_Buffer, nullptr)), m_Memory(std::move(other.m_Memory)), m_Size(std::exchange(other.m_Size, 0)) {
   other.m_Memory.clear();
}


Buffer& Buffer::operator=(Buffer&& other) noexcept {
   if (this != &other) {
      Release();
      m_Device = std::exchange(other.m_Device, nullptr);
      m_Buffer = std::exchange(other.m_Buffer, nullptr);
      m_Memory = std::move(other.m_Memory);
      m_Size = std::exchange(other.m_Size, 0);
      other.m_Memory.clear();
   }
   return *this;
}


nri::Buffer& Buffer::GetNriBuffer() const {
   return *m_Buffer;
}


uint64_t Buffer::GetSize() const {
   return m_Size;
}


// The caller is responsible for making sure the GPU is no longer using the buffer.
void Buffer::Release() {
   if (!m_Device) {
      return;
   }
   const auto& nri = m_Device->GetNri();
   nri.DestroyBuffer(m_Buffer);
   for (nri::Memory* memory : m_Memory) {
      nri.FreeMemory(memory);
   }
   m_Buffer = nullptr;
   m_Memory.clear();
   m_Device = nullptr;
}

}
