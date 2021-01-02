#version 450 core

layout(location = 0) in vec3 inFragPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoords;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outDiffuseSpecular;

layout(set = 1, binding = 0) uniform sampler2D diffuseMap;
layout(set = 1, binding = 1) uniform sampler2D specularMap;

void main() {
    outPosition = vec4(inFragPos, 1.0);
    outNormal = vec4(normalize(inNormal), 1.0);
    outDiffuseSpecular.rgb = texture(diffuseMap, inTexCoords).rgb;
    outDiffuseSpecular.a = texture(specularMap, inTexCoords).r;
}
