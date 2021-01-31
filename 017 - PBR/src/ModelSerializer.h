#pragma once

#include "Model.h"

#include <filesystem>
#include <memory>

namespace PBRdemo {

   namespace ModelSerializer {
      std::unique_ptr<Model> Import(const std::filesystem::path& path);
   }

}
