#pragma once

#include "RenderCore.h"

#include <spirv_glsl.hpp>

namespace Pikzel {

   PKZL_API DataType SPIRTypeToDataType(const spirv_cross::SPIRType type);

}
