#define NUM_SAMPLES 100
#define Density 1
#define Weight 0.02
#define Decay 0.99
#define Exposure 1

cbuffer ExternalData
{
    uint AlbedoIndex;
    uint SunVisibilityIndex;
    uint NormalsIndex;
    uint DepthIndex;
    float4 screenLightPos;
};

struct VertexToPixel // Simplified input data for sampling from the texture
{
    float4 Position : SV_POSITION;
    float2 UV       : TEXCOORD0;
};

SamplerState BasicSampler : register(s0);

// Will eventually do sun ray calculations here
float4 main(VertexToPixel input) : SV_TARGET
{
    Texture2D albedoTexture = ResourceDescriptorHeap[AlbedoIndex];
    Texture2D sunVisibilityTexture = ResourceDescriptorHeap[SunVisibilityIndex];
    Texture2D normalsTexture = ResourceDescriptorHeap[NormalsIndex];
    Texture2D depthTexture = ResourceDescriptorHeap[DepthIndex];
    
    float2 UV = input.UV;
    
    // Adapted from https://developer.nvidia.com/gpugems/gpugems3/part-ii-light-and-shadows/chapter-13-volumetric-light-scattering-post-process
    float2 deltaUV = (UV - screenLightPos.xy);
    deltaUV *= 1.0f / NUM_SAMPLES * Density;
    float3 color = albedoTexture.Sample(BasicSampler, input.UV);
    float illuminationDecay = 1.0f;
    for (int i = 0; i < NUM_SAMPLES; i++)
    {
        UV -= deltaUV;
        float3 sample = sunVisibilityTexture.Sample(BasicSampler, UV);
        sample *= illuminationDecay * Weight;
        color += sample;
        illuminationDecay *= Decay;
    }
    return float4(color * Exposure, 1);
}