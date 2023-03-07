#ifndef __GGP_SHADER_INCLUDES__ // Each .hlsli file needs a unique identifier!
#define __GGP_SHADER_INCLUDES__

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
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
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
    float4 screenPosition : SV_POSITION;
    float3 worldPosition : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

float Random(float2 s)
{
    return frac(sin(dot(s, float2(12, 75))) * 43758);
}

#define MAX_SPECULAR_EXPONENT 256.0f

#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT	   1
#define LIGHT_TYPE_SPOT		   2

struct Light
{
    int type; // Which kind of light? 0, 1 or 2
    float3 direction; // Directional and Spot lights need a direction
    float range; // Point and Spot lights have a max range for attenuation
    float3 position; // Point and Spot lights have a position in space
    float intensity; // All lights need an intensity
    float3 color; // All lights need a color
    float spotFalloff; // Spot lights need a value to define their “cone” size
    float3 padding; // Purposefully padding to hit the 16-byte boundary
};

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

float3 DirectionalLight(Light light, float3 surfaceColor, float3 normal, float3 cameraPosition, float3 worldPosition, float roughness, float specularScale=1)
{
    float3 viewVector = normalize(cameraPosition - worldPosition); // vector from surface to camera
    float3 directionToLight = normalize(-light.direction);

    float diffuse = DiffuseBRDF(normal, directionToLight);
    float specular = SpecularBRDF(normal, -directionToLight, viewVector, roughness) * specularScale;

    //return lightColor * light.color;
    return (diffuse * surfaceColor + specular) * light.color;
}

float3 PointLight(Light light, float3 surfaceColor, float3 normal, float3 cameraPosition, float3 worldPosition, float roughness, float specularScale=1)
{
    float3 viewVector = normalize(cameraPosition - worldPosition); // vector from surface to camera
    float3 directionToLight = normalize(light.position - worldPosition);
    
    //float lightColor = DiffuseBRDF(normal, directionToLight) * surfaceColor;
    //lightColor += SpecularBRDF(normal, -directionToLight, viewVector, roughness);
    float attenuate = Attenuate(light, worldPosition);
    float diffuse = DiffuseBRDF(normal, directionToLight);
    float specular = SpecularBRDF(normal, -directionToLight, viewVector, roughness) * specularScale;
    
    //return (lightColor * light.color) * attenuate;
    return (diffuse * surfaceColor + specular) * attenuate * light.color;
}

float3 SpotLight(Light light, float3 surfaceColor, float3 normal, float3 cameraPosition, float3 worldPosition, float roughness, float specularScale=1)
{
    return 0;
}
#endif