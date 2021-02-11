#version 450 core
layout (location = 0) in vec3 inTexCoords;

layout(push_constant) uniform PC {
   mat4 vp;
   int lod;
   int tonemap;
} constants;

layout(binding = 0) uniform samplerCube uSkybox;

layout (location = 0) out vec4 outFragColor;

void main() {
   outFragColor = textureLod(uSkybox, inTexCoords, constants.lod);

   // Reinhard tonemap
   if(constants.tonemap == 1) {
      outFragColor = vec4(vec3(outFragColor.rgb / (1.0 + outFragColor.rgb)), outFragColor.a);
   }
}
