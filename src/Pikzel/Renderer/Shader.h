#pragma once

#include <cstddef>
#include <filesystem>
#include <span>
#include <string_view>
#include <vector>

namespace Pikzel {

// Compiled shader code, loaded from disk.
//
// Shaders are compiled from Slang to SPIR-V at build time by pikzel_add_shaders() (see
// cmake/Shaders.cmake), which writes <name>.spv into a Shaders directory next to the
// executable. One file holds every entry point of its source file; a pipeline picks the
// entry points it needs by name.
//
// A Shader is plain CPU-side data, not a GPU object: the GPU driver compiles the code
// into its own machine code when a pipeline is created. Once the pipelines using it exist,
// the Shader can be discarded.
class Shader {
public:
   // Loads Shaders/<name>.spv from the executable's directory, for example
   // Shader("Triangle") for a shader built from Triangle.slang.
   // Throws std::runtime_error if the file cannot be read.
   explicit Shader(std::string_view name);

   std::span<const std::byte> GetBytecode() const;
   const std::filesystem::path& GetPath() const;

private:
   std::filesystem::path m_Path;
   std::vector<std::byte> m_Bytecode;
};

}
