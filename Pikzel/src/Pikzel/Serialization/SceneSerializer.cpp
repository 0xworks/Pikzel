#include "SceneSerializer.h"

#include "Serializer.h"
#include "Yaml.h"

#include "Pikzel/Components/Model.h"
#include "Pikzel/Components/Relationship.h"
#include "Pikzel/Components/Transform.h"
#include "Pikzel/Core/Utility.h"
#include "Pikzel/Scene/AssetCache.h"

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <format>
#include <fstream>
#include <memory>
#include <string>
#include <string_view>

namespace Pikzel {

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
      yaml << YAML::Value << YAML::BeginMap;
      {
         yaml << YAML::Key << "Translation" << YAML::Value << YAML::Node{transform.Translation};
         yaml << YAML::Key << "Rotation" << YAML::Value << YAML::Node{transform.RotationEuler};
         yaml << YAML::Key << "Scale" << YAML::Value << YAML::Node{transform.Scale};
      }
      yaml << YAML::EndMap;
   }


   template<>
   void Deserialize<Transform>(YAML::Node node, Transform& transform) {
      if (node.IsMap()) {
         transform.Translation = node["Translation"].as<glm::vec3>();
         transform.RotationEuler = node["Rotation"].as<glm::vec3>();
         transform.Rotation = glm::quat(transform.RotationEuler);
         transform.Scale = node["Scale"].as<glm::vec3>();
      }
   }


   template<>
   void Serialize<Model>(YAML::Emitter& yaml, const Model& model) {
      auto path = AssetCache::GetPath(model.id);
      yaml << YAML::Key << "Path" << YAML::Value << (path ? path->string() : "<unknown>");
   }


   template<>
   void Deserialize<Model>(YAML::Node node, Model& model) {
      if (node.IsMap()) {
         auto path = node["Path"].as<std::string>();
         if (path != "<unknown>") {
            model.id = entt::hashed_string(path.data());
            if (!AssetCache::GetModelAsset(model.id)) {
               AssetCache::LoadModelAsset(path);
            }
         }
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


   template<>
   void DeserializeComponent<Transform>(YAML::Node node, const std::string_view key, Scene& scene, Object object) {
      if (auto componentNode = node[key.data()]) {
         auto& transform = scene.AddComponent<Transform>(object);
         Deserialize(componentNode, transform);
         scene.AddComponent<glm::mat4>(object) = glm::translate(glm::identity<glm::mat4>(), transform.Translation) * glm::toMat4(transform.Rotation) * glm::scale(glm::identity<glm::mat4>(), transform.Scale);
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


   SceneSerializerYAML::SceneSerializerYAML(const SceneSerializerSettings& settings)
   : m_Settings{settings}
   {}


   void SceneSerializerYAML::Serialize(const Scene& scene) {
      std::ofstream out{m_Settings.Path};

      YAML::Emitter yaml{out};

      yaml << YAML::BeginMap;
      {
         yaml << YAML::Key << "Scene";
         yaml << YAML::Value << YAML::BeginMap;
         {
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
         std::unique_ptr<std::istream> in = IStream(m_Settings.Path);
         YAML::Node yaml = YAML::Load(*in);
         if (auto sceneNode = yaml["Scene"]) {
            PKZL_CORE_LOG_INFO("Deserializing scene from path '{}'", m_Settings.Path.string());
            DeserializeObjects(sceneNode["Objects"], *scene);
         } else {
            throw std::runtime_error{std::format("No scene found in stream from path '{}'", m_Settings.Path) };
         }

      } catch (const std::exception& err) {
         PKZL_LOG_ERROR("Failed to load scene: {0}", err.what());
         scene = nullptr;
      }

      return scene;
   }

}
