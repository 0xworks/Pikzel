#include "pch.h"
#include "Buffer.h"

namespace Pikzel {

   uint32_t DataTypeSize(DataType type) {
      switch (type) {
         case DataType::Float:    return 4;
         case DataType::Float2:   return 4 * 2;
         case DataType::Float3:   return 4 * 3;
         case DataType::Float4:   return 4 * 4;
         case DataType::Mat3:     return 4 * 3 * 3;
         case DataType::Mat4:     return 4 * 4 * 4;
         case DataType::Int:      return 4;
         case DataType::Int2:     return 4 * 2;
         case DataType::Int3:     return 4 * 3;
         case DataType::Int4:     return 4 * 4;
         case DataType::Bool:     return 1;
      }

      PKZL_CORE_ASSERT(false, "Unknown DataType!");
      return 0;
   }


   BufferElement::BufferElement(DataType type, const std::string& name, bool normalized /*= false*/) : Name(name)
      , Type(type)
      , Size(DataTypeSize(type))
      , Offset(0)
      , Normalized(normalized) {

   }


   uint32_t BufferElement::GetComponentCount() const {
      switch (Type) {
         case DataType::Float:   return 1;
         case DataType::Float2:  return 2;
         case DataType::Float3:  return 3;
         case DataType::Float4:  return 4;
         case DataType::Mat3:    return 3; // 3* float3
         case DataType::Mat4:    return 4; // 4* float4
         case DataType::Int:     return 1;
         case DataType::Int2:    return 2;
         case DataType::Int3:    return 3;
         case DataType::Int4:    return 4;
         case DataType::Bool:    return 1;
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
