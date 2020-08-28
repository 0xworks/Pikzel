#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec4 vColor;

void main() {
    vColor = vec4(aColor.rgb, 1.0f);
    gl_Position = vec4(aPos.xyz, 1.0);
}
