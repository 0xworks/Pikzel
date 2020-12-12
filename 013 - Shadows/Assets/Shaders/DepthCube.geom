#version 450 core
#extension GL_GOOGLE_include_directive: require

#include "PointLight.glsl"

layout (triangles) in;
layout (triangle_strip, max_vertices = MAX_POINT_LIGHTS * 18) out;

layout(push_constant) uniform PC {
   mat4 model;
   float farPlane;
   uint numPointLights;
} constants;

layout(set = 0, binding = 0) uniform UBOLightViews {
   mat4 view[MAX_POINT_LIGHTS * 6];
} lightViews;

layout (location = 0) out vec4 outFragPos;

void main() {
   for(int light = 0; light < constants.numPointLights; ++light) {
      for(int face = 0; face < 6; ++face) {
         gl_Layer = light * 6 + face;
         for(int i = 0; i < 3; ++i) {
            outFragPos = gl_in[i].gl_Position;
            gl_Position = lightViews.view[light * 6 + face] * outFragPos;
            EmitVertex();
         }
         EndPrimitive();
      }
   }
}
