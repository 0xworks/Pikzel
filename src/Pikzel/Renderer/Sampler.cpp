#include "Pikzel/Renderer/Sampler.h"
#include "Pikzel/Renderer/GraphicsDevice.h"
#include "Pikzel/Renderer/Nri.h"

#include <algorithm>
#include <utility>

namespace Pikzel {

namespace {

nri::Filter ToNri(const Filter filter) {
   switch (filter) {
      case Filter::Nearest:
         return nri::Filter::NEAREST;
      case Filter::Linear:
      default:
         return nri::Filter::LINEAR;
   }
}


nri::AddressMode ToNri(const AddressMode addressMode) {
   switch (addressMode) {
      case AddressMode::MirroredRepeat:
         return nri::AddressMode::MIRRORED_REPEAT;
      case AddressMode::ClampToEdge:
         return nri::AddressMode::CLAMP_TO_EDGE;
      case AddressMode::Repeat:
      default:
         return nri::AddressMode::REPEAT;
   }
}

}


Sampler::Sampler(GraphicsDevice& device, const SamplerSettings& settings)
: m_Device(&device) {
   const auto filter = ToNri(settings.filter);
   const auto addressMode = ToNri(settings.addressMode);

   nri::SamplerDesc desc = {};
   desc.filters = {.min = filter, .mag = filter, .mip = filter};
   desc.addressModes = {.u = addressMode, .v = addressMode, .w = addressMode};
   // NRI treats 0 and 1 alike (anisotropy off). Values above 16 are clamped, as no
   // hardware supports more.
   desc.anisotropy = static_cast<uint8_t>(std::clamp(settings.maxAnisotropy, 1u, 16u));
   // Allow every mip level a texture has. The mip range is a property of the sampler, not
   // the texture, so it cannot be tailored to each texture.
   desc.mipMax = 16.0f;

   CheckNri(device.GetNri().CreateSampler(device.GetDevice(), desc, m_Sampler), "CreateSampler");
}


Sampler::~Sampler() {
   Release();
}


Sampler::Sampler(Sampler&& other) noexcept
: m_Device(std::exchange(other.m_Device, nullptr)), m_Sampler(std::exchange(other.m_Sampler, nullptr)) {}


Sampler& Sampler::operator=(Sampler&& other) noexcept {
   if (this != &other) {
      Release();
      m_Device = std::exchange(other.m_Device, nullptr);
      m_Sampler = std::exchange(other.m_Sampler, nullptr);
   }
   return *this;
}


nri::Descriptor& Sampler::GetNriSampler() const {
   return *m_Sampler;
}


// The caller is responsible for making sure the GPU is no longer using the sampler.
void Sampler::Release() {
   if (m_Device) {
      m_Device->GetNri().DestroyDescriptor(m_Sampler);
      m_Sampler = nullptr;
      m_Device = nullptr;
   }
}

}
