//--------------------------------------------------------------------------------------
// Scene geometry and layout preparation
// Scene rendering & update
//--------------------------------------------------------------------------------------

#ifndef _SCENE_H_INCLUDED_
#define _SCENE_H_INCLUDED_

class SceneModel;

enum RenderMode
{
	Default,
	Wiggle,
	TextureFade,
	NormalMap,
	ParallaxMap,
	AddBlend,
	AddBlendLight,
	MultBlend,
	AlphBlend,
	None // Assign this to hide the object
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
