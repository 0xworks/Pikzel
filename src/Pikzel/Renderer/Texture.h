#pragma once

#include "Pikzel/Renderer/Format.h"

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace nri {
struct Descriptor;
struct Memory;
struct Texture;
}

namespace Pikzel {

class GraphicsDevice;
class RenderTarget;

// How a texture will be used. Flags can be combined, e.g. ColorAttachment | ShaderResource
// for a texture that is rendered into and later sampled.
enum class TextureUsage : uint32_t {
   None = 0,
   // Sampled (read) by shaders.
   ShaderResource = 1 << 0,
   // Rendered into as a color target.
   ColorAttachment = 1 << 1,
   // Used as a depth buffer.
   DepthAttachment = 1 << 2,
};

constexpr TextureUsage operator|(const TextureUsage lhs, const TextureUsage rhs) {
   return static_cast<TextureUsage>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

constexpr bool HasFlag(const TextureUsage usage, const TextureUsage flag) {
   return (static_cast<uint32_t>(usage) & static_cast<uint32_t>(flag)) != 0;
}

struct TextureSettings {
   uint32_t width = 0;
   uint32_t height = 0;
   Format format = Format::Unknown;
   TextureUsage usage = TextureUsage::ShaderResource;
};

// A 2D GPU texture, the memory backing it, and the views (descriptors) through which it is
// used.
//
// A GPU never accesses a texture directly: shaders read it through a shader resource view,
// and rendering writes it through an attachment view. Views also fix how the texels are
// interpreted, which is how an sRGB texture is decoded to linear values when sampled.
// Texture creates the views its usage needs.
//
// Color textures (albedo, UI) should use an *Srgb format, so that shaders receive linear
// values and filtering happens in linear space. Data textures (normal maps, roughness)
// hold values that are already linear and use *Unorm formats.
//
// Textures currently have a single mip level and no array layers. Mip chains will arrive
// with the asset pipeline, which generates them offline (KTX files store them).
class Texture {
public:
   // Creates a texture with undefined contents, for example a render target or depth
   // buffer that will be cleared before use.
   Texture(GraphicsDevice& device, const TextureSettings& settings);

   // Creates a texture and uploads data into it. data holds tightly packed rows of texels,
   // top row first, in the texture's format. The texture is left ready to be sampled by
   // shaders, so settings.usage must include ShaderResource. Like Buffer's initial upload,
   // this waits for the GPU to finish.
   Texture(GraphicsDevice& device, const TextureSettings& settings, std::span<const std::byte> data);

   ~Texture();

   Texture(const Texture&) = delete;
   Texture& operator=(const Texture&) = delete;
   Texture(Texture&& other) noexcept;
   Texture& operator=(Texture&& other) noexcept;

   uint32_t GetWidth() const;
   uint32_t GetHeight() const;
   Format GetFormat() const;
   TextureUsage GetUsage() const;

   // The texture as a target for rendering. Requires ColorAttachment or DepthAttachment usage.
   RenderTarget GetRenderTarget() const;

   // Renderer internal.
   nri::Texture& GetNriTexture() const;
   nri::Descriptor& GetNriShaderResourceView() const;

private:
   void Release();

   GraphicsDevice* m_Device = nullptr;
   nri::Texture* m_Texture = nullptr;
   nri::Descriptor* m_ShaderResourceView = nullptr;
   nri::Descriptor* m_AttachmentView = nullptr;
   std::vector<nri::Memory*> m_Memory;
   TextureSettings m_Settings;
};

}
