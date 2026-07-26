struct VSInput {
    [[vk::location(0)]] float3 inPosition;
    [[vk::location(1)]] float3 inNormal;
    [[vk::location(2)]] float3 inColor;
    [[vk::location(3)]] float2 inTexCoord;
    [[vk::location(4)]] uint4  inBoneIDs;
    [[vk::location(5)]] float4 inBoneWeights;
};

struct UniformBuffer {
    float4x4 model;
    float4x4 view;
    float4x4 proj;
    float4x4 boneMatrices[128]; // Required for skinning!
};

[[vk::binding(0, 0)]] 
ConstantBuffer<UniformBuffer> ubo;

struct VSOutput {
    float4 pos          : SV_Position;

    [[vk::location(0)]] float3 fragColor;
    [[vk::location(1)]] float3 normal;
    [[vk::location(2)]] float2 fragTexCoord;
    [[vk::location(3)]] float3 worldPos;
};
[shader("vertex")]
VSOutput vertSkinned(VSInput input) {
    VSOutput output;
    
  float4x4 boneTransform = 
        ubo.boneMatrices[input.inBoneIDs.x] * input.inBoneWeights.x +
        ubo.boneMatrices[input.inBoneIDs.y] * input.inBoneWeights.y +
        ubo.boneMatrices[input.inBoneIDs.z] * input.inBoneWeights.z +
        ubo.boneMatrices[input.inBoneIDs.w] * input.inBoneWeights.w;

    float4 localPos = mul(boneTransform, float4(input.inPosition, 1.0));
    float4 worldPos = mul(ubo.model, localPos);
    float4 viewPos  = mul(ubo.view, worldPos);
    output.pos      = mul(ubo.proj, viewPos);
    output.worldPos = worldPos.xyz;
    output.fragColor = input.inColor;
    output.fragTexCoord = input.inTexCoord;
    float4 localNormal = mul(boneTransform, float4(input.inNormal, 0.0));
    output.normal = normalize(mul(ubo.model, localNormal).xyz);
    
    return output;
}