#pragma once

#include <cstdint>

namespace Pikzel {

// An RGBA color with linear components.
//
// Linear vs sRGB
// --------------
// Color values come in two encodings:
//  - Linear values are proportional to light intensity: 0.5 is half as bright as 1.0.
//    Lighting, blending and interpolation are only correct on linear values.
//  - sRGB values are gamma-encoded to match human perception and the display: 0.5 is
//    perceived as roughly half as bright, but emits only about 21% as much light. Colors
//    from color pickers, image editors, hex codes and CSS are all sRGB.
//
// The engine works in linear space throughout: Color always holds linear values, and
// shaders receive and output linear values. Conversion to sRGB for display happens in
// hardware, when the GPU writes to the swap chain's sRGB render target.
//
// To use a color from a picker or a design, convert it on the way in with one of the
// FromSrgb functions, for example Color::FromSrgbHex(0xFF8000). Constructing a Color
// directly, as Color{r, g, b, a}, means the values are already linear.
//
// Alpha is coverage or opacity, not light, so it is never encoded: it is passed through
// unchanged by every conversion.
//
// sRGB components and alpha must be in the range [0, 1]. Values outside it trigger an
// assertion in debug builds and are clamped in release builds. (Linear values in a Color
// constructed directly are not checked: HDR rendering will legitimately exceed 1.0.)
struct Color {
   float r = 0.0f;
   float g = 0.0f;
   float b = 0.0f;
   float a = 1.0f;

   // From sRGB components in the range [0, 1].
   static Color FromSrgb(float r, float g, float b, float a = 1.0f);

   // From 8-bit sRGB components, as found in most image files and color pickers.
   static Color FromSrgb8(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

   // From a 24-bit sRGB hex code, 0xRRGGBB, as used in HTML and CSS.
   static Color FromSrgbHex(uint32_t rgb, float a = 1.0f);
};

// The sRGB transfer functions, for a single component in the range [0, 1]. They are the
// exact piecewise functions from the sRGB standard (IEC 61966-2-1): a short linear segment
// near black, and a power curve with exponent 2.4 elsewhere. Inputs outside [0, 1] are
// handled as described for Color above.
float SrgbToLinear(float value);
float LinearToSrgb(float value);

}
