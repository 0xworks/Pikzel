#version 450 core
#extension GL_GOOGLE_include_directive: require

#include "Lights.glsl"
#include "Matrices.glsl"

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inTexCoords;

layout(push_constant) uniform PC {
   mat4 model;
   vec2 textureRepeat;
   float heightScale;
   float lightRadius;
   uint numPointLights;
} constants;

layout(set = 0, binding = 0) uniform UBOMatrices {
   Matrices matrices;
} uboMatrices;

layout(set = 1, binding = 0) uniform UBODirectionalLight {
   DirectionalLight light;
} directionalLight;


layout(location = 0) out vec3 outFragPos;
layout(location = 1) out vec4 outFragPosLightSpace;
layout(location = 2) out vec2 outTexCoords;
layout(location = 3) out mat3 outTangentBasis;

void main() {
   outFragPos = vec3(constants.model * vec4(inPos, 1.0));
   outFragPosLightSpace = uboMatrices.matrices.lightSpace * vec4(outFragPos, 1.0);
   outTexCoords = inTexCoords * constants.textureRepeat;

   vec3 T = normalize(vec3(constants.model * vec4(inTangent,   0.0)));
   const vec3 N = normalize(vec3(constants.model * vec4(inNormal, 0.0)));
   T = normalize(T - dot(T, N) * N);
   const vec3 B = cross(N, T);
   outTangentBasis = mat3(T, B, N);

   gl_Position = uboMatrices.matrices.viewProjection * vec4(outFragPos, 1.0);
}
