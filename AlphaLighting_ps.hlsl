#include "Common.hlsli" // Shaders can also use include files - note the extension

//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------
Texture2D DiffuseSpecularMap : register(t0);

Texture2D ShadowMap[15]      : register(t10);

SamplerState TexSampler      : register(s0);
SamplerState PointClamp      : register(s1);

//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------
float4 main(LightingPixelShaderInput input) : SV_Target
{
	input.worldNormal = normalize(input.worldNormal);

	///////////////////////
	// Calculate lighting
	float3 diffuseLight;
	float3 specularLight;
	CalculateLighting(ShadowMap, input.worldPosition, input.worldNormal, PointClamp, diffuseLight, specularLight);

	////////////////////
	// Combine lighting and textures
	float4 finalColour = DiffuseSpecularMap.Sample(TexSampler, input.uv); // Texture colour and alpha
	finalColour.xyz *= diffuseLight;
	finalColour.xyz += specularLight;

	return finalColour;
}