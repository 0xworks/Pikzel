#pragma once

namespace Pikzel {

   enum class DataType {
      None,
      Float,
      Float2,
      Float3,
      Float4,
      Mat3,
      Mat4,
      Int,
      Int2,
      Int3,
      Int4,
      Bool
   };


   static uint32_t DataTypeSize(DataType type);


   struct BufferElement {
      std::string Name;
      DataType Type;
      uint32_t Size;
      size_t Offset;
      bool Normalized;

      BufferElement(DataType type, const std::string& name, bool normalized = false);

      uint32_t GetComponentCount() const;
   };


   class BufferLayout {
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


   class Buffer {
   public:
      virtual ~Buffer() = default;

      virtual void Bind() const = 0;
      virtual void Unbind() const = 0;

      virtual void CopyFromHost(const uint64_t offset, const uint64_t size, const void* pData) = 0;
   };


   class VertexBuffer : public Buffer {
   public:
      virtual ~VertexBuffer() = default;

      virtual const BufferLayout& GetLayout() const = 0;
      virtual void SetLayout(const BufferLayout& layout) = 0;
   };


   class IndexBuffer : public Buffer {
   public:
      virtual ~IndexBuffer() = default;

      virtual uint32_t GetCount() const = 0;
   };

}
