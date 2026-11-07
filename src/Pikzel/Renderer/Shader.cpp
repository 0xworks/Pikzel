#include "Pikzel/Renderer/Shader.h"
#include "Pikzel/Core/Filesystem.h"
#include "Pikzel/Renderer/GraphicsDevice.h"

#include <format>

namespace Pikzel {

namespace {

std::string_view GetBytecodeExtension(const GraphicsApi api) {
   switch (api) {
      case GraphicsApi::D3D12:
         return "dxil";
      case GraphicsApi::Vulkan:
      default:
         return "spv";
   }
}

}


Shader::Shader(const GraphicsDevice& device, const std::string_view name, const std::string_view entryPoint)
: m_EntryPoint(entryPoint), m_Path(Filesystem::GetExecutableDirectory() / "Shaders" / std::format("{}.{}.{}", name, entryPoint, GetBytecodeExtension(device.GetApi()))) {
   m_Bytecode = Filesystem::ReadBinaryFile(m_Path);
}


std::span<const std::byte> Shader::GetBytecode() const {
   return m_Bytecode;
}


const std::string& Shader::GetEntryPoint() const {
   return m_EntryPoint;
}


const std::filesystem::path& Shader::GetPath() const {
   return m_Path;
}

}
