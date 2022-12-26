#pragma once

#include <yaml-cpp/yaml.h>

namespace Pikzel {

   template<typename T>
   void Serialize(YAML::Emitter& yaml, const T& component); // not defined on purpose.  you must specialize


   template<typename T>
   void Deserialize(YAML::Node node, T& component); // not defined on purpose.  you must specialize

}