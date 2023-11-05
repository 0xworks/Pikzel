#version 450 core
layout (location = 0) in vec3 inTexCoords;

layout(push_constant) uniform PC {
   mat4 vp;
   int lod;
   float intensity;
} constants;

layout(binding = 0) uniform samplerCube uSkybox;

layout (location = 0) out vec4 outFragColor;

void main() {
   outFragColor = textureLod(uSkybox, inTexCoords, constants.lod) * constants.intensity;
}
