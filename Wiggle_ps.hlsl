#include "Common.hlsli" // Shaders can also use include files - note the extension

//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------
Texture2D DiffuseSpecularMap : register(t0); // Textures here can contain a diffuse map (main colour) in their rgb channels and a specular map (shininess) in the a channel
SamplerState TexSampler      : register(s0); // A sampler is a filter for a texture like bilinear, trilinear or anisotropic - this is the sampler used for the texture above

SamplerState PointClamp   : register(s1); // No filtering for shadow maps (you might think you could use trilinear or similar, but it will filter light depths not the shadows cast...)

Texture2D ShadowMap[15] : register(t2);

//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------
float4 main(LightingPixelShaderInput input) : SV_Target
{
	// Slight adjustment to calculated depth of pixels so they don't shadow themselves
	const float DepthAdjust = 0.0005f;

	// Normal might have been scaled by model scaling or interpolation so renormalise
	input.worldNormal = normalize(input.worldNormal);

	///////////////////////
	// Calculate lighting
	float3 diffuseLight;
	float3 specularLight;
	CalculateLighting(ShadowMap, input.worldPosition, input.worldNormal, PointClamp, diffuseLight, specularLight);

	// Scrolling effect
	//input.uv.y += gWiggle;

	////////////////////
	// Combine lighting and textures

	// Sample diffuse material and specular material colour for this pixel from a texture using a given sampler that you set up in the C++ code
	float4 textureColour = DiffuseSpecularMap.Sample(TexSampler, input.uv);
	float3 diffuseMaterialColour = textureColour.rgb; // Diffuse material colour in texture RGB (base colour of model)
	float specularMaterialColour = textureColour.a;   // Specular material colour in texture A (shininess of the surface)

	float3 tint = 0;
	tint.x = 0.6f;
	tint.z = 0.45f;

	// Combine lighting with texture colours
	float3 finalColour = diffuseLight * (diffuseMaterialColour * 0.4f + tint) + specularLight * specularMaterialColour;

	return float4(finalColour, 1.0f); // Always use 1.0f for output alpha - no alpha blending in this lab
}