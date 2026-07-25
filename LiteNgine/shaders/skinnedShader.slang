struct SkinnedUBO 
{
    float4x4 viewProj;
    float4x4 model;
    float4x4 boneMatrices[128];
};

// Bind the UBO (Set 0, Binding 0)
[[vk::binding(0, 0)]]
ConstantBuffer<SkinnedUBO> ubo;

// Map your C++ skinnedVertex struct
struct VertexInput 
{
    [[vk::location(0)]] float3 position;
    [[vk::location(1)]] float3 normal;
    [[vk::location(2)]] float3 color;
    [[vk::location(3)]] float2 texCoord;
    [[vk::location(4)]] uint4  boneIDs;
    [[vk::location(5)]] float4 boneWeights;
};

struct VertexOutput 
{
    // SV_Position tells the rasterizer this is the final screen coordinate
    float4 position : SV_Position; 
};

[shader("vertex")]
VertexOutput vertSkinned(VertexInput input) 
{
    VertexOutput output;

    //Accumulate the weighted bone matrices
    float4x4 boneTransform = 
        ubo.boneMatrices[input.boneIDs.x] * input.boneWeights.x +
        ubo.boneMatrices[input.boneIDs.y] * input.boneWeights.y +
        ubo.boneMatrices[input.boneIDs.z] * input.boneWeights.z +
        ubo.boneMatrices[input.boneIDs.w] * input.boneWeights.w;

    //Transform the vertex (Slang uses mul() for matrix multiplication)
    float4 localPos = mul(boneTransform, float4(input.position, 1.0));
    float4 worldPos = mul(ubo.model, localPos);
    
    //Project to screen
    output.position = mul(ubo.viewProj, worldPos);

    return output;
}