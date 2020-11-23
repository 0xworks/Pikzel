#pragma once

namespace Pikzel {

   // sRGB and sRGBA classes are convenience helpers to convert from non-linear color space that we are
   // used to dealing with to linear for shaders.
   //
   // sRGB can be constructed from non-linear values that are either ints (components in range 0 to 255)
   // or floats (components in range 0.0 to 1.0)
   //
   // Example usage:
   //
   // Either use sRGB directly in the vertex buffer:
   //    struct Vertex {
   //       glm::vec3 Pos;
   //       sRGB Color;       <-- Color member is declared as an sRGB
   //    };
   //
   //    Vertex vertices[] = {
   //       {.Pos{-0.5f, -0.5f, 0.0f}, .Color{128,   0,   0}},
   //       {.Pos{ 0.5f, -0.5f, 0.0f}, .Color{  0, 128,   0}},
   //       {.Pos{ 0.0f,  0.5f, 0.0f}, .Color{  0,   0, 128}}
   //    };
   //
   //    vertexBuffer = RenderCore::CreateVertexBuffer(sizeof(vertices), vertices);
   //
   //
   // or vertices can use glm::vec3, with conversion done via implicit cast:
   //    struct Vertex {
   //       glm::vec3 Pos;
   //       glm::vec3 Color;    <-- Color member is declared as glm::vec3
   //    };
   //
   //    Vertex vertices[] = {
   //       {.Pos{-0.5f, -0.5f, 0.0f}, .Color{sRGB{128,   0,   0}}},  <-- and conversion from sRGB is done like so
   //       {.Pos{ 0.5f, -0.5f, 0.0f}, .Color{sRGB{  0, 128,   0}}},
   //       {.Pos{ 0.0f,  0.5f, 0.0f}, .Color{sRGB{  0,   0, 128}}}
   //    };
   //
   //    vertexBuffer = RenderCore::CreateVertexBuffer(sizeof(vertices), vertices);
   //
   // In the shader, the color attribute is just a vec3 (or vec4) as usual.
   //
   // If you do neither of the above, then it is up to you to make sure your shader code
   // deals with input color data appropriately.

   class PKZL_API sRGB {
   public:
      sRGB(const float r, const float g, const float b);
      sRGB(const int r, const int g, const int b);

      operator glm::vec3() const;
      operator glm::vec4() const;

   private:
      float red;
      float green;
      float blue;
   };


   class PKZL_API sRGBA {
   public:
      sRGBA(const float r, const float g, const float b, const float a);
      sRGBA(const int r, const int g, const int b, const int a);

      operator glm::vec3() const;
      operator glm::vec4() const;

   private:
      float red;
      float green;
      float blue;
      float alpha;
   };

}
