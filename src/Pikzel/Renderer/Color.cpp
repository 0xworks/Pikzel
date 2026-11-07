#include "Pikzel/Renderer/Color.h"

#include <algorithm>
#include <cassert>
#include <cmath>

namespace Pikzel {

namespace {

// sRGB values, and alpha, are only defined on [0, 1]. Passing anything else (including
// NaN, which fails the comparison) is a programmer error, so it asserts in debug builds.
// Release builds clamp instead, so a bad value cannot propagate further.
float CheckUnitRange(const float value) {
   assert(value >= 0.0f && value <= 1.0f && "Color component must be in the range [0, 1]");
   return std::clamp(value, 0.0f, 1.0f);
}

}


Color Color::FromSrgb(const float r, const float g, const float b, const float a) {
   return {SrgbToLinear(r), SrgbToLinear(g), SrgbToLinear(b), CheckUnitRange(a)};
}


Color Color::FromSrgb8(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a) {
   return FromSrgb(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}


Color Color::FromSrgbHex(const uint32_t rgb, const float a) {
   assert(rgb <= 0xFFFFFF && "Hex color must be 0xRRGGBB; alpha is a separate argument");
   const auto r = static_cast<uint8_t>((rgb >> 16) & 0xFF);
   const auto g = static_cast<uint8_t>((rgb >> 8) & 0xFF);
   const auto b = static_cast<uint8_t>(rgb & 0xFF);
   return FromSrgb(r / 255.0f, g / 255.0f, b / 255.0f, a);
}


float SrgbToLinear(float value) {
   value = CheckUnitRange(value);
   if (value <= 0.04045f) {
      return value / 12.92f;
   }
   return std::pow((value + 0.055f) / 1.055f, 2.4f);
}


float LinearToSrgb(float value) {
   value = CheckUnitRange(value);
   if (value <= 0.0031308f) {
      return value * 12.92f;
   }
   return 1.055f * std::pow(value, 1.0f / 2.4f) - 0.055f;
}

}
