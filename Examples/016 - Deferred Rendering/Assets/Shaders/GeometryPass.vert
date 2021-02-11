#version 450 core
#extension GL_GOOGLE_include_directive: require

#include "Matrices.glsl"

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inTexCoords;

layout(push_constant) uniform PC {
   mat4 model;
} constants;

layout(set = 0, binding = 0) uniform UBOMatrices {
   Matrices matrices;
} uboMatrices;

layout(location = 0) out vec3 outFragPos;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec2 outTexCoords;


void main() {
   const vec4 worldPos = constants.model * vec4(inPos, 1.0);
   outFragPos = worldPos.xyz;
   outNormal = vec3(constants.model * vec4(inNormal, 0.0));
   outTexCoords = inTexCoords;

   gl_Position = uboMatrices.matrices.viewProjection * worldPos;
}
