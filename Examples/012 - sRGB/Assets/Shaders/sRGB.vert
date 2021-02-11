#version 450 core

layout (location = 0) in vec3 inPos;

layout(push_constant) uniform PC {
   mat4 mvp;
   vec4 color;
} constants;

layout (location = 0) out vec4 outColor;

void main() {
   outColor = constants.color;
   gl_Position = constants.mvp * vec4(inPos, 1.0);
}
