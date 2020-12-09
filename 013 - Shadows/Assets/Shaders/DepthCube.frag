#version 450 core
layout(location = 0) in vec4 inFragPos;

layout(push_constant) uniform PC {
   mat4 model;
   vec3 lightPos;
   float farPlane;
} constants;

void main() {
   gl_FragDepth = length(inFragPos.xyz - constants.lightPos) / constants.farPlane;
}
