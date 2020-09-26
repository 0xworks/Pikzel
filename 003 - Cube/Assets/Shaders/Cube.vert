#version 450 core
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inTexCoord;

layout(push_constant) uniform PC {
   mat4 mvp;
} constants;

layout (location = 0) out vec2 outTexCoord;

void main() {
    outTexCoord = inTexCoord;
    gl_Position = constants.mvp * vec4(inPos, 1.0);
}
