#pragma once
#include "SceneModel.h"

__declspec(selectany) extern const int gShadowMapSize = 4096; // Dimensions of shadow map texture - controls quality of shadows

// Base light class
class Light : public SceneModel
{
public:
    CVector3 colour;
    float    strength;

    Light()
    {

    }

    void SetStrength(float newStrength);
};

// Spotlight class with shadows
class Spotlight : public Light
{
public:
    SpotlightBuffer buffer;
    int number = 0;

    // The shadow texture - effectively a depth buffer of the scene **from the light's point of view**
    // Each frame it is rendered to, then the texture is used to help the per-pixel lighting shader identify pixels in shadow
    ID3D11Texture2D* shadowMapTexture = nullptr; // This object represents the memory used by the texture on the GPU
    ID3D11DepthStencilView* shadowMapDepthStencil = nullptr; // This object is used when we want to render to the texture above **as a depth buffer**
    ID3D11ShaderResourceView* shadowMapSRV = nullptr; // This object is used to give shaders access to the texture above (SRV = shader resource view)

    Spotlight()
    {

    }

    void SetBuffer();

    CVector3 GetFacing();

    void RenderFromLightPOV(int numModels, SceneModel* models[]);

    CMatrix4x4 CalculateLightViewMatrix();
    CMatrix4x4 CalculateLightProjectionMatrix();
    void RenderDepthBufferFromLight(int numModels, SceneModel* models[]);
};

class Pointlight : public Light
{
public:
    Pointlight()
    {

    }

    PointlightBuffer buffer;

    void SetBuffer();
};

class Directionlight : public Light
{
public:
    Directionlight()
    {

    }


};