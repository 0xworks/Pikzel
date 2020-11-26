#version 450 core

layout(location = 0) in vec4 inNormal;
layout(location = 1) in vec4 inFragPos;
layout(location = 2) in vec2 inTexCoords;

layout(push_constant) uniform PC {
   mat4 vp;
   mat4 model;
   mat4 modelInvTrans;
   vec3 viewPos;
   float shininess;
} constants;

struct DirectionalLight {
   vec3 direction;
   vec3 color;
   vec3 ambient;
};

struct PointLight {
   vec3 position;
   vec3 color;
   float power;
};

layout(set = 0, binding = 0) uniform UBODirectionalLight {
   DirectionalLight light;
} directionalLight;

layout(set = 0, binding = 1) uniform UBOPointLights {
   PointLight light[4];
} pointLights;

layout(set = 1, binding = 0) uniform sampler2D diffuseMap;
layout(set = 1, binding = 1) uniform sampler2D specularMap;

layout (location = 0) out vec4 outFragColor;

const float pi = 3.14159265;


float Phong(const vec4 lightDir, const vec4 viewDir, const vec4 normal, const float shininess) {
   const vec4 reflectDir = reflect(-lightDir, normal);
   const float energyPhong = (2.0 + constants.shininess) / (2.0 * pi); 
   return energyPhong * pow(max(dot(viewDir, reflectDir), 0.0), shininess);
}


float BlinnPhong(const vec4 lightDir, const vec4 viewDir, const vec4 normal, const float shininess) {
   const vec4 halfwayDir = normalize(lightDir + viewDir);
   const float energyBlinn = (8.0 + constants.shininess) / (8.0 * pi); 
   return energyBlinn * pow(max(dot(normal, halfwayDir), 0.0), constants.shininess);
}


vec4 CalculateDirectionalLight(DirectionalLight light, vec4 viewDir, vec4 normal, vec4 diffuseColor, vec4 specularColor) {
   const vec4 lightDir = normalize(vec4(-light.direction, 0.0));

   const float diffuse = max(dot(normal, lightDir), 0.0);

   //const float specular = Phong(lightDir, viewDir, normal, constants.shininess);
   float specular = BlinnPhong(lightDir, viewDir, normal, constants.shininess);
   specular = diffuse == 0? 0.0 : specular;

   return diffuseColor * (vec4(light.ambient, 1.0) + (vec4(light.color, 1.0) * diffuse)) + specularColor * (vec4(light.color, 1.0) * specular);
}


vec4 CalculatePointLight(PointLight light, vec4 viewDir, vec4 normal, vec4 diffuseColor, vec4 specularColor) {
   const vec4 lightDir = normalize(vec4(light.position, 1.0) - inFragPos);

   const float diffuse = max(dot(normal, lightDir), 0.0);

   //const float specular = Phong(lightDir, viewDir, normal, constants.shininess);
   float specular = BlinnPhong(lightDir, viewDir, normal, constants.shininess);
   specular = diffuse == 0? 0.0 : specular;

   const float distance = max(length(vec4(light.position, 1.0) - inFragPos), 0.01);
   const float attenuation = light.power / (distance * distance); 

   return ((diffuseColor * diffuse) + (specular * specularColor)) * vec4(light.color, 1.0) * attenuation;
}


void main() {
   vec4 viewDir = normalize(vec4(constants.viewPos, 1.0) - inFragPos);
   vec4 normal = normalize(inNormal);
   vec4 diffuseColor = texture(diffuseMap, inTexCoords);
   vec4 specularColor = texture(specularMap, inTexCoords);

   if (diffuseColor.a < 0.1) {
      discard;
   }

   outFragColor = CalculateDirectionalLight(directionalLight.light, viewDir, normal, diffuseColor, specularColor);

   for(int i = 0; i < 4; ++i) {
      outFragColor += CalculatePointLight(pointLights.light[i], viewDir, normal, diffuseColor, specularColor);
   }
}
