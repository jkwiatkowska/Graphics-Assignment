#pragma once
#include "Texture.h"
#include "Scene.h"
#include "Mesh.h"
#include "Model.h"
#include "Camera.h"
#include "State.h"
#include "Shader.h"
#include "Input.h"
#include "Common.h"

#include "CVector2.h" 
#include "CVector3.h" 
#include "CMatrix4x4.h"
#include "MathHelpers.h"     // Helper functions for maths
#include "GraphicsHelpers.h" // Helper functions to unclutter the code here

#include "ColourRGBA.h" 

#include <sstream>
#include <memory>

class SceneModel
{
public:
	Model* model = nullptr;
	Texture* texture = nullptr;
	Texture* texture2 = nullptr;
	ShaderType shader = Default;

	SceneModel()
	{

	}

	SceneModel(Texture* modelTexture, Texture* modelTexture2 = nullptr)
	{
		texture = modelTexture;
		texture2 = modelTexture2;
	}

	~SceneModel()
	{
		delete model;
		model = nullptr;
	}
};

