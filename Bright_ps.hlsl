#include "Common.hlsli" // Shaders can also use include files - note the extension

//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------
Texture2D DiffuseSpecularMap : register(t0);

Texture2D ShadowMap[15]      : register(t10);
Texture2D ColourMap[15]      : register(t30);

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
	CalculateLighting(ShadowMap, input.worldPosition, input.worldNormal, PointClamp, diffuseLight, specularLight, ColourMap, true);

	////////////////////
	// Combine lighting and textures
	float4 textureColour = DiffuseSpecularMap.Sample(TexSampler, input.uv);
	float3 diffuseMaterialColour = textureColour.rgb;
	float specularMaterialColour = textureColour.a * 0.7f;

	// Increase light's effect
	diffuseLight *= 1.3f;
	if (diffuseLight.r > 1) diffuseLight.r = 1;
	if (diffuseLight.g > 1) diffuseLight.g = 1;
	if (diffuseLight.b > 1) diffuseLight.b = 1;

	float3 tint = 0;
	tint.x = 0.1f;
	tint.z = 0.06f;

	// Combine lighting with texture colours
	float3 finalColour = diffuseLight * (diffuseMaterialColour * 0.9f + tint) + specularLight * specularMaterialColour;

	return float4(finalColour, 1.0f);
}