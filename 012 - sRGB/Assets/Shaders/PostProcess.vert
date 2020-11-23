#version 450 core
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUV;

layout (location = 0) out vec2 outUV;


void main() {
    outUV = inUV;
    gl_Position = vec4(inPos.xy, 0.0, 1.0);
}
