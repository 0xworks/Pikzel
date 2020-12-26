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

layout(set = 0, binding = 0) uniform sampler2D diffuseMap;
layout(set = 0, binding = 1) uniform sampler2D specularMap;

struct DirectionalLight {
   vec3 direction;
   vec3 color;
   vec3 ambient;
   float size;
};

struct PointLight {
   vec3 position;
   vec3 color;
   float size;
   float power;
};

layout(set = 0, binding = 2) uniform UBODirectionalLight {
   DirectionalLight light;
} directionalLight;

layout(set = 0, binding = 3) uniform UBOPointLights {
   PointLight light[4];
} pointLights;

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


vec4 CalculateDirectionalLight(DirectionalLight light, vec4 viewDir, vec4 normal, vec3 diffuseColor, vec3 specularColor) {
   const vec4 lightDir = normalize(vec4(-light.direction, 0.0));

   const float diffuse = max(dot(normal, lightDir), 0.0);

   //const float specular = Phong(lightDir, viewDir, normal, constants.shininess);
   float specular = BlinnPhong(lightDir, viewDir, normal, constants.shininess);
   specular = diffuse == 0? 0.0 : specular;

   return vec4(
      diffuseColor * (light.ambient + (light.color * diffuse)) +
      specularColor * (light.color * specular),
      1.0
   );
}


vec4 CalculatePointLight(PointLight light, vec4 viewDir, vec4 normal, vec3 diffuseColor, vec3 specularColor) {
   const vec4 lightDir = normalize(vec4(light.position, 1.0) - inFragPos);

   const float diffuse = max(dot(normal, lightDir), 0.0);

   //const float specular = Phong(lightDir, viewDir, normal, constants.shininess);
   float specular = BlinnPhong(lightDir, viewDir, normal, constants.shininess);
   specular = diffuse == 0? 0.0 : specular;

   const float distance = max(length(vec4(light.position, 1.0) - inFragPos), 0.01);
   const float attenuation = light.power / (distance * distance); 

   return vec4(
      ((diffuseColor * diffuse) + (specular * specularColor)) * light.color * attenuation,
      1.0
   );
}


void main() {
   vec4 viewDir = normalize(vec4(constants.viewPos, 1.0) - inFragPos);
   vec4 normal = normalize(inNormal);
   vec3 diffuseColor = texture(diffuseMap, inTexCoords).rgb;
   vec3 specularColor = texture(specularMap, inTexCoords).rgb;

   outFragColor = CalculateDirectionalLight(directionalLight.light, viewDir, normal, diffuseColor, specularColor);

   for(int i = 0; i < 4; ++i) {
      outFragColor += CalculatePointLight(pointLights.light[i], viewDir, normal, diffuseColor, specularColor);
   }
}
