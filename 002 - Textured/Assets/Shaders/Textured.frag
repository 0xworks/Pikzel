#version 330 core
in vec4 vColor;
in vec2 vTexCoord;

out vec4 outFragColor;

uniform sampler2D uTexture;

void main() {
   outFragColor = texture(uTexture, vTexCoord) * vColor;
}
