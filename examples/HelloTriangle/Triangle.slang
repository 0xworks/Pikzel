// HelloTriangle shaders: pass a 2D position and a color through, and let the rasteriser
// interpolate the color across the triangle.
//
// Colors are linear throughout. The render target is sRGB, so the GPU encodes the
// fragment shader's output for display as it writes it; the shader must not do so itself.
//
// The vertex inputs must match the attributes in the pipeline settings (see Main.cpp):
// Vulkan matches them by location, D3D12 by semantic (POSITION, COLOR). The locations are
// spelled out explicitly here rather than relying on Slang's declaration-order assignment,
// to make that contract visible.

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
   // Positions are already in clip space: x and y from -1 to +1, with +y pointing up.
   output.position = float4(input.position, 0.0, 1.0);
   output.color = input.color;
   return output;
}

[shader("fragment")]
float4 FragmentMain(VertexOutput input) : SV_Target {
   return input.color;
}
