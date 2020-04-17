#include "Common.hlsli" // Shaders can also use include files - note the extension

TextureCube CubeMap			 : register(t0);

SamplerState TexSampler      : register(s0);

//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------
float4 main(LightingPixelShaderInput input) : SV_Target
{
	float4 textureColour = CubeMap.Sample(TexSampler, input.worldNormal);

	return float4(textureColour.xyz, 1.0f);
}