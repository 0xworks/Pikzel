#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;

layout (location = 0) out vec4 vColor;
layout (location = 1) out vec2 vTexCoord;

void main() {
    vColor = vec4(aColor.rgb, 1.0f);
    vTexCoord = aTexCoord;
    gl_Position = vec4(aPos.xyz, 1.0);
}
