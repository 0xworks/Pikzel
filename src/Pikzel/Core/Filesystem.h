#pragma once

#include <cstddef>
#include <filesystem>
#include <vector>

// Small filesystem helpers that the standard library does not provide directly.
namespace Pikzel::Filesystem {

// Returns the directory containing the running executable.
//
// Files produced by the build (such as compiled shaders) are placed next to the
// executable, so they are located relative to it rather than to the current working
// directory, which depends on how the program was launched.
std::filesystem::path GetExecutableDirectory();

// Reads an entire file into memory. Throws std::runtime_error if the file cannot be read.
std::vector<std::byte> ReadBinaryFile(const std::filesystem::path& path);

}
