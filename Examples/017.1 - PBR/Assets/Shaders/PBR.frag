#version 450 core
#extension GL_GOOGLE_include_directive: require

#include "Lights.glsl"
#include "Matrices.glsl"

const float pi = 3.14159265;
const int numPCSSSamples = 64;
const int numPCFSamples = 64;

layout(location = 0) in vec3 inFragPos;
layout(location = 1) in vec4 inFragPosLightSpace;
layout(location = 2) in vec2 inTexCoords;
layout(location = 3) in mat3 inTangentBasis;


layout(push_constant) uniform PC {
   mat4 model;
   vec2 textureRepeat;
   float heightScale;
} constants;


layout(set = 0, binding = 0) uniform UBOMatrices {
   Matrices matrices;
} uboMatrices;

layout(set = 1, binding = 0) uniform UBODirectionalLight {
   DirectionalLight light;
} directionalLight;


layout(set = 1, binding = 2) uniform sampler2D uDirShadowMap;
layout(set = 1, binding = 3) uniform samplerCube uIrradiance;
layout(set = 1, binding = 4) uniform samplerCube uSpecularIrradiance;
layout(set = 1, binding = 5) uniform sampler2D uSpecularBRDF_LUT;

layout(set = 2, binding = 0) uniform sampler2D uAlbedo;
layout(set = 2, binding = 1) uniform sampler2D uMetallicRoughness;  // Metallic in B, Roughness in G
layout(set = 2, binding = 2) uniform sampler2D uNormals;
layout(set = 2, binding = 3) uniform sampler2D uAmbientOcclusion;   // AO in R
layout(set = 2, binding = 4) uniform sampler2D uHeightMap;          // Height in R ( == 1.0 - displacement)

layout(location = 0) out vec4 outFragColor;

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


float BlockerDepth(vec3 shadowCoords, float bias, float lightSize) {
   float numBlockers = 0.0;
   float sumBlockers = 0.0;
   for (int i = 0; i < numPCSSSamples; ++i) {
      float z = texture(uDirShadowMap, shadowCoords.xy + poissonDisk[i] * lightSize).r;
      if(z > shadowCoords.z + bias) {
         numBlockers += 1.0;
         sumBlockers += z;
      }
   }
   return numBlockers > 0.0 ? sumBlockers / numBlockers : -1.0;
}


float PCFDirectionalShadow(vec3 shadowCoords, float bias, float radius) {
   float sum = 0;
   for (int i = 0; i < numPCFSamples; ++i) {
      float z = texture(uDirShadowMap, shadowCoords.xy + poissonDisk[i] * radius).r;
      if(z > shadowCoords.z + bias) {
         ++sum;
      }
   }
   return sum / numPCFSamples;
}


float CalculateDirectionalShadow(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
   vec3 shadowCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

   // note: Pikzel uses the Vulkan convention where NDC is -1 to 1 for x and y, and 0 to 1 for z
   //       Transform shadowCoords x and y to 0 to 1 here.  z is OK as is.
   shadowCoords = shadowCoords * vec3(0.5, 0.5, 1.0) + vec3(0.5, 0.5, 0.0);

   float shadow = 0.0;
   float bias = 0.001 * (1.0 - dot(-normal, lightDir));
   float lightSize = directionalLight.light.size;
   float blockerDepth = BlockerDepth(shadowCoords, bias, lightSize);
   if (blockerDepth > 0.0) {
      float penumbraWidth = lightSize * (blockerDepth - shadowCoords.z) / (1.0 - blockerDepth);
      shadow = PCFDirectionalShadow(shadowCoords, bias, penumbraWidth);
   }
   return shadow;
}


float TrowbridgeReitzGGX(float NdotH, float roughness) {
   float a2 = roughness * roughness * roughness * roughness;
   float denom  = (NdotH * NdotH * (a2 - 1.0) + 1.0);
   return a2 / (pi * denom * denom);
}


float GeometrySchlickGGX(float NdotV, float roughness) {
   float r = roughness + 1.0;
   float k = (r*r) / 8.0;
   return NdotV / (NdotV * (1.0 - k) + k);
}


