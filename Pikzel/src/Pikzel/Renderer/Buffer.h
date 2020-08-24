#pragma once

namespace Pikzel {

   class Buffer {
   public:
      virtual ~Buffer() = default;

      virtual void CopyFromHost(const uint64_t offset, const uint64_t size, const void* pData) = 0;

   };

}
