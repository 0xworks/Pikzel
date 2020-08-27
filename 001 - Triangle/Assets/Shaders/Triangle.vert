#version 330 core
layout (location = 0) in vec3 inPos;

void main() {
    gl_Position = vec4(inPos.xyz, 1.0);
}
