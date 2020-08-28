#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;

out vec4 vColor;
out vec2 vTexCoord;

void main() {
    vColor = vec4(aColor.rgb, 1.0f);
    vTexCoord = aTexCoord;
    gl_Position = vec4(aPos.xyz, 1.0);
}
