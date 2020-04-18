#include "Common.hlsli" // Shaders can also use include files - note the extension

TextureCube CubeMap			 : register(t0);
TextureCube CubeMap2		 : register(t4);

SamplerState TexSampler      : register(s0);

//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------
float4 main(LightingPixelShaderInput input) : SV_Target
{
	float a1 = radians((gWiggle/3) % 360);
	float a2 = radians((-gWiggle) % 360);

	float4 texture1Colour = CubeMap.Sample(TexSampler, mul(input.worldNormal, GetRotationMatrixX(a1)));
	float4 texture2Colour = CubeMap2.Sample(TexSampler, mul(input.worldNormal, GetRotationMatrixX(a2)));
	float4 textureColour  = lerp(texture1Colour, texture2Colour, texture2Colour.a);

	return float4(textureColour.xyz, 1.0f);
}