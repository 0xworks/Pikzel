// TexturedCubes shaders: transform each cube by its model matrix and the camera, sample its
// texture, and light it with a single directional light.
//
// Resources are grouped by how often they change, and each group has a register space of
// its own. The spaces and registers here must match the pipeline layout in Main.cpp:
//
//   space0  push constants  (b0)      per draw: the cube's model matrix
//   space1  descriptor set  (b0)      per frame: camera and light
//   space2  descriptor set  (t0, s0)  per material: texture and sampler
//
// Matrices follow the engine's row-vector convention (see Renderer/ShaderTypes.h): a
// vector is transformed with mul(vector, matrix), and matrices are composed left to right
// in the order they apply.

struct DrawConstants {
   float4x4 model;
};

struct FrameConstants {
   float4x4 viewProjection;
   // Points towards the light, in world space. Normalised.
   float3 directionToLight;
   // Fraction of the albedo lit even when facing away from the light.
   float ambient;
};

[[vk::push_constant]] ConstantBuffer<DrawConstants> g_Draw : register(b0, space0);
ConstantBuffer<FrameConstants> g_Frame : register(b0, space1);
Texture2D g_Albedo : register(t0, space2);
SamplerState g_Sampler : register(s0, space2);

struct VertexInput {
   [[vk::location(0)]] float3 position : POSITION;
   [[vk::location(1)]] float3 normal : NORMAL;
   [[vk::location(2)]] float2 uv : TEXCOORD0;
};

struct VertexOutput {
   float4 position : SV_Position;
   float3 normal : NORMAL;
   float2 uv : TEXCOORD0;
};

[shader("vertex")]
VertexOutput VertexMain(VertexInput input) {
   const float4 worldPosition = mul(float4(input.position, 1.0), g_Draw.model);

   VertexOutput output;
   output.position = mul(worldPosition, g_Frame.viewProjection);
   // A normal is a direction, so w = 0 drops the translation. Transforming it by the model
   // matrix is only correct for rotations and uniform scales; a non-uniform scale needs the
   // inverse transpose. The rasteriser's interpolation shortens normals, so they are
   // normalised again per fragment.
   output.normal = mul(float4(input.normal, 0.0), g_Draw.model).xyz;
   output.uv = input.uv;
   return output;
}

[shader("fragment")]
float4 FragmentMain(VertexOutput input) : SV_Target {
   // The texture is sRGB, so the sample is already linear: lighting maths on it is valid.
   const float3 albedo = g_Albedo.Sample(g_Sampler, input.uv).rgb;

   // Lambertian diffuse: light received is proportional to the cosine of the angle between
   // the surface normal and the direction to the light.
   const float3 normal = normalize(input.normal);
   const float diffuse = saturate(dot(normal, g_Frame.directionToLight));
   const float lighting = g_Frame.ambient + (1.0 - g_Frame.ambient) * diffuse;

   return float4(albedo * lighting, 1.0);
}
