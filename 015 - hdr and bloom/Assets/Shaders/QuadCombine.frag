#version 450 core
layout (location = 0) in vec2 inTexCoords;

layout(push_constant) uniform PC {
   int bloom; 
   int tonemap;
   float exposure;
} constants;

layout(set = 0, binding = 0) uniform sampler2D uTexture;
layout(set = 0, binding = 1) uniform sampler2D uBloom;

layout (location = 0) out vec4 outFragColor;


void main() {
   vec3 color = texture(uTexture, inTexCoords).rgb;
   if(constants.bloom == 1) {
      color += texture(uBloom, inTexCoords).rgb;
   }

   switch(constants.tonemap) {

      // reinhard tone mapping
      case 1: color = color / (color + vec3(1.0));
      break;

      // exposure tone mapping
      case 2: color = vec3(1.0) - exp(-color * constants.exposure);
      break;

   }

   // note: no need for gamma correction as we are using sRGB framebuffers
   outFragColor = vec4(color, 1.0);
}
