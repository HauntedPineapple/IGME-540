#include "ShaderIncludes.hlsli"

#define NUM_LIGHTS 5

cbuffer ExternalData : register(b0)
{
    float roughness;
    float3 colorTint;
    float3 ambientColor;
    float3 cameraPosition;

    Light directionalLightA;
    
    Light lights[NUM_LIGHTS]; // Array of exactly NUM_LIGHTS lights
}


float Diffuse(float3 normal, float3 dirToLight)
{
    return saturate(dot(normal, dirToLight));
}


float Specular(float3 normal, float3 lightDir, float3 viewVector, float roughness)
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

float Attenuate1(Light light, float3 worldPos)
{
    float dist = distance(light.position, worldPos);
    float att = saturate(1.0f - (dist * dist / (light.range * light.range)));
    return att * att;
}

float3 Directiona(Light light, float3 normal)
{
    return 0;
}

float3 Point(Light light)
{
    return 0;
}


float4 main(VertexToPixel input) : SV_TARGET
{
    input.normal = normalize(input.normal); // Must renormalize any interpolated vectors
    float3 finalPixelColor = ambientColor;
    
    float3 directionToLight = normalize(-directionalLightA.direction);
    float3 viewVector = normalize(cameraPosition - input.worldPosition); // vector from surface to camera
    finalPixelColor += DiffuseBRDF(input.normal, directionToLight) * directionalLightA.color;
    finalPixelColor += SpecularBRDF(input.normal, normalize(directionalLightA.direction), viewVector, roughness);
    
    //for (int i = 0; i < NUM_LIGHTS; i++)
    //{
    //    switch (lights[i].type)
    //    {
    //        case 0: //directional
    //            finalPixelColor += DirectionalLight(lights[i], colorTint, input.normal, cameraPosition, input.worldPosition, roughness);
    //            break;
    //        case 1: //point
    //            finalPixelColor += PointLight(lights[i], colorTint, input.normal, cameraPosition, input.worldPosition, roughness);
    //            break;
    //        case 2: //spot
    //            break;
    //    }
    //}

    return float4(finalPixelColor * colorTint, 1);
}