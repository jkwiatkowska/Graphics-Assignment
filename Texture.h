#pragma once
#include "Model.h"

#include <string>

class Texture
{
public:
	std::string name;
	std::string normalName;

	ID3D11Resource* diffuseSpecularMap = nullptr; // This object represents the memory used by the texture on the GPU
	ID3D11ShaderResourceView* diffuseSpecularMapSRV = nullptr; // This object is used to give shaders access to the texture above (SRV = shader resource view)

	ID3D11Resource* normalMap = nullptr;
	ID3D11ShaderResourceView* normalMapSRV = nullptr;

	Texture(std::string filename, std::string normalFilename = "");
	~Texture();
};