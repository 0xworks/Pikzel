#version 450 core

layout(location = 0) in vec4 inNormal;
layout(location = 1) in vec4 inFragPos;
layout(location = 2) in vec4 inFragPosLightSpace;
layout(location = 3) in vec2 inTexCoords;

layout(push_constant) uniform PC {
   mat4 model;
   mat4 modelInvTrans;
   float farPlane;
} constants;

struct Matrices {
   mat4 vp;
   mat4 lightSpace;
   vec3 viewPos;
};

layout(set = 0, binding = 0) uniform UBOMatrices {
   Matrices matrices;
} uboMatrices;

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

layout(set = 1, binding = 0) uniform UBODirectionalLight {
   DirectionalLight light;
} directionalLight;

layout(set = 1, binding = 1) uniform UBOPointLights {
   PointLight light[1];
} pointLights;

layout(set = 1, binding = 2) uniform sampler2D dirShadowMap;
layout(set = 1, binding = 3) uniform samplerCube ptShadowMap;

layout(set = 2, binding = 0) uniform sampler2D diffuseMap;
layout(set = 2, binding = 1) uniform sampler2D specularMap;

layout (location = 0) out vec4 outFragColor;

const float pi = 3.14159265;
const float maxShininess = 128.0f;


float BlinnPhong(const vec4 lightDir, const vec4 viewDir, const vec4 normal, const float shininess) {
   const vec4 halfwayDir = normalize(lightDir + viewDir);
   return pow(max(dot(normal, halfwayDir), 0.0), shininess);
}


float CalculateDirectionalShadow(const vec4 fragPosLightSpace, const vec4 normal, const vec4 lightDir, const sampler2D map) {
   vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

   // note: Pikzel uses the Vulkan convention where NDC is -1 to 1 for x and y, and 0 to 1 for z
   projCoords = projCoords * vec3(0.5, 0.5, 1.0) + vec3(0.5, 0.5, 0.0);

   const float lightDepth = texture(map, projCoords.xy).r;
   const float fragDepth = projCoords.z > 1.0? 0.0 : projCoords.z;

   const float bias = 0.003 * (1.0 - dot(normal, lightDir));
   const float shadow = fragDepth - bias > lightDepth ? 1.0 : 0.0;
   return shadow;
}


vec4 CalculateDirectionalLight(DirectionalLight light, vec4 viewDir, vec4 normal, vec3 diffuseColor, vec3 specularColor) {
   const vec4 lightDir = normalize(vec4(-light.direction, 0.0));

   const float diffuse = max(dot(normal, lightDir), 0.0);

   float specular = BlinnPhong(lightDir, viewDir, normal, specularColor.g * maxShininess); // shininess in specularmap green channel
   specular = diffuse == 0? 0.0 : specular;

   float shadow = CalculateDirectionalShadow(inFragPosLightSpace, normal, lightDir, dirShadowMap);

   return vec4(
      diffuseColor * (light.ambient + (light.color * diffuse * (1.0 - shadow))) +
      specularColor.r * (light.color * specular * (1.0 - shadow)), // specularity in specularmap red channel
      1.0
   );
}


float CalculatePointShadow(const vec3 fragPos, const vec3 lightPos, const samplerCube map) {
   const vec3 fragToLight = fragPos - lightPos;
   const float lightDepth = texture(map, fragToLight.xyz).r * constants.farPlane;
   const float fragDepth = length(fragToLight);
   const float bias = 0.003;
   return fragDepth - bias > lightDepth ? 1.0 : 0.0;
}


vec4 CalculatePointLight(PointLight light, vec4 viewDir, vec4 normal, vec3 diffuseColor, vec3 specularColor) {
   const vec4 lightDir = normalize(vec4(light.position, 1.0) - inFragPos);

   const float diffuse = max(dot(normal, lightDir), 0.0);   // diffuseIntensity

   float specular = BlinnPhong(lightDir, viewDir, normal, specularColor.g * maxShininess); // shininess in specularmap green channel
   specular = diffuse == 0? 0.0 : specular;

   const float distance = max(length(vec4(light.position, 1.0) - inFragPos), 0.01);
   const float attenuation = light.power / (distance * distance); 

   float shadow = CalculatePointShadow(inFragPos.xyz, pointLights.light[0].position, ptShadowMap);

   return vec4(
      ((diffuseColor * diffuse) + (specular * vec3(specularColor.r))) * light.color * attenuation * (1.0 - shadow), // specularity in specularmap red channel
      1.0
   );
}


void main() {
   vec4 viewDir = normalize(vec4(uboMatrices.matrices.viewPos, 1.0) - inFragPos);
   vec4 normal = normalize(inNormal);
   vec3 diffuseColor = texture(diffuseMap, inTexCoords).rgb;
   vec3 specularColor = texture(specularMap, inTexCoords).rgb;

   outFragColor = CalculateDirectionalLight(directionalLight.light, viewDir, normal, diffuseColor, specularColor);

   for(int i = 0; i < 1; ++i) {
      outFragColor += CalculatePointLight(pointLights.light[i], viewDir, normal, diffuseColor, specularColor);
   }
}
