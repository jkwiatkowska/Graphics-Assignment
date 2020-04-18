#include "SceneModel.h"

SceneModel::SceneModel(Texture* modelTexture, Texture* modelTexture2)
{
	texture = modelTexture;
	texture2 = modelTexture2;
}

SceneModel::~SceneModel()
{
	delete model;
	model = nullptr;
	texture = nullptr;
	texture2 = nullptr;
}