#version 450 core
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec4 inColor;
layout (location = 2) in vec2 inTexCoord;

layout(push_constant) uniform PC {
   mat4 mvp;
} constants;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec2 outTexCoord;

void main() {
    outColor = vec4(inColor.rgb, 1.0f);
    outTexCoord = inTexCoord;
    gl_Position = constants.mvp * vec4(inPos.xyz, 1.0);
}
