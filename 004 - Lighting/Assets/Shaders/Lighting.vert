#version 450 core
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;

layout(push_constant) uniform PC {
   mat4 vp;
   mat4 model;
   mat4 modelInvTrans;
   vec3 lightColor;
   vec3 lightPos;
   vec3 objectColor;
   vec3 viewPos;
} constants;

layout(location = 0) out vec4 outNormal;
layout(location = 1) out vec4 outFragPos;

void main() {
   outFragPos = constants.model * vec4(inPos, 1.0);
   outNormal = normalize(constants.modelInvTrans * vec4(inNormal, 0.0));
   gl_Position = constants.vp * outFragPos;
}
