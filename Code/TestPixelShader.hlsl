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
    return saturate(dot(normal, normalize(dirToLight)));
}


float SpecularBRDF(float3 normal, float3 lightDir, float3 viewVector, float roughness)
{
    float3 refl = reflect(lightDir, normal);
    
    // Compare reflecion against view vector    
    float specular = saturate(dot(refl, viewVector));
    // Raising it to a very high power to ensure falloff to zero is quick
    float specExponent = (1.0f - roughness) * MAX_SPECULAR_EXPONENT;
    if (specExponent > 0.005)
        specular = pow(specular, specExponent);
    else 
        specular = 0.0f;

    return specular;
}

float4 main(VertexToPixel input) : SV_TARGET
{    
    input.normal = normalize(input.normal); // Must renormalize any interpolated vectors
    float3 viewVector = normalize(cameraPosition - input.worldPosition); // vector from surface to camera
    float3 directionToLight = normalize(-directionalLightA.direction);
    
    //float3 totalLightColor = ambientColor;
    //totalLightColor += DiffuseBRDF(input.normal, directionToLight);
    //totalLightColor += SpecularBRDF(input.normal, directionalLightA.direction, viewVector, roughness);

    float diffuse = DiffuseBRDF(input.normal, directionToLight);
    float specular = SpecularBRDF(input.normal, directionalLightA.direction, viewVector, roughness);
    float3 totalLightColor = ambientColor;
    totalLightColor += colorTint * diffuse + specular;
    return float4((float3)colorTint * totalLightColor, 1);
}