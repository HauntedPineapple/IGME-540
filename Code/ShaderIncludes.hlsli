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
    float3 tangent : TANGENT;
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
    float2 uv : TEXCOORD;
    float3 worldPosition : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float4 shadowMapPos : SHADOWPOS;
};

struct VertexToSkyPixel
{
    float4 screenPosition : SV_POSITION;
    float3 sampleDir : DIRECTION;
};

struct Light
{
    int type; // Which kind of light? 0, 1 or 2
    float3 direction; // Directional and Spot lights need a direction
    float range; // Point and Spot lights have a max range for attenuation
    float3 position; // Point and Spot lights have a position in space
    float intensity; // All lights need an intensity
    float3 color; // All lights need a color
    float spotFalloff; // Spot lights need a value to define their �cone� size
    float3 padding; // Purposefully padding to hit the 16-byte boundary
};

#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT	   1
#define LIGHT_TYPE_SPOT		   2

#define MAX_SPECULAR_EXPONENT 256.0f

// =================== CONSTANTS ===================
// The fresnel value for non-metals (dielectrics)
// Page 9: "F0 of nonmetals is now a constant 0.04"
// http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
static const float F0_NON_METAL = 0.04f;
// Minimum roughness for when spec distribution function denominator goes to zero
static const float MIN_ROUGHNESS = 0.0000001f; // 6 zeros after decimal
static const float PI = 3.14159265359f;

