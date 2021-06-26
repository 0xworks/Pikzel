#include "SceneSerializer.h"

#include <cereal/archives/json.hpp>

#include <fstream>

namespace Pikzel {

   SceneSerializerJSON::SceneSerializerJSON(const SerializerSettings& settings)
   : m_Settings{settings}
   {}


   void SceneSerializerJSON::Serialize(const Scene& scene) {
      std::ofstream out(m_Settings.Path.string().c_str());
      cereal::JSONOutputArchive archive(out);
      archive(cereal::make_nvp("Scene", scene));
   }


   std::unique_ptr<Scene> SceneSerializerJSON::Deserialise() {
      std::unique_ptr<Scene> scene = std::make_unique<Scene>();
      std::ifstream in(m_Settings.Path.string().c_str());
      cereal::JSONInputArchive archive(in);
      archive(cereal::make_nvp("Scene", *scene));
      return scene;
   }

}