float GeometrySmith(float NdotV, float NdotL, float roughness) {
    return GeometrySchlickGGX(NdotV, roughness) * GeometrySchlickGGX(NdotL, roughness);
}


vec3 FresnelSchlick(float cosTheta, vec3 F0) {
   return F0 + (vec3(1.0) - F0) * pow(1.0 - cosTheta, 5.0);
}


vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
   return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}


vec2 ParallaxMapping(vec2 texCoords, vec3 viewDirTangentSpace, float height) {
   float displacement =  1.0 - height;
   vec2 p = viewDirTangentSpace.xy  * (displacement * constants.heightScale);
   return texCoords - p;
}


void main() {
   vec3 viewDir = normalize(uboMatrices.matrices.eyePosition - inFragPos); // world space

   vec2 texCoords = ParallaxMapping(inTexCoords, inTangentBasis * viewDir, texture(uHeightMap, inTexCoords).r);

   vec3 albedo = texture(uAlbedo, texCoords).rgb;
   vec3 normal = normalize(inTangentBasis * (texture(uNormals, texCoords).xyz * 2.0 - 1.0));
   vec3 metallicRoughness = texture(uMetallicRoughness, texCoords).rgb;
   float metalness = metallicRoughness.b;
   float roughness = metallicRoughness.g;

   vec3 F0 = mix(vec3(0.04), albedo, metalness);

   vec3 lightDir = normalize(-directionalLight.light.direction); // world space
   vec3 halfwayDir = normalize(viewDir + lightDir);

   float NdotV = max(dot(normal, viewDir), 0.0);
   float NdotL = max(dot(normal, lightDir), 0.0);
   float NdotH = max(dot(normal, halfwayDir), 0.0);
   float HdotV = max(dot(halfwayDir, viewDir), 0.0);

   // Direct lighting
   vec3 directLight = vec3(0.0);
   {
      float D = TrowbridgeReitzGGX(NdotH, roughness);
      float G = GeometrySmith(NdotV, NdotL, roughness);
      vec3 F  = FresnelSchlick(HdotV, F0);
      vec3 specularBRDF = D * G * F / max(4.0 * NdotV * NdotL, 0.001);

      vec3 Kd = mix(vec3(1.0) - F, vec3(0.0), metalness); // Ks == F
      // Lambertian diffuse term not scaled by 1/pi. See: https://seblagarde.wordpress.com/2012/01/08/pi-or-not-to-pi-in-game-lighting-equation/
      vec3 diffuseBRDF = Kd * albedo;

      vec3 radiance = directionalLight.light.color * (1.0 - CalculateDirectionalShadow(inFragPosLightSpace, normal, lightDir));

      directLight = (diffuseBRDF + specularBRDF) * radiance * NdotL;
   }

   // Ambient lighting
   vec3 ambientLight = vec3(0.0);
   {
      vec3 irradiance = texture(uIrradiance, normal).rgb;
      float ao = texture(uAmbientOcclusion, texCoords).r;

      vec3 F = FresnelSchlickRoughness(NdotV, F0, roughness); // Ks == F
      vec3 Kd = mix(vec3(1.0) - F, vec3(0.0), metalness);
      vec3 diffuseIBL = Kd * albedo * irradiance;

      // Sample pre-filtered specular reflection environment at correct mipmap level.
      vec3 reflectDir = (2.0 * NdotV * normal) - viewDir;
      float specularTextureLevels = textureQueryLevels(uSpecularIrradiance) - 1;
      vec3 specularIrradiance = textureLod(uSpecularIrradiance, reflectDir, roughness * specularTextureLevels).rgb;

      // Split-sum approximation factors for Cook-Torrance specular BRDF.
      vec2 specularBRDF = texture(uSpecularBRDF_LUT, vec2(NdotV, roughness)).rg;
      vec3 specularIBL = (F0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;

      ambientLight = (diffuseIBL + specularIBL) * ao;
   }
   outFragColor = vec4(directLight + (ambientLight * directionalLight.light.ambient), 1.0);
}