float Random(float2 s)
{
    return frac(sin(dot(s, float2(12, 75))) * 43758);
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

float3 DirectionalLight(Light light, float3 surfaceColor, float3 normal, float3 cameraPosition, float3 worldPosition, float roughness, float specularScale = 1)
{
    float3 viewVector = normalize(cameraPosition - worldPosition); // vector from surface to camera
    float3 directionToLight = normalize(-light.direction);
    
    float diffuse = DiffuseBRDF(normal, directionToLight) * surfaceColor;
    float specular = SpecularBRDF(normal, -directionToLight, viewVector, roughness) * specularScale;
    specular *= any(diffuse);
        
    float3 lightColor = DiffuseBRDF(normal, directionToLight) * surfaceColor;
    lightColor += specular;

    return lightColor * light.color * light.intensity;
}

float3 PointLight(Light light, float3 surfaceColor, float3 normal, float3 cameraPosition, float3 worldPosition, float roughness, float specularScale = 1)
{
    float3 viewVector = normalize(cameraPosition - worldPosition); // vector from surface to camera
    float3 directionToLight = normalize(light.position - worldPosition);

    float attenuate = Attenuate(light, worldPosition);
    float3 lightColor = DiffuseBRDF(normal, directionToLight) * surfaceColor;
    lightColor += SpecularBRDF(normal, -directionToLight, viewVector, roughness) * specularScale;
    
    return lightColor * attenuate * light.color * light.intensity;
}

float3 SpotLight(Light light, float3 surfaceColor, float3 normal, float3 cameraPosition, float3 worldPosition, float roughness, float specularScale = 1)
{
    float3 toLight = normalize(light.position - worldPosition);
    float penumbra = pow(saturate(dot(-toLight, light.direction)), light.spotFalloff);

    // Combine with the point light calculation
    return PointLight(light, surfaceColor, normal, cameraPosition, worldPosition, roughness, specularScale) * penumbra;
}

// ================ PBR FUNCTIONS ================
// Normal Distribution Function: GGX (Trowbridge-Reitz)
// D(n, h, a) = a^2 / pi * ((n dot h)^2 * (a^2 - 1) + 1)^2
float D_GGX(float3 a_normal, float3 a_halfwayVector, float a_roughness)
{
    float NdotH = saturate(dot(a_normal, a_halfwayVector));
    float NdotH2 = NdotH * NdotH;
    float a = a_roughness * a_roughness;
    float a2 = max(a * a, MIN_ROUGHNESS); // Applied after remap!

    // ((n dot h)^2 * (a^2 - 1) + 1)
    // Can go to zero if roughness is 0 and NdotH is 1
    float denomToSquare = NdotH2 * (a2 - 1) + 1;

    // Final value
    return a2 / (PI * denomToSquare * denomToSquare);
}

// Fresnel term - Schlick approx.
// f0 - Value when l = n
// F(v,h,f0) = f0 + (1-f0)(1 - (v dot h))^5
float3 F_Schlick(float3 a_toCamera, float3 a_halfwayVector, float3 a_specularColor)
{
    float VdotH = saturate(dot(a_toCamera, a_halfwayVector));
    return a_specularColor + (1 - a_specularColor) * pow(1 - VdotH, 5);
}

// Geometric Shadowing - Schlick-GGX
// - k is remapped to a / 2, roughness remapped to (r+1)/2 before squaring!
// G_Schlick(n,v,a) = (n dot v) / ((n dot v) * (1 - k) * k)
// G(n,v,l,a) = G_SchlickGGX(n,v,a) * G_SchlickGGX(n,l,a)
float G_SchlickGGX(float3 a_normal, float3 a_vector, float a_roughness)
{
    float k = pow(a_roughness + 1, 2) / 8.0f;
    float NdotV = saturate(dot(a_normal, a_vector));
    return 1 / (NdotV * (1 - k) + k);
}

// Cook-Torrance Microfacet BRDF (Specular)
// f(l,v) = D(h)F(v,h)G(l,v,h) / 4(n dot l)(n dot v)
float3 MicrofacetBRDF(float3 a_normal, float3 a_toLight, float3 a_toCamera, float a_roughness, float3 a_specularColor, out float3 F_out)
{
    float3 halfwayVector = normalize(a_toCamera + a_toLight);
    
    float D = D_GGX(a_normal, halfwayVector, a_roughness);
    float3 F = F_out = F_Schlick(a_toCamera, halfwayVector, a_specularColor);
    float G = G_SchlickGGX(a_normal, a_toCamera, a_roughness) * G_SchlickGGX(a_normal, a_toLight, a_roughness);
    
    float3 specularResult = (D * F * G) / 4;
    return specularResult * max(dot(a_normal, a_toLight), 0);
}

// Calculates diffuse amount based on energy conservation
// F - Fresnel result from microfacet BRDF
float3 DiffuseEnergyConserve(float3 diffuse, float3 F, float metalness)
{
    return diffuse * (1 - F) * (1 - metalness);
}

float3 DirectionalLightPBR(Light light, float3 surfaceColor, float3 normal, float3 cameraPosition, float3 worldPosition, float roughness, float metalness, float3 specularColor)
{
    float3 viewVector = normalize(cameraPosition - worldPosition); // vector from surface to camera
    float3 directionToLight = normalize(-light.direction);
    
    // Calculate the light amounts
    float diff = DiffuseBRDF(normal, directionToLight);
    float3 F;
    float3 specular = MicrofacetBRDF(normal, directionToLight, viewVector, roughness, specularColor, F);
    // Calculate diffuse with energy conservation, including cutting diffuse for metals
    float3 balancedDiff = DiffuseEnergyConserve(diff, F, metalness);
    // Combine the final diffuse and specular values for this light
    return (balancedDiff * surfaceColor + specular) * light.intensity * light.color;
}

float3 PointLightPBR(Light light, float3 surfaceColor, float3 normal, float3 cameraPosition, float3 worldPosition, float roughness, float metalness, float3 specularColor)
{
    return Attenuate(light, worldPosition) * DirectionalLightPBR(light, surfaceColor, normal, cameraPosition, worldPosition, roughness, metalness, specularColor);
}

float3 SpotLightPBR(Light light, float3 surfaceColor, float3 normal, float3 cameraPosition, float3 worldPosition, float roughness, float metalness, float3 specularColor)
{
    float3 toLight = normalize(light.position - worldPosition);
    float penumbra = pow(saturate(dot(-toLight, light.direction)), light.spotFalloff);

    // Combine with the point light calculation
    return PointLightPBR(light, surfaceColor, normal, cameraPosition, worldPosition, roughness, metalness, specularColor) * penumbra;
}
#endif