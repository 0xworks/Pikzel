#include "SceneSerializer.h"

#include "Pikzel/Components/Model.h"
#include "Pikzel/Components/Relationship.h"
#include "Pikzel/Components/Transform.h"
#include "Pikzel/Scene/AssetCache.h"

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <yaml-cpp/yaml.h>
#include <fstream>

namespace YAML {

   template<>
   struct convert<glm::vec2> {

      static Node encode(const glm::vec2& vec) {
         Node node;
         node.SetStyle(EmitterStyle::Flow);
         node.push_back(vec.x);
         node.push_back(vec.y);
         return node;
      }

      static bool decode(const Node& node, glm::vec2& vec) {
         if (!node.IsSequence() || node.size() != 2) {
            return false;
         }
         vec.x = node[0].as<float>();
         vec.y = node[1].as<float>();
         return true;
      }
   };


   template<>
   struct convert<glm::vec3> {

      static Node encode(const glm::vec3& vec) {
         Node node;
         node.SetStyle(EmitterStyle::Flow);
         node.push_back(vec.x);
         node.push_back(vec.y);
         node.push_back(vec.z);
         return node;
      }

      static bool decode(const Node& node, glm::vec3& vec) {
         if (!node.IsSequence() || node.size() != 3) {
            return false;
         }
         vec.x = node[0].as<float>();
         vec.y = node[1].as<float>();
         vec.z = node[2].as<float>();
         return true;
      }
   };


   template<>
   struct convert<glm::vec4> {

      static Node encode(const glm::vec4& vec) {
         Node node;
         node.SetStyle(EmitterStyle::Flow);
         node.push_back(vec.x);
         node.push_back(vec.y);
         node.push_back(vec.z);
         node.push_back(vec.w);
         return node;
      }

      static bool decode(const Node& node, glm::vec4& vec) {
         if (!node.IsSequence() || node.size() != 4) {
            return false;
         }
         vec.x = node[0].as<float>();
         vec.y = node[1].as<float>();
         vec.z = node[2].as<float>();
         vec.w = node[3].as<float>();
         return true;
      }
   };


   template<>
   struct convert<glm::quat> {

      static Node encode(const glm::quat& quat) {
         Node node;
         node.SetStyle(EmitterStyle::Flow);
         node.push_back(quat.x);
         node.push_back(quat.y);
         node.push_back(quat.z);
         node.push_back(quat.w);
         return node;
      }

      static bool decode(const Node& node, glm::quat& quat) {
         if (!node.IsSequence() || node.size() != 4) {
            return false;
         }
         quat.x = node[0].as<float>();
         quat.y = node[1].as<float>();
         quat.z = node[2].as<float>();
         quat.w = node[3].as<float>();
         return true;
      }
   };

}


namespace Pikzel {

   template<typename T>
   void Serialize(YAML::Emitter& yaml, const T& component); // not defined on purpose.  you must specialize


   template<typename T>
   void Deserialize(YAML::Node node, T& component); // not defined on purpose.  you must specialize


   template<>
   void Serialize<Id>(YAML::Emitter& yaml, const Id& id) {
      yaml << id;
   }


   template<>
   void Deserialize<Id>(YAML::Node node, Id& id) {
      id = node.as<Pikzel::Id>();
   }


   template<>
   void Serialize<std::string>(YAML::Emitter& yaml, const std::string& name) {
      yaml << name;
   }


   template<>
   void Deserialize<std::string>(YAML::Node node, std::string& name) {
      name = node.as<std::string>();
   }


   template<>
   void Serialize<Transform>(YAML::Emitter& yaml, const Transform& transform) {
      glm::vec3 position;
      glm::quat rotation;
      glm::vec3 scale;
      glm::vec3 skew;
      glm::vec4 perspective;
      glm::decompose(transform.Matrix, scale, rotation, position, skew, perspective);
      rotation = glm::conjugate(rotation);

      yaml << YAML::Value << YAML::BeginMap;
      {
         yaml << YAML::Key << "Position" << YAML::Value << YAML::Node{ position };
         yaml << YAML::Key << "Rotation" << YAML::Value << YAML::Node{ glm::eulerAngles(rotation) };
         yaml << YAML::Key << "Scale" << YAML::Value << YAML::Node{ scale };
      }
      yaml << YAML::EndMap;
   }


