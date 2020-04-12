#include "Common.hlsli" // Shaders can also use include files - note the extension

//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------
Texture2D DiffuseSpecularMap : register(t0);
Texture2D DiffuseSpecularMap2 : register(t4);

Texture2D ShadowMap[15] : register(t10);

SamplerState TexSampler      : register(s0);
SamplerState PointClamp   : register(s1);

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
    float4 textureColour = DiffuseSpecularMap.Sample(TexSampler, input.uv);
    float3 diffuseMaterialColour = textureColour.rgb;
    float specularMaterialColour = textureColour.a;

    // Combine lighting with texture colours
    float3 finalColour = diffuseLight * diffuseMaterialColour + specularLight * specularMaterialColour;

    return float4(finalColour, 1.0f);
}