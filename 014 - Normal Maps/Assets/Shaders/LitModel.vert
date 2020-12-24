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
   float lightRadius;
   uint numPointLights;
   uint showDirectionalLight;
   uint showPointLights;
   uint useNormalMaps;
   uint useDisplacementMaps;
} constants;

layout(set = 0, binding = 0) uniform UBOMatrices {
   Matrices matrices;
} uboMatrices;

layout(set = 1, binding = 0) uniform UBODirectionalLight {
   DirectionalLight light;
} directionalLight;

layout(set = 1, binding = 1) uniform UBOPointLights {
   PointLight light[MAX_POINT_LIGHTS];
} pointLights;

layout(location = 0) out vec3 outFragPos;
layout(location = 1) out vec4 outFragPosLightSpace;
layout(location = 2) out vec2 outTexCoords;
layout(location = 3) out vec3 outTangentViewPos;
layout(location = 4) out vec3 outTangentFragPos;
layout(location = 5) out vec3 outTangentLightDir;
layout(location = 6) out vec3 outTangentLightPos[MAX_POINT_LIGHTS];

//layout(location = 3) out mat3 outTBN;


void main() {
   outFragPos = vec3(constants.model * vec4(inPos, 1.0));
   outFragPosLightSpace = uboMatrices.matrices.lightSpace * vec4(outFragPos, 1.0);
   outTexCoords = inTexCoords;

   vec3 T = normalize(vec3(constants.model * vec4(inTangent,   0.0)));
   vec3 B = normalize(vec3(constants.model * vec4(cross(inNormal, inTangent), 0.0)));
   vec3 N = normalize(vec3(constants.model * vec4(inNormal, 0.0)));
   const mat3 TBN = transpose(mat3(T, B, N));

   outTangentViewPos = TBN * uboMatrices.matrices.eyePosition;
   outTangentFragPos = TBN * outFragPos;
   outTangentLightDir = TBN * directionalLight.light.direction;
   for(uint i = 0; i < constants.numPointLights; ++i) {
      outTangentLightPos[i] = TBN * pointLights.light[i].position;
   }

   gl_Position = uboMatrices.matrices.viewProjection * vec4(outFragPos, 1.0);
}


