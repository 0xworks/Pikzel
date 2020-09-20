#include "Buffer.h"

namespace Pikzel {

   std::string DataTypeToString(DataType type) {
      switch (type) {
         case DataType::None:     return "None";
         case DataType::Bool:     return "Bool";
         case DataType::Int:      return "Int";
         case DataType::UInt:     return "UInt";
         case DataType::Float:    return "Float";
         case DataType::Double:   return "Double";
         case DataType::BVec2:    return "BVec2";
         case DataType::BVec3:    return "BVec3";
         case DataType::BVec4:    return "BVec4";
         case DataType::IVec2:    return "IVec2";
         case DataType::IVec3:    return "IVec3";
         case DataType::IVec4:    return "IVec4";
         case DataType::UVec2:    return "UVec2";
         case DataType::UVec3:    return "UVec3";
         case DataType::UVec4:    return "UVec4";
         case DataType::Vec2:     return "Vec2";
         case DataType::Vec3:     return "Vec3";
         case DataType::Vec4:     return "Vec4";
         case DataType::DVec2:    return "DVec2";
         case DataType::DVec3:    return "DVec3";
         case DataType::DVec4:    return "DVec4";
         case DataType::Mat2:     return "Mat2";
         case DataType::Mat2x3:   return "Mat2x3";
         case DataType::Mat2x4:   return "Mat2x4";
         case DataType::Mat3x2:   return "Mat3x2";
         case DataType::Mat3:     return "Mat3";
         case DataType::Mat3x4:   return "Mat3x4";
         case DataType::Mat4x2:   return "Mat4x2";
         case DataType::Mat4x3:   return "Mat4x3";
         case DataType::Mat4:     return "Mat4";
         case DataType::DMat2:    return "DMat2";
         case DataType::DMat2x3:  return "DMat2x3";
         case DataType::DMat2x4:  return "DMat2x4";
         case DataType::DMat3x2:  return "DMat3x2";
         case DataType::DMat3:    return "DMat3";
         case DataType::DMat3x4:  return "DMat3x4";
         case DataType::DMat4x2:  return "DMat4x2";
         case DataType::DMat4x3:  return "DMat4x3";
         case DataType::DMat4:    return "DMat4";
      }
      PKZL_CORE_ASSERT(false, "Unknown DataType!");
      return "Unknown";
   }


   uint32_t DataTypeSize(DataType type) {
      switch (type) {
         case DataType::Bool:     return 1;
         case DataType::Int:      return 4;
         case DataType::UInt:     return 4;
         case DataType::Float:    return 4;
         case DataType::Double:   return 8;
         case DataType::BVec2:    return 1 * 2;
         case DataType::BVec3:    return 1 * 3;
         case DataType::BVec4:    return 1 * 4;
         case DataType::IVec2:    return 4 * 2;
         case DataType::IVec3:    return 4 * 3;
         case DataType::IVec4:    return 4 * 4;
         case DataType::UVec2:    return 4 * 2;
         case DataType::UVec3:    return 4 * 3;
         case DataType::UVec4:    return 4 * 4;
         case DataType::Vec2:     return 4 * 2;
         case DataType::Vec3:     return 4 * 3;
         case DataType::Vec4:     return 4 * 4;
         case DataType::DVec2:    return 8 * 2;
         case DataType::DVec3:    return 8 * 3;
         case DataType::DVec4:    return 8 * 4;
         case DataType::Mat2:     return 4 * 2 * 2;
         case DataType::Mat2x3:   return 4 * 2 * 3;
         case DataType::Mat2x4:   return 4 * 2 * 4;
         case DataType::Mat3x2:   return 4 * 3 * 2;
         case DataType::Mat3:     return 4 * 3 * 3;
         case DataType::Mat3x4:   return 4 * 3 * 4;
         case DataType::Mat4x2:   return 4 * 4 * 2;
         case DataType::Mat4x3:   return 4 * 4 * 3;
         case DataType::Mat4:     return 4 * 4 * 4;
         case DataType::DMat2:    return 8 * 2 * 2;
         case DataType::DMat2x3:  return 8 * 2 * 3;
         case DataType::DMat2x4:  return 8 * 2 * 4;
         case DataType::DMat3x2:  return 8 * 3 * 2;
         case DataType::DMat3:    return 8 * 3 * 3;
         case DataType::DMat3x4:  return 8 * 3 * 4;
         case DataType::DMat4x2:  return 8 * 4 * 2;
         case DataType::DMat4x3:  return 8 * 4 * 3;
         case DataType::DMat4:    return 8 * 4 * 4;
      }
      PKZL_CORE_ASSERT(false, "Unknown DataType!");
      return 0;
   }


