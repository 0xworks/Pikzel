#version 450 core
#extension GL_GOOGLE_include_directive: require

#include "Lights.glsl"
#include "Matrices.glsl"

const float pi = 3.14159265;
const float maxShininess = 128.0f;
const int numPCSSSamples = 64;
const int numPCFSamples = 64;


layout(location = 0) in vec4 inNormal;
layout(location = 1) in vec4 inFragPos;
layout(location = 2) in vec4 inFragPosLightSpace;
layout(location = 3) in vec2 inTexCoords;

layout(push_constant) uniform PC {
   mat4 model;
   mat4 modelInvTrans;
   float lightRadius;
   uint numPointLights;
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

layout(set = 1, binding = 2) uniform sampler2D dirShadowMap;
layout(set = 1, binding = 3) uniform samplerCubeArray ptShadowMap;

layout(set = 2, binding = 0) uniform sampler2D diffuseMap;
layout(set = 2, binding = 1) uniform sampler2D specularMap;

layout (location = 0) out vec4 outFragColor;


float BlinnPhong(const vec4 lightDir, const vec4 viewDir, const vec4 normal, const float shininess) {
   const vec4 halfwayDir = normalize(lightDir + viewDir);
   return pow(max(dot(normal, halfwayDir), 0.0), shininess);
}


const vec2 poissonDisk[64] = vec2[](
   vec2(-0.535216938, 0.316231875),
   vec2(-0.365512938, 0.219683875),
   vec2(-0.399080938, 0.419665875),
   vec2(-0.590744938, 0.435377875),
   vec2(-0.636600938, 0.237277875),
   vec2(-0.512502938, 0.055521875),
   vec2(-0.533069938, 0.588651875),
   vec2(-0.118073938, 0.206269875),
   vec2(-0.209342938, 0.404405875),
   vec2(-0.229582938, 0.095921875),
   vec2(-0.391401938, 0.096112875),
   vec2(-0.402816938, 0.664347875),
   vec2(-0.204282938, -0.051433125),
   vec2(-0.325897938, -0.138986125),
   vec2(-0.053900937, 0.069656875),
   vec2(0.029088063, -0.120422125),
   vec2(-0.065058937, -0.248013125),
   vec2(-0.630288938, -0.009501125),
   vec2(-0.516714938, -0.096951125),
   vec2(0.105160063, 0.005365875),
   vec2(0.053944063, 0.135995875),
   vec2(-0.255587938, -0.352507125),
   vec2(-0.069191937, -0.395935125),
   vec2(-0.200291938, -0.224133125),
   vec2(0.110784063, -0.420017125),
   vec2(0.081860063, -0.267958125),
   vec2(0.248858063, -0.037372125),
   vec2(0.246936063, -0.188638125),
   vec2(-0.332602938, -0.509029125),
   vec2(-0.414623938, -0.351642125),
   vec2(-0.200165938, -0.559005125),
   vec2(-0.460180938, -0.216994125),
   vec2(-0.039269937, -0.581704125),
   vec2(-0.080527937, -0.703148125),
   vec2(0.217267063, 0.256801875),
   vec2(0.073862063, 0.294665875),
   vec2(0.242747063, 0.123416875),
   vec2(0.054278063, -0.699771125),
   vec2(-0.280553938, 0.571130875),
   vec2(-0.058392937, 0.531491875),
   vec2(0.420514063, -0.192540125),
   vec2(0.370882063, -0.072049125),
   vec2(0.352743063, 0.055670875),
   vec2(0.211331063, -0.576100125),
   vec2(0.297990063, -0.714324125),
   vec2(0.462997063, 0.121690875),
   vec2(0.512178063, -0.025487125),
   vec2(0.248602063, -0.396248125),
   vec2(0.343922063, 0.317111875),
   vec2(0.384166063, -0.427566125),
   vec2(0.544510063, -0.267278125),
   vec2(0.652833063, -0.154618125),
   vec2(-0.329253938, 0.876842875),
   vec2(-0.279553938, 0.699721875),
   vec2(-0.159608938, 0.650496875),
   vec2(0.380998063, -0.590286125),
   vec2(0.471459063, 0.472096875),
   vec2(0.305221063, 0.503862875),
   vec2(0.481857063, 0.276913875),
   vec2(0.156758063, 0.477591875),
   vec2(0.532485063, -0.521498125),
   vec2(0.614084063, -0.404972125),
   vec2(0.339236063, -0.291314125),
   vec2(0.330348063, 0.627446875)
);


float BlockerDepth(const vec3 shadowCoords, const float bias, const float lightSize) {
   float numBlockers = 0.0;
   float sumBlockers = 0.0;
   for (int i = 0; i < numPCSSSamples; ++i) {
      const float z = texture(dirShadowMap, shadowCoords.xy + poissonDisk[i] * lightSize).r;
      if(z < shadowCoords.z - bias) {
         numBlockers += 1.0;
         sumBlockers += z;
      }
   }
   return numBlockers > 0.0 ? sumBlockers / numBlockers : -1.0;
}


float PCFDirectionalShadow(const vec3 shadowCoords, const float bias, const float radius) {
   float sum = 0;
   for (int i = 0; i < numPCFSamples; ++i) {
      float z = texture(dirShadowMap, shadowCoords.xy + poissonDisk[i] * radius).r;
      if(z < shadowCoords.z - bias) {
         ++sum;
      }
   }
   return sum / numPCFSamples;
}


float CalculateDirectionalShadow(const vec4 fragPosLightSpace, const vec4 normal, const vec4 lightDir) {
   vec3 shadowCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

   // note: Pikzel uses the Vulkan convention where NDC is -1 to 1 for x and y, and 0 to 1 for z
   //       Transform shadowCoords x and y to 0 to 1 here.  z is OK as is.
   shadowCoords = shadowCoords * vec3(0.5, 0.5, 1.0) + vec3(0.5, 0.5, 0.0);
   shadowCoords.z = shadowCoords.z > 1.0? 0.0 : shadowCoords.z;

   const float lightSize = 0.02;
   const float bias = 0.001 * (1.0 - dot(-normal, lightDir));
   const float blockerDepth = BlockerDepth(shadowCoords, bias, lightSize);
   if (blockerDepth > 0.0) {
      const float penumbraWidth = lightSize * (shadowCoords.z - blockerDepth) / blockerDepth;
      return PCFDirectionalShadow(shadowCoords, bias, penumbraWidth);
   }
   return 0.0;
}


vec3 CalculateDirectionalLight(const DirectionalLight light, const vec4 viewDir, const vec4 normal, const vec3 diffuseColor, const vec3 specularColor) {
   const vec4 lightDir = normalize(vec4(-light.direction, 0.0));

   const float diffuse = max(dot(normal, lightDir), 0.0);
   if(diffuse > 0.0) {
      const float specular = BlinnPhong(lightDir, viewDir, normal, specularColor.g * maxShininess); // shininess in specularmap green channel
      const float notShadowed = 1.0 - CalculateDirectionalShadow(inFragPosLightSpace, normal, lightDir);

      return diffuseColor * (light.ambient + (light.color * diffuse * notShadowed)) +
         specularColor.r * (light.color * specular * notShadowed)
      ; // specularity in specularmap red channel
   }
   return diffuseColor * light.ambient;
}


float CalculatePointShadow(const uint lightIndex, const vec4 fragPos, const vec4 lightPos) {
   const float bias = 0.001;
   const vec4 fragToLight = fragPos - lightPos;
   const float fragDepth = (length(fragToLight) / constants.lightRadius) - bias;
   float closestDepth = texture(ptShadowMap, vec4(fragToLight.xyz, lightIndex)).r;
   return (closestDepth < fragDepth) ? 1.0f : 0.0f;
}


vec3 CalculatePointLight(const uint lightIndex, const vec4 viewDir, const vec4 normal, const vec3 diffuseColor, const vec3 specularColor) {
   const vec4 lightPos = vec4(pointLights.light[lightIndex].position, 1.0);
   const vec4 lightDir = normalize(lightPos - inFragPos);

   const float diffuse = max(dot(normal, lightDir), 0.0);   // diffuseIntensity

   if(diffuse > 0.0) {
      const float specular = BlinnPhong(lightDir, viewDir, normal, specularColor.g * maxShininess); // shininess in specularmap green channel
      const float notShadowed = 1.0 - CalculatePointShadow(lightIndex, inFragPos, lightPos);
      const float distance = max(length(lightPos - inFragPos), 0.01);
      const float attenuation = pointLights.light[lightIndex].power / (distance * distance); 

      return (
         (diffuseColor * diffuse) +
         (specular * vec3(specularColor.r))
      ) * pointLights.light[lightIndex].color * attenuation * notShadowed; // specularity in specularmap red channel
   }
   return vec3(0.0);
}


void main() {
   const vec4 viewDir = normalize(vec4(uboMatrices.matrices.eyePosition, 1.0) - inFragPos);
   const vec4 normal = normalize(inNormal);
   const vec4 diffuseColor = texture(diffuseMap, inTexCoords);

   if (diffuseColor.a < 0.1) {
      discard;
   }

   const vec3 specularColor = texture(specularMap, inTexCoords).rgb;

   vec3 color = CalculateDirectionalLight(directionalLight.light, viewDir, normal, diffuseColor.rgb, specularColor);

   for(uint i = 0; i < constants.numPointLights; ++i) {
      color += CalculatePointLight(i, viewDir, normal, diffuseColor.rgb, specularColor);
   }
   outFragColor = vec4(color, diffuseColor.a);
}
