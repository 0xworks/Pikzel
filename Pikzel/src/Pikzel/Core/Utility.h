#pragma once

#include "Pikzel/Core/Application.h"

#include <cmrc/cmrc.hpp>

#include <filesystem>
#include <fstream>
#include <vector>

CMRC_DECLARE(PikzelResources);

namespace Pikzel {

   std::vector<char> ReadFile(const std::filesystem::path& path, const bool readAsBinary);


   template<typename T>
   std::vector<T> ReadCMRCFile(const std::string& path) {
      try {
         auto fs = cmrc::PikzelResources::get_filesystem();
         auto file = fs.open(path);

         size_t fileSize = file.size();
         if (fileSize % sizeof(T) != 0) {
            throw std::runtime_error(fmt::format("Size of file '{0}' is {1}.  Expected a multiple of {2}!", path, fileSize, sizeof(T)));
         }

         std::vector<T> buffer(file.size() / sizeof(T));
         memcpy(buffer.data(), file.begin(), fileSize);

         return buffer;

      } catch (std::system_error) {
         throw std::runtime_error(fmt::format("Could not access file '{0}'!", path));
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
         
         // 2) try application binary directory
         std::filesystem::path bindir = Application::Get().GetArgV()[0];
         bindir.remove_filename();
         file.open(bindir / path, std::ios::ate | std::ios::binary);
         if (!file.is_open()) {

            // 3) try Pikzel embedded CMRC filesystem
            return ReadCMRCFile<T>(path.string());
         }
      }

      std::streampos fileSize = file.tellg();
      if (fileSize % sizeof(T) != 0) {
         throw std::runtime_error(fmt::format("Size of file '{0}' is {1}.  Expected a multiple of {2}!", path.string(), fileSize, sizeof(T)));
      }
      file.seekg(0);

      std::vector<T> buffer(fileSize / sizeof(T));
      file.read(reinterpret_cast<char*>(buffer.data()), fileSize);

      return buffer;
   }

}
