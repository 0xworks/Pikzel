#version 450 core
layout (location = 0) in vec3 inPos;

layout(push_constant) uniform PC {
   mat4 mvp;
   vec3 lightColor;
} constants;

void main() {
    gl_Position = constants.mvp * vec4(inPos.xyz, 1.0);
}
