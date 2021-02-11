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
   vec3 direction;
   float cutOff;
   vec3 ambient;
   vec3 diffuse;
   vec3 specular;
   float constant;
   float linear;
   float quadratic;
} light; 

layout (location = 0) out vec4 outFragColor;


void main() {

   vec4 lightDir = normalize(vec4(light.position, 1.0) - inFragPos);

   float theta = dot(lightDir, normalize(vec4(-light.direction, 0.0)));
    
   if(theta > light.cutOff) {       
      float diffuse = max(dot(inNormal, lightDir), 0.0);
   
      vec4 viewDir = normalize(vec4(constants.viewPos, 1.0) - inFragPos);
      vec4 reflectDir = reflect(-lightDir, inNormal);
      float specular = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
   
      float distance = length(vec4(light.position, 1.0) - inFragPos);
      float attenuation = 1.0 / (light.constant + (light.linear * distance) + (light.quadratic * (distance * distance))); 
   
      outFragColor = vec4(((light.diffuse * diffuse * attenuation) + light.ambient) * vec3(texture(diffuseMap, inTexCoords)) + (specular * attenuation * vec3(texture(specularMap, inTexCoords)) * light.specular), 1.0);
   } else {
      outFragColor = vec4(light.ambient * vec3(texture(diffuseMap, inTexCoords)), 1.0);
   }

}
