
struct VSInput {
    [[vk::location(0)]] float3 inPosition;
    [[vk::location(1)]] float3 inNormal;
    [[vk::location(2)]] float3 inColor;
    [[vk::location(3)]] float2 inTexCoord;
};

struct UniformBuffer {
    float4x4 model;
    float4x4 view;
    float4x4 proj;
};

[[vk::binding(0, 0)]] 
ConstantBuffer<UniformBuffer> ubo;

struct VSOutput
{
    float4 pos : SV_Position;
    
    [[vk::location(0)]] float3 fragColor;
    [[vk::location(1)]] float3 normal;
    [[vk::location(2)]] float2 fragTexCoord;
};

[shader("vertex")]
VSOutput vertMain(VSInput input) {
    VSOutput output;
    
    float4 worldPos = mul(ubo.model, float4(input.inPosition, 1.0));
    float4 viewPos  = mul(ubo.view, worldPos);
    output.pos      = mul(ubo.proj, viewPos);
    
    output.fragColor = input.inColor;
    output.fragTexCoord = input.inTexCoord;
    
    output.normal = normalize(mul((float3x3)ubo.model, input.inNormal));
    
    return output;
}

[[vk::binding(1, 0)]]
Sampler2D texture;

[shader("fragment")]
float4 fragMain(VSOutput vertIn) : SV_TARGET {
    float4 texColor = texture.Sample(vertIn.fragTexCoord);
    
    float3 lightDir = normalize(float3(0.3, 1.0, 0.5));
    
    float diffuse = max(dot(normalize(vertIn.normal), lightDir), 0.0);
    
    float lighting = 0.4 + (diffuse * 0.6);
    
    float3 finalColor = texColor.rgb * vertIn.fragColor * lighting;
    
    return float4(finalColor, texColor.a);
}