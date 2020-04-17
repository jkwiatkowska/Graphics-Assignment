#include "Common.hlsli" // Shaders can also use include files - note the extension

//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------
Texture2D DiffuseSpecularMap : register(t0);
Texture2D DiffuseSpecularMap2 : register(t4);

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
	const float minPos = 0.5f;
	const float maxPos = 20;
	float textureValue;
	if (input.worldPosition.y <= minPos) textureValue = 0;
	else if (input.worldPosition.y >= maxPos) textureValue = 1;
	else textureValue = (input.worldPosition.y - minPos) / (maxPos - minPos);

	float4 texture1Colour = DiffuseSpecularMap.Sample(TexSampler, input.uv);
	float4 texture2Colour = DiffuseSpecularMap2.Sample(TexSampler, input.uv);
	float4 textureColour = texture1Colour * textureValue + texture2Colour * (1 - textureValue);

	float3 diffuseMaterialColour = textureColour.rgb;

	// Combine lighting with texture colours
	float3 finalColour = diffuseLight * diffuseMaterialColour; // Specular light not used because the shader is used for things like grass and ground

	return float4(finalColour, 1.0f);
}