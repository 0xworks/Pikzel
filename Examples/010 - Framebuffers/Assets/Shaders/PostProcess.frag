#version 450 core
layout (location = 0) in vec2 inTexCoord;

layout(push_constant) uniform PC {
   int postprocess;
   float offset;
} constants;

layout (location = 0) out vec4 outFragColor;

layout(binding = 0) uniform sampler2D uTexture;


void main() {
   const vec2 offsets[9] = vec2[](
      vec2(-constants.offset,  constants.offset), // top-left
      vec2( 0.0f,              constants.offset), // top-center
      vec2( constants.offset,  constants.offset), // top-right
      vec2(-constants.offset,  0.0f),             // center-left
      vec2( 0.0f,              0.0f),             // center-center
      vec2( constants.offset,  0.0f),             // center-right
      vec2(-constants.offset, -constants.offset), // bottom-left
      vec2( 0.0f,             -constants.offset), // bottom-center
      vec2( constants.offset, -constants.offset)  // bottom-right    
   );

   const float kernelSharpen[9] = float[](
      -1, -1, -1,
      -1,  9, -1,
      -1, -1, -1
   );

   const float kernelBlur[9] = float[](
      1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0,
      2.0 / 16.0, 4.0 / 16.0, 2.0 / 16.0,
      1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0
   );

   if(constants.postprocess == 0) {
      outFragColor = texture(uTexture, inTexCoord);
   } else if(constants.postprocess == 1) {
      outFragColor = vec4(vec3(1.0) - texture(uTexture, inTexCoord).rgb, 1.0);
   } else if(constants.postprocess == 2) {
       outFragColor = texture(uTexture, inTexCoord);
       float average = dot(vec3(0.2126, 0.7152, 0.0722), outFragColor.rgb);
       outFragColor = vec4(average, average, average, 1.0);
   } else if(constants.postprocess == 3) {
      vec3 sampleTex[9];
      for(int i = 0; i < 9; i++) {
         sampleTex[i] = vec3(texture(uTexture, inTexCoord + offsets[i]));
      }
      vec3 col = vec3(0.0);
      for(int i = 0; i < 9; i++) {
         col += sampleTex[i] * kernelSharpen[i];
      }
      outFragColor = vec4(col, 1.0);
   } else if(constants.postprocess == 4) {
      vec3 sampleTex[9];
      for(int i = 0; i < 9; i++) {
         sampleTex[i] = vec3(texture(uTexture, inTexCoord + offsets[i]));
      }
      vec3 col = vec3(0.0);
      for(int i = 0; i < 9; i++) {
         col += sampleTex[i] * kernelBlur[i];
      }
      outFragColor = vec4(col, 1.0);
   } else {
      outFragColor = vec4(0.0);
   }
}
