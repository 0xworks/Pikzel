#pragma once

#include "Pikzel/Scene/Scene.h"

#include <filesystem>

namespace Pikzel {

   struct PKZL_API SerializerSettings {
      std::filesystem::path Path;
   };


   class PKZL_API SceneSerializerYAML final {
   public:
      SceneSerializerYAML(const SerializerSettings& settings);

      void Serialize(const Pikzel::Scene& scene);

      std::unique_ptr<Pikzel::Scene> Deserialise();

   private:
      SerializerSettings m_Settings;
   };

}
