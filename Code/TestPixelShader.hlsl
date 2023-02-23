#include "ShaderIncludes.hlsli"

cbuffer ExternalData : register(b0)
{
    float roughness;
    float3 cameraPosition;
    float3 ambientColor;
    float4 colorTint;
    Light directionalLightA;
}

float Diffuse(float3 normal, float3 dirToLight)
{
    return saturate(dot(normal, dirToLight));
}

float4 main(VertexToPixel input) : SV_TARGET
{
    input.normal = normalize(input.normal);
    float3 directionToLight = normalize(-directionalLightA.direction);
    float diffuse = Diffuse(input.normal, directionToLight);
    float3 finalPixelColor = (ambientColor * (float3) colorTint) + (diffuse * directionalLightA.color * (float3) colorTint);
    
    return float4(finalPixelColor, 1);
}