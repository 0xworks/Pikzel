#version 450 core
#extension GL_GOOGLE_include_directive: require

#include "Lights.glsl"

layout(location = 0) in vec4 inFragPos;

layout(push_constant) uniform PC {
   mat4 model;
   float lightRadius;
   int lightIndex;
} constants;

layout(set = 1, binding = 0) uniform UBOPointLights {
   PointLight light[MAX_POINT_LIGHTS];
} pointLights;

void main() {
   gl_FragDepth = length(inFragPos.xyz - pointLights.light[constants.lightIndex].position) / constants.lightRadius;
}
