#version 450 core

layout(location = 0) in vec4 inNormal;
layout(location = 1) in vec4 inFragPos;
layout(location = 2) in vec2 inTexCoords;

layout(push_constant) uniform PC {
   mat4 vp;
   mat4 model;
   mat4 modelInvTrans;
   vec3 viewPos;
} constants;

layout(set = 0, binding = 0) uniform sampler2D diffuseMap;
layout(set = 0, binding = 1) uniform sampler2D specularMap;

layout(set = 0, binding = 2) uniform Materials {
    float shininess;
} material; 
  
layout(set = 0, binding = 3) uniform Lights {
   vec3 position;
   vec3 ambient;
   vec3 diffuse;
   vec3 specular;
} light; 

layout (location = 0) out vec4 outFragColor;


void main() {
   vec4 lightDir = normalize(vec4(light.position, 1.0) - inFragPos);
   float diffuse = max(dot(inNormal, lightDir), 0.0);

   vec4 viewDir = normalize(vec4(constants.viewPos, 1.0) - inFragPos);
   vec4 reflectDir = reflect(-lightDir, inNormal);
   float specular = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

   outFragColor = vec4(((light.diffuse * diffuse) + light.ambient) * vec3(texture(diffuseMap, inTexCoords)) + (specular * vec3(texture(specularMap, inTexCoords)) * light.specular), 1.0f);
}
