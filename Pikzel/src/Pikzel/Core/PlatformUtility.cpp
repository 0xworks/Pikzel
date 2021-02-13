#include "PlatformUtility.h"
#include <tinyfiledialogs.h>


namespace Pikzel {

   std::optional<std::filesystem::path> OpenFileDialog(const char* filter /*= nullptr*/) {
      const char* file = tinyfd_openFileDialog(
         "",
         "",
         filter? 1 : 0,
         filter? nullptr : &filter,
         nullptr,
         0
      );
      if(file) {
         return file;
      }
      return std::nullopt;
   }


   std::optional<std::filesystem::path> SaveFileDialog(const char* filter /* = nullptr*/) {
      const char* file = tinyfd_saveFileDialog(
         "",
         "",
         filter? 1 : 0,
         filter? nullptr : &filter,
         nullptr
      );
      if(file) {
         return file;
      }
      return std::nullopt;
   }

}
