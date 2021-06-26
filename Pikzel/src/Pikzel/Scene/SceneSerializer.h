#pragma once

#include "Pikzel/Components/Mesh.h"
#include "Pikzel/Components/ObjectId.h"
#include "Pikzel/Components/Transform.h"
#include "Pikzel/Scene/Scene.h"

#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>
#include <entt/entity/snapshot.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <filesystem>
#include <utility>

namespace glm {

   template<class Archive>
   void serialize(Archive& archive, vec2 vec) {
      archive(
         cereal::make_nvp("x", vec.x),
         cereal::make_nvp("y", vec.y)
      );
   }


   template<class Archive>
   void serialize(Archive& archive, vec3& vec) {
      archive(
         cereal::make_nvp("x", vec.x),
         cereal::make_nvp("y", vec.y),
         cereal::make_nvp("z", vec.z)
      );
   }


   template<class Archive>
   void serialize(Archive& archive, vec4& vec) {
      archive(
         cereal::make_nvp("x", vec.x),
         cereal::make_nvp("y", vec.y),
         cereal::make_nvp("z", vec.z),
         cereal::make_nvp("w", vec.w)
      );
   }


   template<class Archive>
   void serialize(Archive& archive, quat& quat) {
      archive(
         cereal::make_nvp("x", quat.x),
         cereal::make_nvp("y", quat.y),
         cereal::make_nvp("z", quat.z),
         cereal::make_nvp("w", quat.w)
      );
   }


    template<class Archive>
    void save(Archive& archive, const mat4& matrix) {
       vec3 position;
       quat rotation;
       vec3 scale;
       vec3 skew;
       vec4 perspective;
       decompose(matrix, scale, rotation, position, skew, perspective);
       archive(
          cereal::make_nvp("Position", position),
          cereal::make_nvp("Rotation", rotation),
          cereal::make_nvp("Scale", scale)
       );
    }


    template<class Archive>
    void load(Archive& archive, mat4& matrix) {
       vec3 position;
       quat rotation;
       vec3 scale;
       archive(
          cereal::make_nvp("Position", position),
          cereal::make_nvp("Rotation", rotation),
          cereal::make_nvp("Scale", scale)
       );
       matrix = translate(identity<mat4>(), position) * toMat4(rotation) * glm::scale(mat4(1.0f), scale);
    }

}


namespace Pikzel {

   template<class Archive>
   void serialize(Archive& archive, ObjectId& objectId) {
      archive(cereal::make_nvp("Id", objectId.Id));
   }


   template<class Archive>
   void serialize(Archive& archive, Transform& transform) {
      archive(cereal::make_nvp("Matrix", transform.Matrix));
   }


   template<class Archive>
   void serialize(Archive& archive, std::pair<std::reference_wrapper<const Scene>, Object> sceneObject) {

      const Scene& scene = sceneObject.first;
      Object object = sceneObject.second;

      // registry::visit is no good here because that gives you opaque type_info only.
      // we need the actual component types - to pass them off to templated serialize functions
      // scene.m_Registry.visit(object, [&](const entt::type_info info) {
      //   auto&& storage = scene.m_Registry.storage(info);  <-- so we have the data here, but only as a typeless blob
      //});
      //
      // So, unfortunately there is no option but a big long switch of all possible component types...
      std::vector<std::string> components;
      if (auto objectId = scene.m_Registry.try_get<ObjectId>(object)) {
         components.emplace_back("ObjectId");
         archive(cereal::defer(cereal::make_nvp("ObjectId", *objectId)));
      }
      if (auto transform = scene.m_Registry.try_get<Transform>(object)) {
         components.emplace_back("Transform");
         archive(cereal::defer(cereal::make_nvp("Transform", *transform)));
      }
      archive(cereal::make_nvp("Components", components));
      archive.serializeDeferments();
   }


   template<class Archive>
   void save(Archive& archive, const Scene& scene) {
      // entt::snapshot()...  is not quite what we want here because that is "component major"
      // what we want for human readability is "entity major".
      // The situation is different when we come to wanting pure performance (such as for the engine runtime). In that case we will
      // be going for "component major" and in a binary format.  Its as fast as you can get, just splat the things straight into the
      // entt pools.
      archive(cereal::make_nvp("NumObjects", scene.m_Registry.size()));
      scene.m_Registry.each([&](Object object) {
         archive(cereal::make_nvp("Object", std::make_pair(std::cref(scene), object)));
      });
   }


   template<class Archive>
   void load(Archive& archive, Scene& scene) {
      entt::snapshot_loader{scene.m_Registry}.component<ObjectId, Transform>(archive);
   }


   struct PKZL_API SerializerSettings {
      std::filesystem::path Path;
   };


   class PKZL_API SceneSerializerJSON final {
   public:
      SceneSerializerJSON(const SerializerSettings& settings);

      void Serialize(const Scene& scene);

      std::unique_ptr<Scene> Deserialise();

   private:
      SerializerSettings m_Settings;
   };

}
