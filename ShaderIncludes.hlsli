#ifndef __GGP_SHADER_INCLUDES__
#define __GGP_SHADER_INCLUDES__

#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2
#define MAX_SPECULAR_EXPONENT 256.0f
#define PI 3.14159265f
#define MIN_ROUGHNESS 0.0000001f
#define F0_NON_METAL 0.04f

struct Light
{
    int Type;
    float3 Direction;
    float Range;
    float3 Position;
    float Intensity;
    float3 Color;
    float SpotInnerAngle;
    float SpotOuterAngle;
    float2 Padding;
};

// Struct representing a single vertex worth of data
// - This should match the vertex definition in our C++ code
// - By "match", I mean the size, order and number of members
// - The name of the struct itself is unimportant, but should be descriptive
// - Each variable must have a semantic, which defines its usage
struct VertexShaderInput
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
    float3 localPosition : POSITION; // XYZ position
    float2 UV : TEXCOORD; // UV coordinates
    float3 Normal : NORMAL; // Surface normal
    float3 Tangent : TANGENT;
};

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
    float4 screenPosition   : SV_POSITION; // XYZW position (System Value Position)
    float2 UV               : TEXCOORD; // UV coordinates
    float3 Normal           : NORMAL; // Surface normal
    float3 worldPosition    : POSITION; // World space position
    float3 Tangent          : TANGENT; // Texture tangent vector
};

struct SkyboxVertexToPixel
{
    float4 screenPosition : SV_POSITION;
    float3 sampleDir : DIRECTION;
};

float random(float2 s)
{
    return frac(sin(dot(s, float2(12.9898, 78.233))) * 43758.5453123);
}

float Attenuate(Light light, float3 worldPos)
{
    float dist = distance(light.Position, worldPos);
    float att = saturate(1.0f - (dist * dist / (light.Range * light.Range)));
    return att * att;
}

// Raises a color float to a given power. Returns the result with w set to 1.
float4 GammaCorrect(float4 color, float gamma)
{
    return float4(pow(color, gamma).rgb, 1);
}

// Corrected Lambert diffuse BRDF
float DiffusePBR(float3 n, float3 l)
{
    return saturate(dot(n, l)) / PI;
}

// Normal Distribution - Trowbridge-Reitz (GGX)
float D_GGX(float3 n, float3 h, float roughness)
{
    // Pre-calculations
    float NdotH = saturate(dot(n, h));
    float NdotH2 = NdotH * NdotH;
    float a = roughness * roughness; // Remapping roughness
    float a2 = max(a * a, MIN_ROUGHNESS);
    
    // Denominator to be squared is ((n dot h)^2 * (a^2 - 1) + 1
    float denomToSquare = NdotH2 * (a2 - 1) + 1;
    return a2 / (PI * denomToSquare * denomToSquare);
}

// Geometric Shadowing - Schlick-GGX
float G_SchlickGGX(float3 n, float3 v, float roughness)
{
    float k = pow(roughness + 1, 2) / 8.0f; // End result of remaps
    float NdotV = saturate(dot(n, v));
    return 1 / (NdotV * (1 - k) + k);
}

// Fresnel - Schlick's Approximation
float3 F_Schlick(float3 v, float3 h, float3 f0)
{
    float VdotH = saturate(dot(v, h));
    return f0 + (1 - f0) * pow(1 - VdotH, 5);
}

// Cook-Torrance Microfacet BRDF
float3 MicrofacetBRDF(float3 n, float3 l, float3 v, float roughness, float3 f0)
{
    float3 h = normalize(v + l);
    
    // Run each function: D and G are scalars, F is a vector
    float  D = D_GGX(n, h, roughness);
    float3 F = F_Schlick(v, h, f0);
    float  G = G_SchlickGGX(n, v, roughness) * G_SchlickGGX(n, l, roughness);
    
    // Final formula
    return (D * F * G) / 4 * saturate(dot(n, l));
}

// Ensures that the object doesn't reflect more light than what hits it
float3 DiffuseEnergyConserve(float3 diffuse, float3 F, float metalness)
{
    return diffuse * (1 - F) * (1 - metalness);
}
#endif