   template<>
   void Deserialize<Transform>(YAML::Node node, Transform& transform) {
      if (node.IsMap()) {
         auto position = node["Position"].as<glm::vec3>();
         auto rotation = node["Rotation"].as<glm::vec3>();
         auto scale = node["Scale"].as<glm::vec3>();
         transform.Matrix = glm::translate(glm::identity<glm::mat4>(), position) * glm::toMat4(glm::quat(rotation)) * glm::scale(glm::identity<glm::mat4>(), scale);
      }
   }


   template<>
   void Serialize<Model>(YAML::Emitter& yaml, const Model& model) {
      auto handle = AssetCache::GetModelResource(model.Id);
      yaml << (handle? handle->Name : "<unknown>");
   }


   template<>
   void Deserialize<Model>(YAML::Node node, Model& model) {
      model.Id = entt::hashed_string{ node.as<std::string>().data() }.value();
   }


   template<>
   void Serialize<ConstModelResourceHandle>(YAML::Emitter& yaml, const ConstModelResourceHandle& handle) {
      yaml << YAML::BeginMap;
      {
         yaml << YAML::Key << "Name" << YAML::Value << handle->Name;
         yaml << YAML::Key << "Path" << YAML::Value << handle->Path.string().c_str();
      }
      yaml << YAML::EndMap;
   }

   template<>
   void Serialize<ModelResourceCache>(YAML::Emitter& yaml, const ModelResourceCache& cache) {
      yaml << YAML::Value << YAML::BeginSeq;
      {
         cache.each([&](ConstModelResourceHandle handle) {
            Serialize(yaml, handle);
         });
      }
      yaml << YAML::EndSeq;
   }


   template<>
   void Deserialize<ModelResourceCache>(YAML::Node node, ModelResourceCache&) {
      for (auto modelResourceNode : node) {
         auto name = modelResourceNode["Name"].as<std::string>();
         auto path = modelResourceNode["Path"].as<std::string>();
         Pikzel::AssetCache::LoadModelResource(name, path);
      }
   }


   template<typename T>
   void SerializeComponent(YAML::Emitter& yaml, const std::string_view key, const Scene& scene, const Object object) {
      if (auto component = scene.TryGetComponent<T>(object)) {
         yaml << YAML::Key << key.data() << YAML::Value; Serialize(yaml, *component);
      }
   }


   template<typename T>
   void DeserializeComponent(YAML::Node node, const std::string_view key, Scene& scene, Object object) {
      if (auto componentNode = node[key.data()]) {
         auto& component = scene.AddComponent<T>(object);
         Deserialize(componentNode, component);
      }
   }


   void SerializeObject(YAML::Emitter& yaml, const Scene& scene, const Object object, const Relationship& relationship) {
      // registry::visit is no good here because that gives you opaque type_info only.
      // we need the actual component types - to pass them off to templated serialize functions
      // scene.m_Registry.visit(object, [&](const entt::type_info info) {
      //    auto&& storage = scene.m_Registry.storage(info);  <-- so we have the data here, but only as a typeless blob
      // });
      //
      // So, unfortunately there is no option but a big long list of all possible component types...
      yaml << YAML::BeginMap;
      {
         SerializeComponent<Id>(yaml, "Id", scene, object);
         SerializeComponent<std::string>(yaml, "Name", scene, object);
         SerializeComponent<Transform>(yaml, "Transform", scene, object);
         SerializeComponent<Model>(yaml, "Model", scene, object);
         if (Object childObject = relationship.FirstChild; childObject != Null) {
            yaml << YAML::Key << "Objects" << YAML::Value;
            yaml << YAML::BeginSeq;
            {
               while (childObject != Null) {
                  auto childRelationship = scene.GetComponent<Relationship>(childObject);
                  SerializeObject(yaml, scene, childObject, childRelationship);
                  childObject = childRelationship.NextSibling;
               }
            }
            yaml << YAML::EndSeq;
         }
      }
      yaml << YAML::EndMap;
   }


