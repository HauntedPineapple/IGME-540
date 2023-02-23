#include "ShaderIncludes.hlsli"

cbuffer ExternalData : register(b0)
{
    float roughness;
    float3 cameraPosition;
    float3 ambientColor;
    float4 colorTint;
    Light directionalLightA;
}

float SpecularBRDF(float3 normal, float3 lightDir, float3 viewVector)
{
    float3 refl = reflect(lightDir, normal);
    
    // Compare reflecion against view vector    
    float specular = saturate(dot(refl, viewVector));
    // Raising it to a very high power to ensure falloff to zero is quick
    specular = pow(specular, MAX_SPECULAR_EXPONENT);
    
    return specular;
}

float4 main(VertexToPixel input) : SV_TARGET
{    
    input.normal = normalize(input.normal); // Must renormalize any interpolated vectors
    float3 viewVector = normalize(cameraPosition - input.worldPosition); // vector from surface to camera
    float3 directionToLight = normalize(-directionalLightA.direction);
    
    float3 totalLightColor = ambientColor;
    totalLightColor += DiffuseBRDF(input.normal, directionToLight) * directionalLightA.color;
    //totalLightColor += SpecularBRDF(input.normal, directionalLightA.direction, viewVector) * directionalLightA.color;

    return float4((float3) colorTint * totalLightColor, 1);
}