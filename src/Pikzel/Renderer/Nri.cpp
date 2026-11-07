#include "Pikzel/Renderer/Nri.h"

#include <format>
#include <stdexcept>

namespace Pikzel {

std::string_view ToString(const nri::Result result) {
   switch (result) {
      case nri::Result::NOT_READY:
         return "NOT_READY";
      case nri::Result::DEVICE_LOST:
         return "DEVICE_LOST";
      case nri::Result::OUT_OF_DATE:
         return "OUT_OF_DATE";
      case nri::Result::INVALID_SDK:
         return "INVALID_SDK";
      case nri::Result::SUCCESS:
         return "SUCCESS";
      case nri::Result::FAILURE:
         return "FAILURE";
      case nri::Result::INVALID_ARGUMENT:
         return "INVALID_ARGUMENT";
      case nri::Result::OUT_OF_MEMORY:
         return "OUT_OF_MEMORY";
      case nri::Result::UNSUPPORTED:
         return "UNSUPPORTED";
      default:
         return "UNKNOWN";
   }
}


void CheckNri(const nri::Result result, const std::string_view operation) {
   if (result != nri::Result::SUCCESS) {
      throw std::runtime_error(std::format("{} failed: {}", operation, ToString(result)));
   }
}


// The engine's Format enum is a small subset of NRI's. The two conversion functions are
// kept side by side so that adding a format means updating both together.
nri::Format ToNri(const Format format) {
   switch (format) {
      case Format::R32Float:
         return nri::Format::R32_SFLOAT;
      case Format::RG32Float:
         return nri::Format::RG32_SFLOAT;
      case Format::RGB32Float:
         return nri::Format::RGB32_SFLOAT;
      case Format::RGBA32Float:
         return nri::Format::RGBA32_SFLOAT;
      case Format::RGBA8Unorm:
         return nri::Format::RGBA8_UNORM;
      case Format::RGBA8Srgb:
         return nri::Format::RGBA8_SRGB;
      case Format::BGRA8Unorm:
         return nri::Format::BGRA8_UNORM;
      case Format::BGRA8Srgb:
         return nri::Format::BGRA8_SRGB;
      case Format::RGB10A2Unorm:
         return nri::Format::R10_G10_B10_A2_UNORM;
      case Format::RGBA16Float:
         return nri::Format::RGBA16_SFLOAT;
      case Format::Unknown:
      default:
         return nri::Format::UNKNOWN;
   }
}


Format FromNri(const nri::Format format) {
   switch (format) {
      case nri::Format::R32_SFLOAT:
         return Format::R32Float;
      case nri::Format::RG32_SFLOAT:
         return Format::RG32Float;
      case nri::Format::RGB32_SFLOAT:
         return Format::RGB32Float;
      case nri::Format::RGBA32_SFLOAT:
         return Format::RGBA32Float;
      case nri::Format::RGBA8_UNORM:
         return Format::RGBA8Unorm;
      case nri::Format::RGBA8_SRGB:
         return Format::RGBA8Srgb;
      case nri::Format::BGRA8_UNORM:
         return Format::BGRA8Unorm;
      case nri::Format::BGRA8_SRGB:
         return Format::BGRA8Srgb;
      case nri::Format::R10_G10_B10_A2_UNORM:
         return Format::RGB10A2Unorm;
      case nri::Format::RGBA16_SFLOAT:
         return Format::RGBA16Float;
      default:
         return Format::Unknown;
   }
}

}
