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

layout(set = 0, binding = 2) uniform UBOMaterials {
    float shininess;
} material; 

struct DirectionalLight {
   vec3 direction;
   vec3 color;
   vec3 ambient;
};

struct PointLight {
   vec3 position;
   vec3 color;
   float constant;
   float linear;
   float quadratic;
};

layout(set = 0, binding = 3) uniform UBODirectionalLight {
   DirectionalLight light;
} directionalLight;

layout(set = 0, binding = 4) uniform UBOPointLights {
   PointLight light[4];
} pointLights;

layout (location = 0) out vec4 outFragColor;


vec4 CalculateDirectionalLight(DirectionalLight light, vec4 viewDir) {
   vec4 lightDir = normalize(vec4(-light.direction, 0.0));

   float diffuse = max(dot(inNormal, lightDir), 0.0);
   
   vec4 reflectDir = reflect(-lightDir, inNormal);
   float specular = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
   
   return vec4(((light.color * diffuse) + light.ambient) * vec3(texture(diffuseMap, inTexCoords)) + (specular * vec3(texture(specularMap, inTexCoords)) * light.color), 1.0);
}


vec4 CalculatePointLight(PointLight light, vec4 viewDir) {
   vec4 lightDir = normalize(vec4(light.position, 1.0) - inFragPos);

   float diffuse = max(dot(inNormal, lightDir), 0.0);
   
   vec4 reflectDir = reflect(-lightDir, inNormal);
   float specular = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
   
   float distance = length(vec4(light.position, 1.0) - inFragPos);
   float attenuation = 1.0 / (light.constant + (light.linear * distance) + (light.quadratic * (distance * distance))); 
   
   return vec4(((diffuse * attenuation * vec3(texture(diffuseMap, inTexCoords))) + (specular * attenuation * vec3(texture(specularMap, inTexCoords)))) * light.color, 1.0);
}


void main() {
   vec4 viewDir = normalize(vec4(constants.viewPos, 1.0) - inFragPos);
   outFragColor = CalculateDirectionalLight(directionalLight.light, viewDir);
   for(int i = 0; i < 4; ++i) {
      outFragColor += CalculatePointLight(pointLights.light[i], viewDir);
   }
}
