// Custom pixel shader.

// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
    float4 screenPosition : SV_POSITION; // XYZW position (System Value Position)
    float2 UV : TEXCOORD; // UV coordinates
    float3 Normal : NORMAL; // Surface normal
};

cbuffer ExternalData : register(b0)
{
    float4 colorTint;
    float2 textureScale;
    float2 textureOffset;
    float totalTime;
}

float random(float2 s)
{
    return frac(sin(dot(s, float2(12.9898, 78.233))) * 43758.5453123);
}

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
    float red = sin(input.Normal.r) * sin(totalTime) / 2 + random(input.UV) / 2;
    float green = cos(input.Normal.g) * cos(totalTime) / 2 + random(input.UV) / 2;
    float blue = (red + green) / 2 + 0.5;
    
    return float4(red, green, blue, 1);
}