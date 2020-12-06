#version 450 core
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoords;

struct Matrices {
   mat4 vp;
   mat4 lightSpace;
   vec3 viewPos;
};

layout(set = 0, binding = 0) uniform UBOMatrices {
   Matrices matrices;
} uboMatrices;

layout(push_constant) uniform PC {
   mat4 model;
   mat4 modelInvTrans;
} constants;

layout(location = 0) out vec4 outNormal;
layout(location = 1) out vec4 outFragPos;
layout(location = 2) out vec4 outFragPosLightSpace;
layout(location = 3) out vec2 outTexCoords;

void main() {
   outNormal = constants.modelInvTrans * vec4(inNormal, 0.0);
   outFragPos = constants.model * vec4(inPos, 1.0);
   outFragPosLightSpace = uboMatrices.matrices.lightSpace * outFragPos;
   outTexCoords = inTexCoords;
   gl_Position = uboMatrices.matrices.vp * outFragPos;
}
