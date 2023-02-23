#include "ShaderIncludes.hlsli"

cbuffer ExternalData : register(b0)
{
    float4 colorTint;
    float3 ambientColor;
    float3 lightColor;
    float3 lightDirection;
}

float4 main(VertexToPixel input) : SV_TARGET
{
    // Compare the light's direction and the surface normal
    float shadingResult = dot(input.normal, -lightDirection);
    float3 totalLightColor = ambientColor + (lightColor * shadingResult);
	
    return float4((float3) colorTint * totalLightColor, 1);
}