// SrgbTest shaders: positions are already in clip space, and colors are linear with alpha.
// The pipeline blends in linear space, and the sRGB render target encodes the result.

struct VertexInput {
   [[vk::location(0)]] float2 position : POSITION;
   [[vk::location(1)]] float4 color : COLOR;
};

struct VertexOutput {
   float4 position : SV_Position;
   float4 color : COLOR;
};

[shader("vertex")]
VertexOutput VertexMain(VertexInput input) {
   VertexOutput output;
   output.position = float4(input.position, 0.0, 1.0);
   output.color = input.color;
   return output;
}

[shader("fragment")]
float4 FragmentMain(VertexOutput input) : SV_Target {
   return input.color;
}
