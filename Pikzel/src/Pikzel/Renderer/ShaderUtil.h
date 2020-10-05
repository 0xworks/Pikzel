#pragma once

#include "RenderCore.h"

#include <spirv_cross/spirv_glsl.hpp>

namespace Pikzel {

   DataType SPIRTypeToDataType(const spirv_cross::SPIRType type);

}
