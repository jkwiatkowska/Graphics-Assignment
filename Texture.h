#pragma once
#include "Model.h"

#include <string>

class Texture
{
public:
	std::string name;

	ID3D11Resource* diffuseSpecularMap = nullptr; // This object represents the memory used by the texture on the GPU
	ID3D11ShaderResourceView* diffuseSpecularMapSRV = nullptr; // This object is used to give shaders access to the texture above (SRV = shader resource view)

	Texture(std::string filename);
	~Texture();
};