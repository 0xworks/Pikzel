#include "FileSystem.h"

CMRC_DECLARE(PikzelResources);

namespace Pikzel {

   PKZL_API cmrc::embedded_filesystem GetEmbeddedFileSystem() {
      return cmrc::PikzelResources::get_filesystem();
   }

}
