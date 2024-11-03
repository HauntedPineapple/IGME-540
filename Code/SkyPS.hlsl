#include "ShaderIncludes.hlsli"

TextureCube CubeMap : register(t0);
SamplerState BasicSampler : register(s0);

float4 main(VertexToSkyPixel input) : SV_TARGET
{
    return CubeMap.Sample(BasicSampler, input.sampleDir);
}