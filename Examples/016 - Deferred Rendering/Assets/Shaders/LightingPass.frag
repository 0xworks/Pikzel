#version 450 core
#extension GL_GOOGLE_include_directive: require

#include "Lights.glsl"
#include "Matrices.glsl"

const float maxShininess = 128.0f;

layout(location = 0) in vec2 inTexCoords;


layout(push_constant) uniform PC {
   uint numPointLights;
   uint showDirectionalLight;
   uint showPointLights;
} constants;


layout(set = 0, binding = 0) uniform UBOMatrices {
   Matrices matrices;
} uboMatrices;

layout(set = 1, binding = 0) uniform UBODirectionalLight {
   DirectionalLight light;
} directionalLight;

layout(set = 1, binding = 1) uniform UBOPointLights {
   PointLight light[MAX_POINT_LIGHTS];
} pointLights;

layout(set = 2, binding = 0) uniform sampler2D uPosition;
layout(set = 2, binding = 1) uniform sampler2D uNormal;
layout(set = 2, binding = 2) uniform sampler2D uDiffuseSpecular;

layout(location = 0) out vec4 outFragColor;


float BlinnPhong(const vec3 lightDir, const vec3 viewDir, const vec3 normal, const float shininess) {
   const vec3 halfwayDir = normalize(lightDir + viewDir);
   return pow(max(dot(normal, halfwayDir), 0.0), shininess);
}


vec3 CalculateDirectionalLight(const DirectionalLight light, const vec3 viewDir, const vec3 normal, const vec3 diffuseColor, const vec3 specularColor) {
   const vec3 lightDir = normalize(-light.direction);

   const float diffuse = max(dot(normal, lightDir), 0.0);
   vec3 color = diffuseColor * light.ambient;
   if(diffuse > 0.0) {
      const float specular = BlinnPhong(lightDir, viewDir, normal, specularColor.g * maxShininess);   // shininess in specularmap green channel

      color += (diffuse * diffuseColor * light.color) +
         (specular * specularColor.r * light.color)                                                   // specularity in specularmap red channel
      ;
   }
   return color;
}


vec3 CalculatePointLight(const uint lightIndex, const vec3 fragPos, const vec3 viewDir, const vec3 normal, const vec3 diffuseColor, const vec3 specularColor) {
   const vec3 lightPos = pointLights.light[lightIndex].position;
   const vec3 lightDir = normalize(lightPos - fragPos);

   const float diffuse = max(dot(normal, lightDir), 0.0);

   vec3 color = vec3(0.0);
   if(diffuse > 0.0) {
      const float specular = BlinnPhong(lightDir, viewDir, normal, specularColor.g * maxShininess); // shininess in specularmap green channel
      const float distance = max(length(lightPos - fragPos), 0.01);
      const float attenuation = pointLights.light[lightIndex].power / (distance * distance);

      color += (
         (diffuse * diffuseColor) +
         (specular * vec3(specularColor.r))
      ) * pointLights.light[lightIndex].color * attenuation;                                        // specularity in specularmap red channel
   }
   return color;
}


void main() {
   const vec3 fragPos = texture(uPosition, inTexCoords).xyz;
   const vec3 normal = texture(uNormal, inTexCoords).xyz;
   const vec3 diffuseColor = texture(uDiffuseSpecular, inTexCoords).rgb;
   const vec3 specularColor = vec3(texture(uDiffuseSpecular, inTexCoords).a, 0.125, 0.0);

   const vec3 viewDir = normalize(uboMatrices.matrices.eyePosition - fragPos);

   vec3 color = vec3(0);
   if(constants.showDirectionalLight == 1) {
      color += CalculateDirectionalLight(directionalLight.light, viewDir, normal, diffuseColor, specularColor);
   }

   if(constants.showPointLights == 1) {
      for(uint i = 0; i < constants.numPointLights; ++i) {
         color += CalculatePointLight(i, fragPos, viewDir, normal, diffuseColor.rgb, specularColor);
      }
   }

   outFragColor = vec4(color, 1.0);
}
