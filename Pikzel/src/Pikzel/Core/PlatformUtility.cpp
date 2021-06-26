#include "PlatformUtility.h"
#include <tinyfiledialogs.h>


namespace Pikzel {

   std::optional<std::filesystem::path> OpenFileDialog(const char* filter, const char* fileDescription) {
      const char* file = tinyfd_openFileDialog(
         "",
         "",
         filter? 1 : 0,
         filter? &filter : nullptr,
         fileDescription,
         0
      );
      if(file) {
         return file;
      }
      return std::nullopt;
   }


   std::optional<std::filesystem::path> SaveFileDialog(const char* filter, const char* fileDescription) {
      const char* file = tinyfd_saveFileDialog(
         "",
         "",
         filter? 1 : 0,
         filter? &filter : nullptr,
         fileDescription
      );
      if(file) {
         return file;
      }
      return std::nullopt;
   }

}
