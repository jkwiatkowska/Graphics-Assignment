#include "Texture.h"

Texture::Texture(std::string filename, std::string normalFilename)
{
	name = filename;
	normalName = normalFilename;
}

Texture::~Texture()
{
	if (diffuseSpecularMap) diffuseSpecularMap->Release();
	if (diffuseSpecularMapSRV) diffuseSpecularMapSRV->Release();

	if (normalMap) normalMap->Release();
	if (normalMapSRV) normalMapSRV->Release();
}