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
layout(set = 2, binding = 2) uniform sampler2D normalMap;

layout (location = 0) out vec4 outFragColor;


float BlinnPhong(const vec3 lightDir, const vec3 viewDir, const vec3 normal, const float shininess) {
   const vec3 halfwayDir = normalize(lightDir + viewDir);
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

const vec3 poissonSphere[64] = vec3[](
   vec3(-0.044263731, 0.133323159, 0.954641225),
   vec3(0.061727517, -0.134922937, -0.112943223),
   vec3(-0.00139494, 0.075791016, 0.224881291),
   vec3(0.396232598, 0.295744951, 0.876582669),
   vec3(-0.11154675, -0.127642066, -0.535845628),
   vec3(-0.340709156, -0.493824916, 0.558960097),
   vec3(-0.144509189, -0.733564402, 0.172877031),
   vec3(-0.666778464, -0.151001481, 0.073223502),
   vec3(-0.137345963, -0.894431898, 0.725054491),
   vec3(0.211161236, -0.406992904, 0.172493665),
   vec3(-0.612436565, 0.325673514, 0.484385512),
   vec3(0.694342833, 0.009264617, 0.791338626),
   vec3(-0.405504763, 0.553018187, 0.862419089),
   vec3(0.423321759, -0.704795842, 0.57083431),
   vec3(-0.671320064, -0.138169755, 0.786735002),
   vec3(0.114637233, -0.462273951, 0.962318438),
   vec3(0.344575976, 0.340471332, 0.154520094),
   vec3(-0.6919651, -0.49935298, -0.658610979),
   vec3(-0.236112842, -0.242617257, 0.959282467),
   vec3(0.664843909, -0.41139901, 0.872871537),
   vec3(-0.671145013, -0.64518018, 0.050099955),
   vec3(-0.667573866, 0.030067095, -0.366412882),
   vec3(0.284790285, -0.091733699, 0.568420842),
   vec3(0.136513086, -0.474810177, -0.408637159),
   vec3(0.261035423, 0.770113441, -0.04045784),
   vec3(-0.39500358, 0.405593266, 0.154344626),
   vec3(-0.985472734, 0.231965654, 0.976105929),
   vec3(-0.680205153, 0.439197648, -0.345763106),
   vec3(-0.794986363, -0.809452124, 0.771264893),
   vec3(0.578612702, -0.333508496, 0.019169701),
   vec3(-0.572286915, -0.945534977, 0.32860361),
   vec3(-0.161870297, -0.990421901, -0.332392384),
   vec3(0.195611675, -0.988624898, -0.833597961),
   vec3(-0.211985099, -0.789567363, -0.812431491),
   vec3(0.428638157, 0.163855905, -0.395917597),
   vec3(0.315630851, -0.556746012, -0.849661379),
   vec3(0.712215495, -0.740265998, -0.117231035),
   vec3(-0.604685332, -0.966662797, -0.669213485),
   vec3(-0.625485285, -0.069189744, -0.794117653),
   vec3(-0.70258163, 0.644590851, 0.259775229),
   vec3(-0.654490107, 0.974158441, 0.801241317),
   vec3(0.023234655, 0.470224493, -0.23254748),
   vec3(-0.463600624, -0.055251216, 0.414476094),
   vec3(-0.78184984, 0.478015209, -0.84931698),
   vec3(-0.975269758, -0.295164888, 0.496620398),
   vec3(0.195563213, 0.555400136, -0.672300979),
   vec3(0.537458097, 0.949424407, 0.995892851),
   vec3(-0.965270462, 0.257746395, -0.566001904),
   vec3(-0.208767612, 0.223839168, -0.706116559),
   vec3(-0.355135564, 0.861070942, -0.609593664),
   vec3(0.751076185, 0.790835263, 0.606500747),
   vec3(0.929853392, 0.175799212, 0.453344576),
   vec3(0.934221942, 0.430715855, 0.114923059),
   vec3(0.273842422, -0.051124881, -0.738125107),
   vec3(-0.144819895, 0.783242776, -0.978328236),
   vec3(0.719240925, 0.759835787, 0.002843095),
   vec3(-0.97084558, -0.83306509, -0.455136662),
   vec3(0.91611755, -0.02378203, -0.154891865),
   vec3(0.817228621, 0.79555943, -0.523961768),
   vec3(0.141142459, 0.542925226, 0.639137862),
   vec3(-0.1055696, 0.890560873, 0.221190661),
   vec3(0.57724973, -0.190142015, -0.511892203),
   vec3(0.887349108, 0.334576464, -0.504198881),
   vec3(0.762044922, 0.742598864, -0.942781713)
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


float CalculateDirectionalShadow(const vec4 fragPosLightSpace, const vec3 normal, const vec3 lightDir) {
   vec3 shadowCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

   // note: Pikzel uses the Vulkan convention where NDC is -1 to 1 for x and y, and 0 to 1 for z
   //       Transform shadowCoords x and y to 0 to 1 here.  z is OK as is.
   shadowCoords = shadowCoords * vec3(0.5, 0.5, 1.0) + vec3(0.5, 0.5, 0.0);
   shadowCoords.z = shadowCoords.z > 1.0? 0.0 : shadowCoords.z;

   const float lightSize = directionalLight.light.size;
   const float bias = 0.001 * (1.0 - dot(-normal, lightDir));
   const float blockerDepth = BlockerDepth(shadowCoords, bias, lightSize);
   if (blockerDepth > 0.0) {
      const float penumbraWidth = lightSize * (shadowCoords.z - blockerDepth) / blockerDepth;
      return PCFDirectionalShadow(shadowCoords, bias, penumbraWidth);
   }
   return 0.0;
}


vec3 CalculateDirectionalLight(const DirectionalLight light, const vec3 viewDir, const vec3 normal, const vec3 diffuseColor, const vec3 specularColor) {
   const vec3 lightDir = normalize(-inTangentLightDir);

   const float diffuse = max(dot(normal, lightDir), 0.0);
   if(diffuse > 0.0) {
      const float specular = BlinnPhong(lightDir, viewDir, normal, 0.125 /*specularColor.r*/ * maxShininess);       // shininess in specularmap green channel
      const float notShadowed = 1.0 - CalculateDirectionalShadow(inFragPosLightSpace, normal, lightDir);

      return diffuseColor * (light.ambient + (light.color * diffuse * notShadowed)) +
         specularColor.r * (light.color * specular * notShadowed)                                         // specularity in specularmap red channel
      ;
   }
   return diffuseColor * light.ambient;
}


float BlockerDepthPt(const uint lightIndex, const vec3 fragPos, const float fragDepth, const vec3 lightPos, const float lightSize) {
   float numBlockers = 0.0;
   float sumBlockers = 0.0;
   for (int i = 0; i < numPCSSSamples; ++i) {
      const vec3 fragToLight = (fragPos + poissonSphere[i] * lightSize) - lightPos;
      const float z = texture(ptShadowMap, vec4(fragToLight, lightIndex)).r;
      if(z < fragDepth) {
         numBlockers += 1.0;
         sumBlockers += z;
      }
   }
   return numBlockers > 0.0 ? sumBlockers / numBlockers : -1.0;
}


float PCFPointShadow(const uint lightIndex, const vec3 fragPos, const float fragDepth, const vec3 lightPos, const float radius) {
   float sum = 0;
   for (int i = 0; i < numPCFSamples; ++i) {
      const vec3 fragToLight = (fragPos + poissonSphere[i] * radius) - lightPos;
      const float z = texture(ptShadowMap, vec4(fragToLight, lightIndex)).r;
      if(z < fragDepth) {
         ++sum;
      }
   }
   return sum / numPCFSamples;
}


float CalculatePointShadow(const uint lightIndex, const vec3 fragPos, const vec3 lightPos) {
   const float bias = 0.001;
   const float lightSize = pointLights.light[lightIndex].size;
   const vec3 fragToLight = fragPos - lightPos;
   const float fragDepth = (length(fragToLight) / constants.lightRadius) - bias;

   const float blockerDepth = BlockerDepthPt(lightIndex, fragPos, fragDepth, lightPos, lightSize);
   if (blockerDepth > 0.0) {
      const float penumbraWidth = lightSize * (fragDepth - blockerDepth) / blockerDepth;
      return PCFPointShadow(lightIndex, fragPos, fragDepth, lightPos, penumbraWidth);
   }
   return 0.0;
}


vec3 CalculatePointLight(const uint lightIndex, const vec3 viewDir, const vec3 normal, const vec3 diffuseColor, const vec3 specularColor) {
   const vec3 lightPos = pointLights.light[lightIndex].position;
   const vec3 lightDir = normalize(inTangentLightPos[lightIndex] - inTangentFragPos);

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
   const vec3 viewDir = normalize(inTangentViewPos - inTangentFragPos);
   const vec4 diffuseColor = texture(diffuseMap, inTexCoords);

   if (diffuseColor.a < 0.1) {
      discard;
   }

   const vec3 specularColor = texture(specularMap, inTexCoords).rgb;
   const vec3 normal = normalize(texture(normalMap, inTexCoords).xyz * 2.0 - 1.0);

   vec3 color = CalculateDirectionalLight(directionalLight.light, viewDir, normal, diffuseColor.rgb, specularColor);

   for(uint i = 0; i < constants.numPointLights; ++i) {
      color += CalculatePointLight(i, viewDir, normal, diffuseColor.rgb, specularColor);
   }
   outFragColor = vec4(color, diffuseColor.a);
}
