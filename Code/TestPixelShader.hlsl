#include "ShaderIncludes.hlsli"

cbuffer ExternalData : register(b0)
{
    float roughness;
    float3 cameraPosition;
    float3 ambientColor;
    float3 colorTint;
    Light directionalLightA;
}
float DiffuseBRDF(float3 normal, float3 dirToLight)
{
    return saturate(dot(normal, dirToLight));
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

float Attenuate(Light light, float3 worldPos)
{
    float dist = distance(light.position, worldPos);
    float att = saturate(1.0f - (dist * dist / (light.range * light.range)));
    return att * att;
}

float3 DirectionalLight(Light light, float3 normal)
{
    return 0;
}

float3 PointLight(Light light)
{
    return 0;
}

float4 main(VertexToPixel input) : SV_TARGET
{
    input.normal = normalize(input.normal); // Must renormalize any interpolated vectors
    float3 directionToLight = normalize(-directionalLightA.direction);
    float3 viewVector = normalize(cameraPosition - input.worldPosition); // vector from surface to camera
    
    float3 finalPixelColor = ambientColor * colorTint;
    finalPixelColor += DiffuseBRDF(input.normal, directionToLight) * directionalLightA.color;
    finalPixelColor += SpecularBRDF(input.normal, normalize(directionalLightA.direction), viewVector, roughness);

    return float4(finalPixelColor, 1);
}