#pragma once

#include "Pikzel/Core/Application.h"
#include "Pikzel/Core/FileSystem.h"

#include <filesystem>
#include <fstream>
#include <vector>

namespace Pikzel {

   template<typename T>
   std::vector<T> ReadCMRCFile(const std::string& path) {
      try {
         auto fs = GetEmbeddedFileSystem();
         auto file = fs.open(path);

         size_t fileSize = file.size();
         if (fileSize % sizeof(T) != 0) {
            throw std::runtime_error {fmt::format("Size of file '{}' is {}.  Expected a multiple of {}!", path, fileSize, sizeof(T))};
         }

         std::vector<T> buffer(file.size() / sizeof(T));
         memcpy(buffer.data(), file.begin(), fileSize);

         return buffer;

      } catch (const std::system_error&) {
         throw std::runtime_error {fmt::format("Could not access file '{}'!", path)};
      }
   }


   template<typename T>
   std::vector<T> ReadFile(const std::filesystem::path& path) {

      // search paths for the file are as follows:
      // 1) The current working directory
      // 2) The application binary directory
      // 3) The Pikzel embedded cmrc filesystem

      // 1) try current working directory (i.e. use path exactly as passed in)
      std::ifstream file(path, std::ios::ate | std::ios::binary);
      if (!file.is_open()) {
         
         // 2) try application "root" directory
         std::filesystem::path root = Application::Get().GetRootDir();
         file.open(root / path, std::ios::ate | std::ios::binary);
         if (!file.is_open()) {

            // 3) try Pikzel embedded CMRC filesystem
            return ReadCMRCFile<T>(path.string());
         }
      }

      std::streampos fileSize = file.tellg();
      if (fileSize % sizeof(T) != 0) {
         throw std::runtime_error {fmt::format("Size of file '{}' is {}.  Expected a multiple of {}!", path, static_cast<size_t>(fileSize), sizeof(T))};
      }
      file.seekg(0);

      std::vector<T> buffer(fileSize / sizeof(T));
      file.read(reinterpret_cast<char*>(buffer.data()), fileSize);

      return buffer;
   }


   inline std::unique_ptr<std::istream> IStream(const std::filesystem::path& path) {
      // search paths for the stream are as follows:
      // 1) The current working directory
      // 2) The application binary directory
      // 3) The Pikzel embedded cmrc filesystem

      // 1) try current working directory (i.e. use path exactly as passed in)
      auto file = std::make_unique<std::ifstream>(path, std::ios::binary);
      if (!file->is_open()) {

         // 2) try application "root" directory
         std::filesystem::path root = Application::Get().GetRootDir();
         file->open(root / path, std::ios::binary);
         if (!file->is_open()) {

            // 3) try Pikzel embedded CMRC filesystem
            auto fs = GetEmbeddedFileSystem();
            auto cmrcFile = fs.open(path.string());
            return std::make_unique<std::istringstream>(std::string(cmrcFile.begin(), cmrcFile.end()));
         }
      }
      return file;
   }
}
