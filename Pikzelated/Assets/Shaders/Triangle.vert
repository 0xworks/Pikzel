#version 450 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inColor;

// for now Pikzel:
// - requires that interface blocks have an instance name
// - does not support structs within a block
layout(push_constant) uniform PC {
   mat4 mvp;
} constants;

layout (location = 0) out vec3 outColor;

void main() {
   outColor = inColor;
   gl_Position = constants.mvp * vec4(inPos, 1.0);
}
