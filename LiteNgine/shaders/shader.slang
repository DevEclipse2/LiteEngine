
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
    [[vk::location(3)]] float3 worldPos;
};

[shader("vertex")]
VSOutput vertMain(VSInput input) {
    VSOutput output;
    
    float4 worldPos = mul(ubo.model, float4(input.inPosition, 1.0));
    float4 viewPos  = mul(ubo.view, worldPos);
    output.pos      = mul(ubo.proj, viewPos);
    
    output.fragColor = input.inColor;
    output.fragTexCoord = input.inTexCoord;
    output.worldPos = worldPos.xyz;
    output.normal = normalize(mul((float3x3)ubo.model, input.inNormal));
    
    return output;
}

[[vk::binding(1, 0)]]
Sampler2D texture;

[shader("fragment")]
float4 fragMain(VSOutput vertIn) : SV_TARGET {
    float3 N = normalize(vertIn.normal);

    float3 lightDir = normalize(float3(0.5, -1.0, -0.5));
    
    float3 L = normalize(-lightDir);

    float3 ambientLight = float3(0.15, 0.15, 0.2); 
    float diff = max(dot(N, L), 0.0);
    float3 diffuseLight = diff * float3(1.0, 1.0, 0.9); // Warm sunlight color
    float3 viewDir = normalize(float3(0.0, 2.0, -5.0) - vertIn.worldPos); 
    float3 reflectDir = reflect(-L, N);
    
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    float3 specularLight = spec * float3(0.3, 0.3, 0.3); 

    float4 texColor = texture.Sample(vertIn.fragTexCoord);

    float3 finalLighting = ambientLight + diffuseLight + specularLight;
    float3 finalColor = finalLighting * texColor.rgb * vertIn.fragColor;

    return float4(finalColor, texColor.a);
}