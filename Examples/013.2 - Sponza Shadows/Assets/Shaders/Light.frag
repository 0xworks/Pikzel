#version 450 core

layout(push_constant) uniform PC {
   mat4 mvp;
   vec3 lightColor;
} constants;

layout (location = 0) out vec4 outFragColor;

void main() {
   outFragColor = vec4(constants.lightColor, 1.0);
}
