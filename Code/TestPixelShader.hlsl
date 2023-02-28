#include "ShaderIncludes.hlsli"

cbuffer ExternalData : register(b0)
{
    float roughness;
    float3 cameraPosition;
    float3 ambientColor;
    float4 colorTint;
    Light directionalLightA;
}
float DiffuseBRDF(float3 normal, float3 dirToLight)
{
    return saturate(dot(normal, dirToLight));
}


float SpecularBRDF(float3 normal, float3 lightDir, float3 viewVector, float roughness)
{
    float specExponent = (1.0f - roughness) * MAX_SPECULAR_EXPONENT;
    if (specExponent > 0.05)
    {
        float reflection = reflect(normalize(lightDir), normal);
        float specular = saturate(dot(reflection, viewVector));
        return pow(specular, specExponent);
    }
    
    return 0.0f;
}

float4 main(VertexToPixel input) : SV_TARGET
{
    input.normal = normalize(input.normal); // Must renormalize any interpolated vectors
    float3 directionToLight = normalize(-directionalLightA.direction);
    float3 viewVector = normalize(cameraPosition - input.worldPosition); // vector from surface to camera
    
    float3 totalLightColor = ambientColor * colorTint;
    totalLightColor += DiffuseBRDF(input.normal, directionToLight) * directionalLightA.color;
    //totalLightColor += SpecularBRDF(input.normal, directionalLightA.direction, viewVector, roughness);

    return float4((float3)totalLightColor, 1);
}