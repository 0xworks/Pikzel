#version 450 core
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inTexCoord;

layout (location = 0) out vec2 outTexCoord;

void main() {
    outTexCoord = inTexCoord;
    gl_Position = vec4(inPos.xyz, 1.0);
}
