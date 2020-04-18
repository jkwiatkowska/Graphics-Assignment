#include "Common.hlsli" // Shaders can also use include files - note the extension

TextureCube CubeMap			 : register(t0);

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
	float4 textureColour = CubeMap.Sample(TexSampler, input.worldNormal);
	float3 diffuseMaterialColour = textureColour.rgb;
	float specularMaterialColour = textureColour.a;

	// Combine lighting with texture colours
	float3 finalColour = lerp(diffuseMaterialColour, diffuseLight * diffuseMaterialColour + specularLight * specularMaterialColour, 0.5f); // Lighting effects decreased

	return float4(finalColour, 1.0f);
}