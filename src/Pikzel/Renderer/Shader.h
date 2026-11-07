#pragma once

#include <cstddef>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace Pikzel {

class GraphicsDevice;

// One compiled shader entry point (for example a vertex shader), loaded from disk.
//
// Shaders are compiled from Slang at build time by pikzel_add_shader() (see
// cmake/Shaders.cmake), which writes one file per entry point into a Shaders directory
// next to the executable: <source>.<entry point>.spv (SPIR-V) for Vulkan, and
// <source>.<entry point>.dxil for D3D12 when that backend is built. The constructor loads
// the file for the device's graphics API, so the same code works on either API.
//
// A Shader is plain CPU-side data, not a GPU object: the GPU driver compiles the code
// into its own machine code when a pipeline is created. Once the pipelines using it exist,
// the Shader can be discarded.
class Shader {
public:
   // Loads Shaders/<name>.<entryPoint>.<spv|dxil> from the executable's directory, for
   // example Shader(device, "Triangle", "VertexMain") for the VertexMain entry point of
   // Triangle.slang. The device is only used to find out which graphics API to load for.
   // Throws std::runtime_error if the file cannot be read.
   Shader(const GraphicsDevice& device, std::string_view name, std::string_view entryPoint);

   std::span<const std::byte> GetBytecode() const;
   const std::string& GetEntryPoint() const;
   const std::filesystem::path& GetPath() const;

private:
   std::string m_EntryPoint;
   std::filesystem::path m_Path;
   std::vector<std::byte> m_Bytecode;
};

}
