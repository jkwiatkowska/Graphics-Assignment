#include "Texture.h"

Texture::Texture(std::string filename)
{
	name = filename;
}

Texture::~Texture()
{
	if (diffuseSpecularMap) diffuseSpecularMap->Release();
	if (diffuseSpecularMapSRV) diffuseSpecularMapSRV->Release();
}