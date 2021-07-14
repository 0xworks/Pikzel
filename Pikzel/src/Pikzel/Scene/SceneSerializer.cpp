#include "SceneSerializer.h"

#include "Pikzel/Components/Model.h"
#include "Pikzel/Components/Transform.h"

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <yaml-cpp/yaml.h>
#include <fstream>

namespace Pikzel {

   template<typename T>
   void Serialize(YAML::Emitter& yaml, const T& component); // not defined on purpose.  you must use a template specialization


   template<>
   void Serialize<Id>(YAML::Emitter& yaml, const Id& id) {
      yaml << YAML::Key << "Id" << YAML::Value << id;
   }


   template<>
   void Serialize<glm::vec2>(YAML::Emitter& yaml, const glm::vec2& vec) {
      yaml << YAML::Flow << YAML::BeginMap;
      {
         yaml << YAML::Key << "x" << YAML::Value << vec.x;
         yaml << YAML::Key << "y" << YAML::Value << vec.y;
      }
      yaml << YAML::EndMap;
   }


   template<>
   void Serialize<glm::vec3>(YAML::Emitter& yaml, const glm::vec3& vec) {
      yaml << YAML::Flow << YAML::BeginMap;
      {
         yaml << YAML::Key << "x" << YAML::Value << vec.x;
         yaml << YAML::Key << "y" << YAML::Value << vec.y;
         yaml << YAML::Key << "z" << YAML::Value << vec.z;
      }
      yaml << YAML::EndMap;
   }


   template<>
   void Serialize<glm::vec4>(YAML::Emitter& yaml, const glm::vec4& vec) {
      yaml << YAML::Flow << YAML::BeginMap;
      {
         yaml << YAML::Key << "x" << YAML::Value << vec.x;
         yaml << YAML::Key << "y" << YAML::Value << vec.y;
         yaml << YAML::Key << "z" << YAML::Value << vec.z;
         yaml << YAML::Key << "w" << YAML::Value << vec.w;
      }
      yaml << YAML::EndMap;
   }


   template<>
   void Serialize<glm::quat>(YAML::Emitter& yaml, const glm::quat& quat) {
      yaml << YAML::Flow << YAML::BeginMap;
      {
         yaml << YAML::Key << "x" << YAML::Value << quat.x;
         yaml << YAML::Key << "y" << YAML::Value << quat.y;
         yaml << YAML::Key << "z" << YAML::Value << quat.z;
         yaml << YAML::Key << "w" << YAML::Value << quat.w;
      }
      yaml << YAML::EndMap;
   }


   template<>
   void Serialize<Transform>(YAML::Emitter& yaml, const Transform& transform) {
      glm::vec3 position;
      glm::quat rotation;
      glm::vec3 scale;
      glm::vec3 skew;
      glm::vec4 perspective;
      glm::decompose(transform.Matrix, scale, rotation, position, skew, perspective);

      yaml << YAML::Key << "Transform";
      yaml << YAML::Value << YAML::BeginMap;
      {
         yaml << YAML::Key << "Position" << YAML::Value; Serialize(yaml, position);
         yaml << YAML::Key << "Rotation" << YAML::Value; Serialize(yaml, rotation);
         yaml << YAML::Key << "Scale" << YAML::Value; Serialize(yaml, scale);
      }
      yaml << YAML::EndMap;
   }


   template<>
   void Serialize<Model>(YAML::Emitter& yaml, const Model& model) {
      yaml << YAML::Key << "Model";
      yaml << YAML::Value << YAML::BeginMap;
      {
         yaml << YAML::Key << "Id" << YAML::Value << model.Id;
      }
      yaml << YAML::EndMap;
   }


   template<>
   void Serialize<ModelResourceCache>(YAML::Emitter& yaml, const ModelResourceCache& cache) {
      yaml << YAML::Value << YAML::BeginSeq;
      {
         cache.each([&](Id id, ModelResourceHandle handle) {
            yaml << YAML::Key << "Id" << YAML::Value << id;
            yaml << YAML::Key << "Path" << YAML::Value << handle->Path.c_str();
         });
      }
      yaml << YAML::EndSeq;
   }


   template<typename T>
   void SerializeComponent(YAML::Emitter& yaml, const Scene& scene, const Object object) {
      if (auto component = scene.m_Registry.try_get<T>(object)) {
         Serialize(yaml, *component);
      }
   }


   void SerializeObject(YAML::Emitter& yaml, const Scene& scene, const Object object) {
      // registry::visit is no good here because that gives you opaque type_info only.
      // we need the actual component types - to pass them off to templated serialize functions
      // scene.m_Registry.visit(object, [&](const entt::type_info info) {
      //    auto&& storage = scene.m_Registry.storage(info);  <-- so we have the data here, but only as a typeless blob
      // });
      //
      // So, unfortunately there is no option but a big long list of all possible component types...
      yaml << YAML::BeginMap;
      {
         SerializeComponent<Id>(yaml, scene, object);
         SerializeComponent<Transform>(yaml, scene, object);
         SerializeComponent<Model>(yaml, scene, object);
      }
      yaml << YAML::EndMap;
   }


   SceneSerializerYAML::SceneSerializerYAML(const SerializerSettings& settings)
   : m_Settings { settings }
   {}


   void SceneSerializerYAML::Serialize(const Scene& scene) {
      YAML::Emitter yaml;

      yaml << YAML::BeginDoc;
      {
         yaml << YAML::BeginMap;
         {
            yaml << YAML::Key << "Assets";
            yaml << YAML::Value << YAML::BeginMap;
            {
               yaml << YAML::Key << "Models";
               yaml << YAML::Value; Pikzel::Serialize(yaml, scene.m_ModelCache);
            }
            yaml << YAML::EndMap;
         }
         yaml << YAML::EndMap;
      }
      {
         yaml << YAML::BeginMap;
         {
            yaml << YAML::Key << "Objects";
            yaml << YAML::Value << YAML::BeginSeq;
            {
               for (auto object : scene.m_Registry.view<const Id>()) {
                  SerializeObject(yaml, scene, object);
               }
            }
            yaml << YAML::EndSeq;
         }
         yaml << YAML::EndMap;
      }
      yaml << YAML::EndDoc;

      std::ofstream out(m_Settings.Path.string().c_str());
      out << yaml.c_str();
   }


   std::unique_ptr<Scene> SceneSerializerYAML::Deserialise() {
      std::unique_ptr<Scene> scene = std::make_unique<Scene>();
      try {
         std::ifstream in(m_Settings.Path.string().c_str());
         //         cereal::JSONInputArchive archive(in);
         //         archive(cereal::make_nvp("Scene", *scene));

         auto object = scene->CreateObject();
         scene->AddComponent<Id>(object, 1234ul);


      } catch (const std::exception& err) {
         PKZL_LOG_ERROR("Failed to load scene: {0}", err.what());
         scene = nullptr;
      }
      return scene;
   }

}
