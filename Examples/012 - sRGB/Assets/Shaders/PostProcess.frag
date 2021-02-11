#version 450 core
layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outFragColor;

layout(binding = 0) uniform sampler2D uTexture;


void main() {
   outFragColor = vec4(texture(uTexture, inUV).rgb, 1.0);
}
