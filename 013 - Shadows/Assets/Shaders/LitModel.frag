#version 450 core
#extension GL_GOOGLE_include_directive: require

#include "PointLight.glsl"

layout(location = 0) in vec4 inNormal;
layout(location = 1) in vec4 inFragPos;
layout(location = 2) in vec4 inFragPosLightSpace;
layout(location = 3) in vec2 inTexCoords;

layout(push_constant) uniform PC {
   mat4 model;
   mat4 modelInvTrans;
   float farPlane;
   uint numPointLights;
} constants;

struct Matrices {
   mat4 vp;
   mat4 lightSpace;
   vec3 viewPos;
};

struct DirectionalLight {
   vec3 direction;
   vec3 color;
   vec3 ambient;
};

layout(set = 0, binding = 0) uniform UBOMatrices {
   Matrices matrices;
} uboMatrices;

layout(set = 1, binding = 0) uniform UBODirectionalLight {
   DirectionalLight light;
} directionalLight;

layout(set = 1, binding = 1) uniform UBOPointLights {
   PointLight light[MAX_POINT_LIGHTS];
} pointLights;

// TODO: try out samplerXXShadow here...
layout(set = 1, binding = 2) uniform sampler2D dirShadowMap;
layout(set = 1, binding = 3) uniform samplerCubeArray ptShadowMap;

layout(set = 2, binding = 0) uniform sampler2D diffuseMap;
layout(set = 2, binding = 1) uniform sampler2D specularMap;

layout (location = 0) out vec4 outFragColor;

const float pi = 3.14159265;
const float maxShininess = 128.0f;


float BlinnPhong(const vec4 lightDir, const vec4 viewDir, const vec4 normal, const float shininess) {
   const vec4 halfwayDir = normalize(lightDir + viewDir);
   return pow(max(dot(normal, halfwayDir), 0.0), shininess);
}


float CalculateDirectionalShadow(const vec4 fragPosLightSpace, const vec4 normal, const vec4 lightDir) {
   vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

   // note: Pikzel uses the Vulkan convention where NDC is -1 to 1 for x and y, and 0 to 1 for z
   projCoords = projCoords * vec3(0.5, 0.5, 1.0) + vec3(0.5, 0.5, 0.0);

   const float lightDepth = texture(dirShadowMap, projCoords.xy).r;
   const float fragDepth = projCoords.z > 1.0? 0.0 : projCoords.z;

   const float bias = 0.003 * (1.0 - dot(normal, lightDir));
   const float shadow = fragDepth - bias > lightDepth ? 1.0 : 0.0;
   return shadow;
}


vec4 CalculateDirectionalLight(const DirectionalLight light, const vec4 viewDir, const vec4 normal, const vec3 diffuseColor, const vec3 specularColor) {
   const vec4 lightDir = normalize(vec4(-light.direction, 0.0));

   const float diffuse = max(dot(normal, lightDir), 0.0);

   float specular = BlinnPhong(lightDir, viewDir, normal, specularColor.g * maxShininess); // shininess in specularmap green channel
   specular = diffuse == 0? 0.0 : specular;

   float shadow = CalculateDirectionalShadow(inFragPosLightSpace, normal, lightDir);

   return vec4(
      diffuseColor * (light.ambient + (light.color * diffuse * (1.0 - shadow))) +
      specularColor.r * (light.color * specular * (1.0 - shadow)), // specularity in specularmap red channel
      1.0
   );
}


float CalculatePointShadow(const uint lightIndex, const vec4 fragPos, const vec4 lightPos) {
   const vec4 fragToLight = fragPos - lightPos;
   const float lightDepth = texture(ptShadowMap, vec4(fragToLight.xyz, lightIndex)).r * constants.farPlane;
   const float fragDepth = length(fragToLight);
   const float bias = 0.003;
   return fragDepth - bias > lightDepth ? 1.0 : 0.0;
}


vec4 CalculatePointLight(const uint lightIndex, const vec4 viewDir, const vec4 normal, const vec3 diffuseColor, const vec3 specularColor) {
   const vec4 lightPos = vec4(pointLights.light[lightIndex].position, 1.0);
   const vec4 lightDir = normalize(lightPos - inFragPos);

   const float diffuse = max(dot(normal, lightDir), 0.0);   // diffuseIntensity

   float specular = BlinnPhong(lightDir, viewDir, normal, specularColor.g * maxShininess); // shininess in specularmap green channel
   specular = diffuse == 0? 0.0 : specular;

   const float distance = max(length(lightPos - inFragPos), 0.01);
   const float attenuation = pointLights.light[lightIndex].power / (distance * distance); 

   float shadow = CalculatePointShadow(lightIndex, inFragPos, lightPos);

   return vec4(
      ((diffuseColor * diffuse) + (specular * vec3(specularColor.r))) * pointLights.light[lightIndex].color * attenuation * (1.0 - shadow), // specularity in specularmap red channel
      1.0
   );
}


void main() {
   vec4 viewDir = normalize(vec4(uboMatrices.matrices.viewPos, 1.0) - inFragPos);
   vec4 normal = normalize(inNormal);
   vec3 diffuseColor = texture(diffuseMap, inTexCoords).rgb;
   vec3 specularColor = texture(specularMap, inTexCoords).rgb;

   outFragColor = CalculateDirectionalLight(directionalLight.light, viewDir, normal, diffuseColor, specularColor);

   for(uint i = 0; i < constants.numPointLights; ++i) {
      outFragColor += CalculatePointLight(i, viewDir, normal, diffuseColor, specularColor);
   }
}
