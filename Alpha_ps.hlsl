#include "Common.hlsli"
//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------
Texture2D    DiffuseMap : register(t0);

SamplerState TexSampler : register(s0);


//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------
float4 main(SimplePixelShaderInput input) : SV_Target
{
    float4 diffuseMapColour = DiffuseMap.Sample(TexSampler, input.uv);

    return diffuseMapColour;
}