   Object DeserializeObject(YAML::Node objectNode, Scene& scene, Object parent) {
      Object object = Null;
      if (objectNode.IsMap()) {
         object = scene.CreateEmptyObject();
         DeserializeComponent<Id>(objectNode, "Id", scene, object);
         DeserializeComponent<std::string>(objectNode, "Name", scene, object);
         DeserializeComponent<Transform>(objectNode, "Transform", scene, object);
         DeserializeComponent<Model>(objectNode, "Model", scene, object);
         auto& relationship = scene.AddComponent<Relationship>(object);
         relationship.Parent = parent;
         Object prevChildObject = Null;
         YAML::Node childObjectsNode = objectNode["Objects"];
         for (auto childObjectNode : childObjectsNode) {
            Object childObject = DeserializeObject(childObjectNode, scene, object);
            if (prevChildObject != Null) {
               scene.GetComponent<Relationship>(prevChildObject).NextSibling = childObject;
            }
            prevChildObject = childObject;
         }
      }
      return object;
   }


   void SerializeObjects(YAML::Emitter& yaml, const Scene& scene) {
      yaml << YAML::BeginSeq;
      {
         Object object = scene.GetView<Relationship>().front();
         while (object != Null) {
            auto relationship = scene.GetComponent<Relationship>(object);
            SerializeObject(yaml, scene, object, relationship);
            object = relationship.NextSibling;
         }
      }
      yaml << YAML::EndSeq;
   }


   void DeserializeObjects(YAML::Node objectsNode, Scene& scene) {
      Object prevObject = Null;
      for (auto objectNode : objectsNode) {
         Object object = DeserializeObject(objectNode, scene, Null);
         if (prevObject != Null) {
            scene.GetComponent<Relationship>(prevObject).NextSibling = object;
         }
         if (object != Null) {
            prevObject = object;
         }
      }
      scene.SortObjects();
   }


   SceneSerializerYAML::SceneSerializerYAML(const SerializerSettings& settings)
   : m_Settings { settings }
   {}


   void SceneSerializerYAML::Serialize(const Scene& scene) {
      std::ofstream out{ m_Settings.Path };
   
      YAML::Emitter yaml{ out };

      yaml << YAML::BeginMap;
      {
         yaml << YAML::Key << "Scene";
         yaml << YAML::Value << YAML::BeginMap;
         {
            yaml << YAML::Key << "Assets";
            yaml << YAML::Value << YAML::BeginMap;
            {
               yaml << YAML::Key << "Models";
               yaml << YAML::Value; Pikzel::Serialize(yaml, AssetCache::m_ModelCache);
            }
            yaml << YAML::EndMap;

            yaml << YAML::Key << "Objects";
            yaml << YAML::Value; SerializeObjects(yaml, scene);
         }
         yaml << YAML::EndMap;
      }
      yaml << YAML::EndMap;
   }


   std::unique_ptr<Scene> SceneSerializerYAML::Deserialize() {
      std::unique_ptr<Scene> scene = std::make_unique<Scene>();
      try {
         YAML::Node node = YAML::LoadFile(m_Settings.Path.string().c_str());
         if (auto sceneNode = node["Scene"]) {
            PKZL_CORE_LOG_INFO("Deserializing scene from path '{0}'", m_Settings.Path);

            if (auto assetsNode = sceneNode["Assets"]) {
               Pikzel::Deserialize(assetsNode["Models"], AssetCache::m_ModelCache);
            }

            DeserializeObjects(sceneNode["Objects"], *scene);

         } else {
            throw std::runtime_error{fmt::format("No scene found in stream from path '{}'", m_Settings.Path) };
         }

      } catch (const std::exception& err) {
         PKZL_LOG_ERROR("Failed to load scene: {0}", err.what());
         scene = nullptr;
      }

      return scene;
   }

}
