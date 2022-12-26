#pragma once

#include "Pikzel/Scene/Scene.h"

#include <filesystem>

namespace Pikzel {

   struct PKZL_API SceneSerializerSettings {
      std::filesystem::path Path;
   };

   class PKZL_API SceneSerializerYAML final {
   public:
      SceneSerializerYAML(const SceneSerializerSettings& settings);

      void Serialize(const Pikzel::Scene& scene);

      std::unique_ptr<Pikzel::Scene> Deserialize();

   private:
      SceneSerializerSettings m_Settings;
   };

}
