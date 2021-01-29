#version 450 core
#extension GL_GOOGLE_include_directive: require

#include "Lights.glsl"
#include "Matrices.glsl"

const float pi = 3.14159265;
const float maxShininess = 128.0f;
const int numPCSSSamples = 64;
const int numPCFSamples = 64;

layout(location = 0) in vec3 inFragPos;
layout(location = 1) in vec4 inFragPosLightSpace;
layout(location = 2) in vec2 inTexCoords;
layout(location = 3) in vec3 inTangentViewPos;
layout(location = 4) in vec3 inTangentFragPos;
layout(location = 5) in vec3 inTangentLightDir;
layout(location = 6) in vec3 inTangentLightPos[MAX_POINT_LIGHTS];


layout(push_constant) uniform PC {
   mat4 model;
   float lightRadius;
   uint numPointLights;
   uint showDirectionalLight;
   uint showPointLights;
   uint useNormalMaps;
   uint useDisplacementMaps;
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

layout(set = 2, binding = 0) uniform sampler2D diffuseMap;
layout(set = 2, binding = 1) uniform sampler2D specularMap;
layout(set = 2, binding = 2) uniform sampler2D normalMap;
layout(set = 2, binding = 3) uniform sampler2D displacementMap;

layout (location = 0) out vec4 outFragColor;


float BlinnPhong(const vec3 lightDir, const vec3 viewDir, const vec3 normal, const float shininess) {
   const vec3 halfwayDir = normalize(lightDir + viewDir);
   return pow(max(dot(normal, halfwayDir), 0.0), shininess);
}


vec3 CalculateDirectionalLight(const DirectionalLight light, const vec3 viewDir, const vec3 normal, const vec3 diffuseColor, const vec3 specularColor) {
   const vec3 lightDir = normalize(-inTangentLightDir); // POI: we are doing the lighting calulations in tangent space

   const float diffuse = max(dot(normal, lightDir), 0.0);
   if(diffuse > 0.0) {
      const float specular = BlinnPhong(lightDir, viewDir, normal, specularColor.g * maxShininess); // shininess in specularmap green channel

      return diffuseColor * (light.ambient + (light.color * diffuse)) +
         specularColor.r * (light.color * specular)                                                 // specularity in specularmap red channel
      ;
   }
   return diffuseColor * light.ambient;
}


vec3 CalculatePointLight(const uint lightIndex, const vec3 viewDir, const vec3 normal, const vec3 diffuseColor, const vec3 specularColor) {
   const vec3 lightPos = pointLights.light[lightIndex].position;
   const vec3 lightDir = normalize(inTangentLightPos[lightIndex] - inTangentFragPos);

   const float diffuse = max(dot(normal, lightDir), 0.0);   // diffuseIntensity

   if(diffuse > 0.0) {
      const float specular = BlinnPhong(lightDir, viewDir, normal, specularColor.g * maxShininess); // shininess in specularmap green channel
      const float distance = max(length(lightPos - inFragPos), 0.01);
      const float attenuation = pointLights.light[lightIndex].power / (distance * distance); 

      return (
         (diffuseColor * diffuse) +
         (specular * vec3(specularColor.r))                                                         // specularity in specularmap red channel
      ) * pointLights.light[lightIndex].color * attenuation;
   }
   return vec3(0.0);
}


vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir) {
   const float height_scale = 0.05;
   float height =  texture(displacementMap, texCoords).r;
   vec2 p = viewDir.xy  * (height * height_scale);
   return texCoords - p;
}


void main() {
   const vec3 viewDir = normalize(inTangentViewPos - inTangentFragPos);
   vec2 texCoords = inTexCoords;
   if(constants.useDisplacementMaps == 1) {
      texCoords = ParallaxMapping(inTexCoords, viewDir);
   }

   const vec4 diffuseColor = texture(diffuseMap, texCoords);

   if (diffuseColor.a < 0.1) {
      discard;
   }

   const vec3 specularColor = texture(specularMap, texCoords).rgb;
   vec3 normal = vec3(0.0, 0.0, 1.0); 
   if(constants.useNormalMaps == 1) {
      normal = normalize(texture(normalMap, texCoords).xyz * 2.0 - 1.0);
   }

   vec3 color = vec3(0);
   if(constants.showDirectionalLight == 1) {
      color += CalculateDirectionalLight(directionalLight.light, viewDir, normal, diffuseColor.rgb, specularColor);
   }

   if(constants.showPointLights == 1) {
      for(uint i = 0; i < constants.numPointLights; ++i) {
         color += CalculatePointLight(i, viewDir, normal, diffuseColor.rgb, specularColor);
      }
   }
   outFragColor = vec4(color, diffuseColor.a);
}
