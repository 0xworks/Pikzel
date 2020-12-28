#version 450 core
layout(location = 0) in vec2 inTexCoords;

layout(push_constant) uniform PC {
   uint horizontal;
} constants;

layout(set = 0, binding = 0) uniform sampler2D uTexture;

layout(location = 0) out vec4 outFragColor;

const float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);


void main() {
   vec2 texOffset = 1.0 / textureSize(uTexture, 0);
   vec3 result = texture(uTexture, inTexCoords).rgb * weight[0];
   if(constants.horizontal == 1) {
      for(int i = 1; i < 5; ++i) {
         result += texture(uTexture, inTexCoords + vec2(texOffset.x * i, 0.0)).rgb * weight[i];
         result += texture(uTexture, inTexCoords - vec2(texOffset.x * i, 0.0)).rgb * weight[i];
      }
   } else {
      for(int i = 1; i < 5; ++i) {
         result += texture(uTexture, inTexCoords + vec2(0.0, texOffset.y * i)).rgb * weight[i];
         result += texture(uTexture, inTexCoords - vec2(0.0, texOffset.y * i)).rgb * weight[i];
      }
   }
   outFragColor = vec4(result, 1.0);
}
