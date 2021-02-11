#version 450 core

layout(location = 0) in vec4 inNormal;
layout(location = 1) in vec4 inFragPos;

layout(push_constant) uniform PC {
   mat4 vp;
   mat4 model;
   mat4 modelInvTrans;
   vec3 lightColor;
   vec3 lightPos;
   vec3 objectColor;
   vec3 viewPos;
} constants;

layout (location = 0) out vec4 outFragColor;

void main() {
   float ambient = 0.1;

   vec4 lightDir = normalize(vec4(constants.lightPos, 1.0) - inFragPos);
   float diffuse = max(dot(inNormal, lightDir), 0.0);

   vec4 viewDir = normalize(vec4(constants.viewPos, 1.0) - inFragPos);
   vec4 reflectDir = reflect(-lightDir, inNormal);
   float specular = 0.5 * pow(max(dot(viewDir, reflectDir), 0.0), 32);

   outFragColor = vec4((((ambient + diffuse) * constants.objectColor) + specular) * constants.lightColor, 1.0);
}
