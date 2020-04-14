#include "Common.hlsli" // Shaders can also use include files - note the extension

//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------
Texture2D DiffuseSpecularMap : register(t0);
Texture2D NormalMap : register(t1);

Texture2D ShadowMap[15] : register(t10);

SamplerState TexSampler      : register(s0);
SamplerState PointClamp   : register(s1);

//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------
float4 main(NormalMappingPixelShaderInput input) : SV_Target
{
	///////////////////////
	// Normal mapping
	float3 modelNormal = normalize(input.modelNormal);
	float3 modelTangent = normalize(input.modelTangent);

	// Calculate bi-tangent to complete the three axes of tangent space - then create the *inverse* tangent matrix to convert *from*
	// tangent space into model space. This is just a matrix built from the three axes (very advanced note - by default shader matrices
	// are stored as columns rather than in rows as in the C++. This means that this matrix is created "transposed" from what we would
	// expect. However, for a 3x3 rotation matrix the transpose is equal to the inverse, which is just what we require)
	float3 modelBiTangent = cross(modelNormal, modelTangent);
	float3x3 invTangentMatrix = float3x3(modelTangent, modelBiTangent, modelNormal);

	// Get the texture normal from the normal map. The r,g,b pixel values actually store x,y,z components of a normal. However, r,g,b
	// values are stored in the range 0->1, whereas the x, y & z components should be in the range -1->1. So some scaling is needed
	float3 textureNormal = 2.0f * NormalMap.Sample(TexSampler, input.uv).rgb - 1.0f; // Scale from 0->1 to -1->1

	// Now convert the texture normal into model space using the inverse tangent matrix, and then convert into world space using the world
	// matrix. Normalise, because of the effects of texture filtering and in case the world matrix contains scaling
	float3 worldNormal = normalize(mul((float3x3)gWorldMatrix, mul(textureNormal, invTangentMatrix)));

	///////////////////////
	// Calculate lighting
	float3 diffuseLight;
	float3 specularLight;
	CalculateLighting(ShadowMap, input.worldPosition, worldNormal, PointClamp, diffuseLight, specularLight);
	
	////////////////////
	// Combine lighting and textures
	float4 textureColour = DiffuseSpecularMap.Sample(TexSampler, input.uv);
	float3 diffuseMaterialColour = textureColour.rgb;
	float specularMaterialColour = textureColour.a;

	// Combine lighting with texture colours
	float3 finalColour = diffuseLight * diffuseMaterialColour + specularLight * specularMaterialColour;

	return float4(finalColour, 1.0f);
}