#include "ShaderIncludes.hlsli"

cbuffer ExternalData : register(b0)
{
    Light lights[10];
    unsigned int albedoMapIndex;
    unsigned int normalMapIndex;
    unsigned int metalnessIndex;
    unsigned int roughnessIndex;
    float2 uvScale;
    float2 uvOffset;
    float3 colorTint;
    float padding;
    float3 cameraPos;
}

SamplerState BasicSampler : register(s0);

struct PS_Output
{
    float4 color            : SV_TARGET0; // MRT index 0
    float4 sunVisibility    : SV_TARGET1; // MRT index 1
    float4 normals          : SV_TARGET2; // MRT index 2... and so on
    float depth             : SV_TARGET3;
};

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
PS_Output main(VertexToPixel input) // Semantics are now handled in the PS_Output struct, so no semantic here!
{
    // Get bindless textures using ResourceDescriptorHeap (intellisense does not understand this; it's fine)
    Texture2D AlbedoMap = ResourceDescriptorHeap[albedoMapIndex];
    Texture2D NormalMap = ResourceDescriptorHeap[normalMapIndex];
    Texture2D Metalness = ResourceDescriptorHeap[metalnessIndex];
    Texture2D Roughness = ResourceDescriptorHeap[roughnessIndex];
    
    // (Ortho)normalize vectors as necessary
    input.Normal = normalize(input.Normal);
    input.Tangent = normalize(input.Tangent - dot(input.Tangent, input.Normal) * input.Normal);
    
    // Modify UV coords
    input.UV = input.UV * uvScale + uvOffset;
    
    // Calculate bitangent and create TBN matrix
    float3 Bitangent = normalize(cross(input.Tangent, input.Normal));
    float3x3 TBN = float3x3(input.Tangent, Bitangent, input.Normal);
    
    // Sample albedo color and gamma correct it
    float4 albedoColor = GammaCorrect(AlbedoMap.Sample(BasicSampler, input.UV), 2.2);
    
    // Sample normal map, unpack it, and transform it from tangent space to world space with TBN matrix
    float3 finalNormal = normalize(mul(normalize(NormalMap.Sample(BasicSampler, input.UV) * 2 - 1).xyz, TBN));
    
    // Sample metal and roughness maps
    float metalness = Metalness.Sample(BasicSampler, input.UV).r;
    float roughness = Roughness.Sample(BasicSampler, input.UV).r;
    
    // Calculate unit vector to camera
    float3 dirToCamera = normalize(cameraPos - input.worldPos);
    
    // Calculate specular color
    float3 f0 = lerp(F0_NON_METAL, albedoColor.rgb, metalness);
    
    // Variable to hold sum of lighting calculations
    float3 lightTotal = float3(0.0f, 0.0f, 0.0f);
    
    // Loop for each light
    for (int i = 0; i < 10; i++)
    {
        Light light = lights[i]; // Assign current light to a variable for easy access
        
        // If light has no data, skip it
        if (light.Type == LIGHT_TYPE_NONE) continue;
        
        light.Direction = normalize(light.Direction); // Normalize light's direction, if it has one
        
        float3 dirToLight = -light.Direction; // This will be replaced if light is Point or Spot; done to prevent "dirToLight potentially uninitialized" warning
        float attenuation = 1; // Default to 100% intensity
        
        // Calculate dirToLight and attenuation for Point and Spot lights
        if (light.Type == LIGHT_TYPE_POINT || light.Type == LIGHT_TYPE_SPOT)
        {
            dirToLight = normalize(light.Position - input.worldPos); // Normalized direction to light in world space
            attenuation = Attenuate(light, input.worldPos); // Attenuation is only relevant to lights with a range
        }
        
        // For spot lights ONLY, consider the angle to the light's direction and scale attenuation accordingly
        if (light.Type == LIGHT_TYPE_SPOT)
        {
            float pixelAngle = saturate(dot(-dirToLight, light.Direction));
            float cosOuter = cos(light.SpotOuterAngle);
            float cosInner = cos(light.SpotInnerAngle);
            float falloffRange = cosOuter - cosInner;
            float spotTerm = saturate((cosOuter - pixelAngle) / falloffRange);
            
            attenuation *= spotTerm;
        }
        
        // Prepare variables for BRDF calculations
        float3 h = normalize(dirToCamera + dirToLight); // Half vector between v and h
        
        // Calculate the light amounts
        float diff = DiffusePBR(finalNormal, dirToLight);
        float3 spec = MicrofacetBRDF(finalNormal, dirToLight, dirToCamera, roughness, f0);
        
        // Calculate diffuse with energy conservation, including cutting diffuse for metals
        float3 F = F_Schlick(dirToCamera, h, f0);
        float3 balancedDiff = DiffuseEnergyConserve(diff, F, metalness);
        
        // Combine the final diffuse and specular values for this light
        float3 total = (balancedDiff * albedoColor.rgb + spec) * light.Intensity * light.Color * attenuation;
        
        // Add it to the overall total
        lightTotal = lightTotal + total;
    }
    
    // Lastly, gamma correct the total light
    float4 finalColor = GammaCorrect(float4(lightTotal, 1), 1.0 / 2.2);
    
    // Build output struct and return it
    PS_Output output;
    output.color = finalColor;
    output.sunVisibility = float4(0, 0, 0, 0);
    output.normals = float4(finalNormal * 0.5f + 0.5f, 1); // Normal has to be repacked
    output.depth = input.screenPosition.z;
    return output;
}