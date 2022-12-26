#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>

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
