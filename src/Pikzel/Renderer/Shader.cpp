#include "Pikzel/Renderer/Shader.h"
#include "Pikzel/Core/Filesystem.h"

#include <string>

namespace Pikzel {

Shader::Shader(const std::string_view name)
: m_Path(Filesystem::GetExecutableDirectory() / "Shaders" / (std::string(name) + ".spv")) {
   m_Bytecode = Filesystem::ReadBinaryFile(m_Path);
}


std::span<const std::byte> Shader::GetBytecode() const {
   return m_Bytecode;
}


const std::filesystem::path& Shader::GetPath() const {
   return m_Path;
}

}
