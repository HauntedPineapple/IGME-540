#include "ShaderIncludes.hlsli"

cbuffer ExternalData : register(b0)
{
    matrix view;
    matrix projection;
}

float4 main(float4 pos : POSITION) : SV_POSITION
{
    return pos;
}