#pragma once

#include "Pikzel/Core/Core.h"

namespace Pikzel {

   enum class DataType {
      None,
      Bool,
      Int,
      UInt,
      Float,
      Double,
      BVec2,
      BVec3,
      BVec4,
      IVec2,
      IVec3,
      IVec4,
      UVec2,
      UVec3,
      UVec4,
      Vec2,
      Vec3,
      Vec4,
      DVec2,
      DVec3,
      DVec4,
      Mat2,
      Mat2x3,
      Mat2x4,
      Mat3x2,
      Mat3,
      Mat3x4,
      Mat4x2,
      Mat4x3,
      Mat4,
      DMat2,
      DMat2x3,
      DMat2x4,
      DMat3x2,
      DMat3,
      DMat3x4,
      DMat4x2,
      DMat4x3,
      DMat4
   };


   std::string PKZL_API DataTypeToString(DataType type);
   uint32_t PKZL_API DataTypeSize(DataType type);


   struct PKZL_API BufferElement {
      std::string Name;
      DataType Type;
      uint32_t Size;
      size_t Offset;
      bool Normalized;

      BufferElement(const std::string& name, DataType type);

      uint32_t GetComponentCount() const;
   };


   class PKZL_API BufferLayout {
   public:
      BufferLayout() = default;
      BufferLayout(const std::initializer_list<BufferElement>& elements);

      uint32_t GetStride() const;
      const std::vector<BufferElement>& GetElements() const;

      std::vector<BufferElement>::iterator begin();
      std::vector<BufferElement>::iterator end();
      std::vector<BufferElement>::const_iterator begin() const;
      std::vector<BufferElement>::const_iterator end() const;

   private:
      void CalculateOffsetsAndStride();

   private:
      std::vector<BufferElement> m_Elements;
      uint32_t m_Stride = 0;
   };


   class PKZL_API Buffer {
   public:
      virtual ~Buffer() = default;

      virtual void CopyFromHost(const uint64_t offset, const uint64_t size, const void* pData) = 0;

   };


   class PKZL_API VertexBuffer : public Buffer {
   public:
      virtual ~VertexBuffer() = default;

      virtual const BufferLayout& GetLayout() const = 0;
      virtual void SetLayout(const BufferLayout& layout) = 0;
   };


   class PKZL_API IndexBuffer : public Buffer {
   public:
      virtual ~IndexBuffer() = default;

      virtual uint32_t GetCount() const = 0;
   };


   class PKZL_API UniformBuffer : public Buffer {
   public:
      virtual ~UniformBuffer() = default;
   };

}
