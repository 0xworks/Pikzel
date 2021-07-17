#version 450 core

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inTexCoords;

layout(push_constant) uniform PC {
   mat4 mvp;
} constants;

layout (location = 0) out vec3 outColor;

void main() {
   outColor = vec3(inTexCoords, 0);
   gl_Position = constants.mvp * vec4(inPos, 1.0);
}
