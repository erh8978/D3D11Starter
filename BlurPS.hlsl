struct VertexToPixel
{
    float4 position : SV_POSITION;
    float2 UV       : TEXCOORD0;
};

cbuffer externalData : register(b0)
{
    int blurRadius;
    float pixelWidth;
    float pixelHeight;
}

Texture2D Pixels            : register(t0);
SamplerState ClampSampler   : register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
    // Track total color and # of samples
    float4 total = 0;
    int sampleCount = 0;
    
    // Loop through the "box"
    for (int x = -blurRadius; x <= blurRadius; x++)
    {
        for (int y = -blurRadius; y <= blurRadius; y++)
        {
            // Calculate the UV for this sample
            float2 uv = input.UV;
            uv += float2(x * pixelWidth, y * pixelHeight);
            
            // Add this color to the running total
            total += Pixels.Sample(ClampSampler, uv);
            sampleCount++;
        }
    }
    
    return total / sampleCount;
}