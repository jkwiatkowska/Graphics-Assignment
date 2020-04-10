//--------------------------------------------------------------------------------------
// Per-Pixel Lighting Pixel Shader
//--------------------------------------------------------------------------------------
// Pixel shader receives position and normal from the vertex shader and uses them to calculate
// lighting per pixel. Also samples a samples a diffuse + specular texture map and combines with light colour.

#include "Common.hlsli" // Shaders can also use include files - note the extension


//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------

// Here we allow the shader access to a texture that has been loaded from the C++ side and stored in GPU memory.
// Note that textures are often called maps (because texture mapping describes wrapping a texture round a mesh).
// Get used to people using the word "texture" and "map" interchangably.
Texture2D DiffuseSpecularMap : register(t0); // Textures here can contain a diffuse map (main colour) in their rgb channels and a specular map (shininess) in the a channel
SamplerState TexSampler      : register(s0); // A sampler is a filter for a texture like bilinear, trilinear or anisotropic - this is the sampler used for the texture above

Texture2D ShadowMapLight[25];
SamplerState PointClamp   : register(s1); // No filtering for shadow maps (you might think you could use trilinear or similar, but it will filter light depths not the shadows cast...)


//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------

// Pixel shader entry point - each shader has a "main" function
// This shader just samples a diffuse texture map
float4 main(LightingPixelShaderInput input) : SV_Target
{
	// Slight adjustment to calculated depth of pixels so they don't shadow themselves
	const float DepthAdjust = 0.0005f;

    // Normal might have been scaled by model scaling or interpolation so renormalise
    input.worldNormal = normalize(input.worldNormal); 

	///////////////////////
	// Calculate lighting
    
    // Direction from pixel to camera
    float3 cameraDirection = normalize(gCameraPosition - input.worldPosition);

	// Lights

	float3 diffuseLight = gAmbientColour;
	float3 specularLight = 0;
	
	for (int i = 0; i < gSpotlightNumber; i++)
	{
		// Direction from pixel to light
		float3 lightDirection = normalize(gSpotlights[i].position - input.worldPosition);

		// Check if pixel is within light cone
		if (dot(gSpotlights[i].facing, -lightDirection) > gSpotlights[i].cosHalfAngle)
		{
			// Using the world position of the current pixel and the matrices of the light (as a camera), find the 2D position of the
			// pixel *as seen from the light*. Will use this to find which part of the shadow map to look at.
			// These are the same as the view / projection matrix multiplies in a vertex shader (can improve performance by putting these lines in vertex shader)
			float4 lightViewPosition = mul(gSpotlights[i].viewMatrix, float4(input.worldPosition, 1.0f));
			float4 lightProjection = mul(gSpotlights[i].projectionMatrix, lightViewPosition);

			// Convert 2D pixel position as viewed from light into texture coordinates for shadow map - an advanced topic related to the projection step
			// Detail: 2D position x & y get perspective divide, then converted from range -1->1 to UV range 0->1. Also flip V axis
			float2 shadowMapUV = 0.5f * lightProjection.xy / lightProjection.w + float2(0.5f, 0.5f);
			shadowMapUV.y = 1.0f - shadowMapUV.y;	// Check if pixel is within light cone

			// Get depth of this pixel if it were visible from the light (another advanced projection step)
			float depthFromLight = lightProjection.z / lightProjection.w;// - DepthAdjust; //*** Adjustment so polygons don't shadow themselves

			// Compare pixel depth from light with depth held in shadow map of the light. If shadow map depth is less than something is nearer
			// to the light than this pixel - so the pixel gets no effect from this light
			if (depthFromLight < ShadowMapLight[i].Sample(PointClamp, shadowMapUV).r)
			{
				float3 lightDist = length(gSpotlights[i].position - input.worldPosition);
				diffuseLight += gSpotlights[i].colour * max(dot(input.worldNormal, lightDirection), 0) / lightDist; 
				float3 halfway = normalize(lightDirection + cameraDirection);
				specularLight += diffuseLight * pow(max(dot(input.worldNormal, halfway), 0), gSpecularPower); 
			}
		}
	}
	
	for (int i = 0; i < gPointlightNumber; i++)
	{
		float3 lightDirection = normalize(gPointlights[i].position - input.worldPosition);

		float3 lightDist = length(gPointlights[i].position - input.worldPosition);
		diffuseLight += gPointlights[i].colour * max(dot(input.worldNormal, lightDirection), 0) / lightDist; 
		float3 halfway = normalize(lightDirection + cameraDirection);
		specularLight += diffuseLight * pow(max(dot(input.worldNormal, halfway), 0), gSpecularPower);
	}

	////////////////////
	// Combine lighting and textures

    // Sample diffuse material and specular material colour for this pixel from a texture using a given sampler that you set up in the C++ code
    float4 textureColour = DiffuseSpecularMap.Sample(TexSampler, input.uv);
    float3 diffuseMaterialColour = textureColour.rgb; // Diffuse material colour in texture RGB (base colour of model)
    float specularMaterialColour = textureColour.a;   // Specular material colour in texture A (shininess of the surface)

    // Combine lighting with texture colours
    float3 finalColour = diffuseLight * diffuseMaterialColour + specularLight * specularMaterialColour;

    return float4(finalColour, 1.0f); // Always use 1.0f for output alpha - no alpha blending in this lab
}