#version 450 core

layout(push_constant) uniform PC {
   mat4 mvp;
   vec3 color;
} constants;

layout(location = 0) out vec4 outFragColor;
layout(location = 1) out vec4 outBrightColor;

void main() {
   outFragColor = vec4(constants.color, 1.0);
   outBrightColor = outFragColor;
}
