struct VertexToPixel
{
    float4 position : SV_POSITION;
    float2 UV : TEXCOORD0;
};

cbuffer externalData : register(b0)
{
    int pixelSize;
    float2 windowSize;
}

Texture2D Pixels : register(t0);
SamplerState ClampSampler : register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
    float x = int(input.position.x) % pixelSize;
    float y = int(input.position.y) % pixelSize;
    
    x = floor(pixelSize / 2.0) - x;
    y = floor(pixelSize / 2.0) - y;
    
    x = input.position.x + x;
    y = input.position.y + y;
    
    return Pixels.Sample(ClampSampler, float2(x, y) / windowSize);
}