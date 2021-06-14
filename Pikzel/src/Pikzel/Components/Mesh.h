#pragma once

#include "Pikzel/Renderer/Buffer.h"

namespace Pikzel {

   struct PKZL_API Mesh {

//      struct Vertex {
//         Vertex(glm::vec3 pos, glm::vec3 normal, glm::vec3 tangent, glm::vec2 uv) : Pos{pos}, Normal{normal}, Tangent{tangent}, UV{uv} {}
//         glm::vec3 Pos;
//         glm::vec3 Normal;
//         glm::vec3 Tangent;
//         glm::vec2 UV;
//     };

      std::unique_ptr<VertexBuffer> VertexBuffer;
      std::unique_ptr<IndexBuffer> IndexBuffer;
   };

}