   BufferElement::BufferElement(const std::string& name, DataType type)
   : Name {name}
   , Type {type}
   , Size {DataTypeSize(type)}
   {}


   uint32_t BufferElement::GetComponentCount() const {
      switch (Type) {
         case DataType::Bool:     return 1;
         case DataType::Int:      return 1;
         case DataType::UInt:     return 1;
         case DataType::Float:    return 1;
         case DataType::Double:   return 1;
         case DataType::BVec2:    return 2;
         case DataType::BVec3:    return 3;
         case DataType::BVec4:    return 4;
         case DataType::IVec2:    return 2;
         case DataType::IVec3:    return 3;
         case DataType::IVec4:    return 4;
         case DataType::UVec2:    return 2;
         case DataType::UVec3:    return 3;
         case DataType::UVec4:    return 4;
         case DataType::Vec2:     return 2;
         case DataType::Vec3:     return 3;
         case DataType::Vec4:     return 4;
         case DataType::DVec2:    return 2;
         case DataType::DVec3:    return 3;
         case DataType::DVec4:    return 4;
         case DataType::Mat2:     return 2; // 2 * vec2,
         case DataType::Mat2x3:   return 2; // 2 * vec3, etc.
         case DataType::Mat2x4:   return 2;
         case DataType::Mat3x2:   return 3;
         case DataType::Mat3:     return 3;
         case DataType::Mat3x4:   return 3;
         case DataType::Mat4x2:   return 4;
         case DataType::Mat4x3:   return 4;
         case DataType::Mat4:     return 4;
         case DataType::DMat2:    return 2;
         case DataType::DMat2x3:  return 2;
         case DataType::DMat2x4:  return 2;
         case DataType::DMat3x2:  return 3;
         case DataType::DMat3:    return 3;
         case DataType::DMat3x4:  return 3;
         case DataType::DMat4x2:  return 4;
         case DataType::DMat4x3:  return 4;
         case DataType::DMat4:    return 4;
      }
      PKZL_CORE_ASSERT(false, "Unknown DataType!");
      return 0;
   }


   BufferLayout::BufferLayout(const std::initializer_list<BufferElement>& elements) : m_Elements(elements) {
      CalculateOffsetsAndStride();
   }


   uint32_t BufferLayout::GetStride() const {
      return m_Stride;
   }


   const std::vector<Pikzel::BufferElement>& BufferLayout::GetElements() const {
      return m_Elements;
   }


   std::vector<BufferElement>::iterator BufferLayout::begin() {
      return m_Elements.begin();
   }


   std::vector<Pikzel::BufferElement>::const_iterator BufferLayout::begin() const {
      return m_Elements.begin();
   }


   std::vector<BufferElement>::iterator BufferLayout::end() {
      return m_Elements.end();
   }


   std::vector<Pikzel::BufferElement>::const_iterator BufferLayout::end() const {
      return m_Elements.end();
   }


   void BufferLayout::CalculateOffsetsAndStride() {
      size_t offset = 0;
      m_Stride = 0;
      for (auto& element : m_Elements) {
         element.Offset = offset;
         offset += element.Size;
         m_Stride += element.Size;
      }
   }

}
