#version 450 core
layout (location = 0) in vec3 inPos;

layout(push_constant) uniform PC {
   mat4 model;
   vec3 lightPos;
   float farPlane;
} constants;

void main() {
    gl_Position = constants.model * vec4(inPos.xyz, 1.0);
}
