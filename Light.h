#pragma once
#include "SceneModel.h"

__declspec(selectany) extern const int gShadowMapSize = 4096; // Dimensions of shadow map texture - controls quality of shadows

// Base light class
class Light : public SceneModel
{
public:
    // Basic information
    CVector3 colour;
    float    strength;

    // Flickering
    bool flicker;
    float flickerTime = 1.2f;
    float strengthMax;
    float currentStrength;
    bool flickerDown = true;

    // Colour change
    bool colourChange = false;
    CVector3 colours[7]
    {
        { 1.0f,  0.0f, 0.24f },
        { 1.0f,  0.4f, 0.0f  },
        { 0.9f,  1.0f, 0.0f  },
        { 0.0f,  1.0f, 0.58f },
        { 0.0f,  1.0f, 1.0f  },
        { 0.0f,  0.5f, 1.0f  },
        { 0.83f, 0.0f, 1.0f  }
    };
    int lastColour = 6;
    float colourSpeed = 1;
    int currentColour = 0;
    int nextColour = 1;
    float colourProgress = 0;

    Light()
    {

    }

    virtual void SetStrength(float newStrength);

    void MakeFlicker();
    void MakeRainbow();

    void Update(float frameTime);
};

// Spotlight class with shadows
class Spotlight : public Light
{
public:
    SpotlightBuffer buffer;
    float gSpotlightConeAngle = 90.0f; // Spot light cone angle (degrees), like the FOV (field-of-view) of the spot light
    bool isSpot = true;

    // The shadow texture - effectively a depth buffer of the scene **from the light's point of view**
    // Each frame it is rendered to, then the texture is used to help the per-pixel lighting shader identify pixels in shadow
    ID3D11Texture2D* shadowMapTexture = nullptr; // This object represents the memory used by the texture on the GPU
    ID3D11DepthStencilView* shadowMapDepthStencil = nullptr; // This object is used when we want to render to the texture above **as a depth buffer**
    ID3D11ShaderResourceView* shadowMapSRV = nullptr; // This object is used to give shaders access to the texture above (SRV = shader resource view)

    ID3D11Texture2D* colourMapTexture = nullptr;
    ID3D11RenderTargetView* colourMapRenderTarget = nullptr;
    ID3D11ShaderResourceView* colourMapSRV = nullptr;

    Spotlight()
    {

    }

    void SetBuffer();

    CVector3 GetFacing();

    void RenderFromLightPOV(int numModels, SceneModel* models[]);
    void RenderColourMap(int numModels, SceneModel* models[]);

    CMatrix4x4 CalculateLightViewMatrix();
    CMatrix4x4 CalculateLightProjectionMatrix();
    void RenderShadowMap(int numModels, SceneModel* models[]);
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