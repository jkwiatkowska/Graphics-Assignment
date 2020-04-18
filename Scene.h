//--------------------------------------------------------------------------------------
// Scene geometry and layout preparation
// Scene rendering & update
//--------------------------------------------------------------------------------------

#ifndef _SCENE_H_INCLUDED_
#define _SCENE_H_INCLUDED_

class SceneModel;

enum RenderMode
{
	Default,	 	 // Default shaders, receives lighting and casts shadows
	Bright,		 	 // Similar to above, but the light's effects are increased and a tint is added
	Wiggle,		 	 // Vertex deformation, movement up and down, texture scrolling and tint. Casts and receives shadows.
	TextureFade, 	 // Goes from one texture to another and back over time, receives lighting and shadows
	TextureGradient, // Second texture is more visible near the ground, receives lighting and casts shadow
	TexGradientNS,   // Same as above, but doesn't cast shadows 
	NormalMap,		 // Normal mapping, receives lighting and shadows
	ParallaxMap,	 // Same as above but with parallax mapping added
	CubeMap,		 // Object using a cubemap, casts shadows and receives no light
	CubeMapLight,	 // Same as above, but casts shadows
	CubeMapAnimated, // Cube map with two texture layers that rotate around the x axis at different speeds, no shadows/lighting
	AddBlend,		 // Transparent object that uses additive blending, doesn't receive light or shadows
	AddBlendLight,	 // Similar to above, but receives and casts (coloured) shadows
	Ghost,			 // Same as above, but doesn't cast shadows
	MultBlend,		 // Transparent object rendered using multiplicative blending, no lighting
	AlphBlend,		 // Texture transparency is retained, no lighting
	None			 // Assign this to hide an object
};

//--------------------------------------------------------------------------------------
// Scene Geometry and Layout
//--------------------------------------------------------------------------------------

// Prepare the geometry required for the scene
// Returns true on success
bool InitGeometry();

// Layout the scene
// Returns true on success
bool InitScene();

// Release the geometry resources created above
void ReleaseResources();

//--------------------------------------------------------------------------------------
// Scene Render and Update
//--------------------------------------------------------------------------------------

void RenderScene();

// frameTime is the time passed since the last frame
void UpdateScene(float frameTime);


#endif //_SCENE_H_INCLUDED_
