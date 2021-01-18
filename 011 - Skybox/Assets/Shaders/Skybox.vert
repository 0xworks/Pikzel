#version 450 core
layout (location = 0) in vec3 inPos;


layout(push_constant) uniform PC {
   mat4 vp;
   int lod;
   int tonemap;
} constants;

layout (location = 0) out vec3 outTexCoords;

void main() {
   outTexCoords = inPos;
   vec4 pos = constants.vp * vec4(inPos, 1.0);
   gl_Position.xyw = pos.xyw;
   gl_Position.z = 0.0;
}
