#include "ShaderIncludes.hlsli"

cbuffer ExternalData : register(b0)
{
    unsigned int skyTextureIndex;
    float3 sunDir;
}

SamplerState BasicSampler : register(s0);

struct PS_Output
{
    float4 color            : SV_TARGET0;
    float4 sunVisibility    : SV_TARGET1;
};

PS_Output main(SkyboxVertexToPixel input)
{
    TextureCube SkyTexture = ResourceDescriptorHeap[skyTextureIndex];
    float4 finalColor = SkyTexture.Sample(BasicSampler, input.sampleDir);
    float4 sunColor = SkyTexture.Sample(BasicSampler, sunDir);
    
    PS_Output output;
    output.color = finalColor;
    output.sunVisibility = pow(saturate(dot(normalize(sunDir), normalize(input.sampleDir))), 256) * sunColor;
    return output;
};