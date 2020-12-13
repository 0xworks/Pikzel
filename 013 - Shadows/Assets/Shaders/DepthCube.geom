#version 450 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

layout(push_constant) uniform PC {
   mat4 model;
   float farPlane;
   int lightIndex;
} constants;

layout(set = 0, binding = 0) uniform UBOLightViews {
   mat4 view[6];
} lightViews;

layout (location = 0) out vec4 outFragPos;

void EmitFace(const mat4 lightView) {
   for(int i = 0; i < 3; ++i) {
      outFragPos = gl_in[i].gl_Position;
      gl_Position = lightView * outFragPos;
      EmitVertex();
   }
   EndPrimitive();
}

void main() {
   // some devices require that gl_Layer is assigned to a constant
   gl_Layer = (constants.lightIndex * 6) + 0;
   EmitFace(lightViews.view[0]);

   gl_Layer = (constants.lightIndex * 6) + 1;
   EmitFace(lightViews.view[1]);

   gl_Layer = (constants.lightIndex * 6) + 2;
   EmitFace(lightViews.view[2]);

   gl_Layer = (constants.lightIndex * 6) + 3;
   EmitFace(lightViews.view[3]);

   gl_Layer = (constants.lightIndex * 6) + 4;
   EmitFace(lightViews.view[4]);

   gl_Layer = (constants.lightIndex * 6) + 5;
   EmitFace(lightViews.view[5]);
}
