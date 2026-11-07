#include "Pikzel/Renderer/Texture.h"
#include "Pikzel/Renderer/CommandList.h"
#include "Pikzel/Renderer/GraphicsDevice.h"
#include "Pikzel/Renderer/Nri.h"

#include <cassert>
#include <format>
#include <stdexcept>
#include <utility>

namespace Pikzel {

namespace {

nri::TextureUsageBits ToNri(const TextureUsage usage) {
   auto bits = nri::TextureUsageBits::NONE;
   if (HasFlag(usage, TextureUsage::ShaderResource)) {
      bits |= nri::TextureUsageBits::SHADER_RESOURCE;
   }
   if (HasFlag(usage, TextureUsage::ColorAttachment)) {
      bits |= nri::TextureUsageBits::COLOR_ATTACHMENT;
   }
   if (HasFlag(usage, TextureUsage::DepthAttachment)) {
      bits |= nri::TextureUsageBits::DEPTH_STENCIL_ATTACHMENT;
   }
   return bits;
}

}


Texture::Texture(GraphicsDevice& device, const TextureSettings& settings)
: m_Device(&device), m_Settings(settings) {
   if (settings.width == 0 || settings.height == 0) {
      throw std::invalid_argument("TextureSettings::width and height must be non-zero");
   }
   if (settings.format == Format::Unknown) {
      throw std::invalid_argument("TextureSettings::format must be set");
   }
   if (HasFlag(settings.usage, TextureUsage::DepthAttachment) != IsDepthFormat(settings.format)) {
      throw std::invalid_argument("TextureUsage::DepthAttachment requires a depth format, and depth formats require DepthAttachment usage");
   }

   const auto& nri = device.GetNri();

   nri::TextureDesc desc = {};
   desc.type = nri::TextureType::TEXTURE_2D;
   desc.usage = ToNri(settings.usage);
   desc.format = ToNri(settings.format);
   desc.width = ToNriDim(settings.width);
   desc.height = ToNriDim(settings.height);
   desc.depth = 1;
   desc.mipNum = 1;
   desc.layerNum = 1;
   desc.sampleNum = 1;
   CheckNri(nri.CreateTexture(device.GetDevice(), desc, m_Texture), "CreateTexture");

   // As in Buffer, release what has been created so far if a later step throws.
   try {
      nri::ResourceGroupDesc resourceGroupDesc = {};
      resourceGroupDesc.memoryLocation = nri::MemoryLocation::DEVICE;
      resourceGroupDesc.textures = &m_Texture;
      resourceGroupDesc.textureNum = 1;

      m_Memory.resize(nri.CalculateAllocationNumber(device.GetDevice(), resourceGroupDesc), nullptr);
      CheckNri(nri.AllocateAndBindMemory(device.GetDevice(), resourceGroupDesc, m_Memory.data()), "AllocateAndBindMemory");

      // mipNum and layerNum of REMAINING (zero) mean "all of them", here just the one.
      if (HasFlag(settings.usage, TextureUsage::ShaderResource)) {
         const nri::TextureViewDesc viewDesc = {.texture = m_Texture, .type = nri::TextureView::TEXTURE, .format = desc.format};
         CheckNri(nri.CreateTextureView(viewDesc, m_ShaderResourceView), "CreateTextureView");
      }
      if (HasFlag(settings.usage, TextureUsage::ColorAttachment)) {
         const nri::TextureViewDesc viewDesc = {.texture = m_Texture, .type = nri::TextureView::COLOR_ATTACHMENT, .format = desc.format};
         CheckNri(nri.CreateTextureView(viewDesc, m_AttachmentView), "CreateTextureView");
      } else if (HasFlag(settings.usage, TextureUsage::DepthAttachment)) {
         const nri::TextureViewDesc viewDesc = {.texture = m_Texture, .type = nri::TextureView::DEPTH_STENCIL_ATTACHMENT, .format = desc.format};
         CheckNri(nri.CreateTextureView(viewDesc, m_AttachmentView), "CreateTextureView");
      }
   } catch (...) {
      Release();
      throw;
   }
}


Texture::Texture(GraphicsDevice& device, const TextureSettings& settings, const std::span<const std::byte> data)
: Texture(device, settings) {
   // From here on the object is fully constructed (the delegated constructor finished), so
   // if anything throws, the destructor releases the texture.
   if (!HasFlag(settings.usage, TextureUsage::ShaderResource)) {
      throw std::invalid_argument("A texture created with initial data must have TextureUsage::ShaderResource");
   }

   const auto* formatProps = nri::nriGetFormatProps(ToNri(settings.format));
   const uint32_t rowPitch = settings.width * formatProps->stride;
   const uint32_t slicePitch = rowPitch * settings.height;
   if (data.size() != slicePitch) {
      throw std::invalid_argument(std::format("Texture data is {} bytes, but a {}x{} {} texture needs {}", data.size(), settings.width, settings.height, formatProps->name, slicePitch));
   }

   // NRI copies the rows into a staging buffer with whatever row alignment the graphics API
   // requires, so the source rows can be tightly packed.
   const nri::TextureSubresourceUploadDesc subresource = {
      .slices = data.data(),
      .sliceNum = 1,
      .rowPitch = rowPitch,
      .slicePitch = slicePitch,
   };

   nri::TextureUploadDesc uploadDesc = {};
   uploadDesc.subresources = &subresource;
   uploadDesc.texture = m_Texture;
   uploadDesc.after = {nri::AccessBits::SHADER_RESOURCE, nri::Layout::SHADER_RESOURCE, nri::StageBits::VERTEX_SHADER | nri::StageBits::FRAGMENT_SHADER};
   CheckNri(device.GetNri().UploadData(device.GetGraphicsQueue(), &uploadDesc, 1, nullptr, 0), "UploadData");
}


Texture::~Texture() {
   Release();
}


Texture::Texture(Texture&& other) noexcept
: m_Device(std::exchange(other.m_Device, nullptr)), m_Texture(std::exchange(other.m_Texture, nullptr)), m_ShaderResourceView(std::exchange(other.m_ShaderResourceView, nullptr)), m_AttachmentView(std::exchange(other.m_AttachmentView, nullptr)), m_Memory(std::exchange(other.m_Memory, {})), m_Settings(std::exchange(other.m_Settings, {})) {}


Texture& Texture::operator=(Texture&& other) noexcept {
   if (this != &other) {
      Release();
      m_Device = std::exchange(other.m_Device, nullptr);
      m_Texture = std::exchange(other.m_Texture, nullptr);
      m_ShaderResourceView = std::exchange(other.m_ShaderResourceView, nullptr);
      m_AttachmentView = std::exchange(other.m_AttachmentView, nullptr);
      m_Memory = std::exchange(other.m_Memory, {});
      m_Settings = std::exchange(other.m_Settings, {});
   }
   return *this;
}


uint32_t Texture::GetWidth() const {
   return m_Settings.width;
}


uint32_t Texture::GetHeight() const {
   return m_Settings.height;
}


Format Texture::GetFormat() const {
   return m_Settings.format;
}


TextureUsage Texture::GetUsage() const {
   return m_Settings.usage;
}


RenderTarget Texture::GetRenderTarget() const {
   assert(m_AttachmentView && "Only textures with ColorAttachment or DepthAttachment usage can be rendered into");
   return RenderTarget(*m_AttachmentView, m_Settings.width, m_Settings.height);
}


nri::Texture& Texture::GetNriTexture() const {
   return *m_Texture;
}


nri::Descriptor& Texture::GetNriShaderResourceView() const {
   assert(m_ShaderResourceView && "Only textures with ShaderResource usage can be read by shaders");
   return *m_ShaderResourceView;
}


// The caller is responsible for making sure the GPU is no longer using the texture.
void Texture::Release() {
   if (!m_Device) {
      return;
   }
   const auto& nri = m_Device->GetNri();
   nri.DestroyDescriptor(m_ShaderResourceView);
   nri.DestroyDescriptor(m_AttachmentView);
   nri.DestroyTexture(m_Texture);
   for (nri::Memory* memory : m_Memory) {
      nri.FreeMemory(memory);
   }
   m_ShaderResourceView = nullptr;
   m_AttachmentView = nullptr;
   m_Texture = nullptr;
   m_Memory.clear();
   m_Device = nullptr;
}

}
