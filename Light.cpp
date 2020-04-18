#include "Light.h"

const FLOAT gWhite[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

void Light::SetStrength(float newStrength)
{
    strength = newStrength;
    model->SetScale(pow(strength, 0.7f));
}

void Light::MakeFlicker()
{
    flicker = true;
    strengthMax = strength;
    currentStrength = strength;
}

void Light::MakeRainbow()
{
    colourChange = true;
}

void Light::Update(float frameTime)
{
    // Flickering
    if (flicker)
    {
        if (flickerDown)
        {
            currentStrength -= frameTime * strengthMax / flickerTime;
            if (currentStrength < 0)
            {
                currentStrength = 0;
                flickerDown = false;
            }
        }
        else
        {
            currentStrength += frameTime * strengthMax / flickerTime;
            if (currentStrength > strengthMax)
            {
                currentStrength = strengthMax;
                flickerDown = true;
            }
        }

        SetStrength(currentStrength);
    }

    if (colourChange)
    {
        colourProgress += colourSpeed * frameTime;
        if (colourProgress > 1)
        {
            colourProgress -= 1.0f;
            currentColour = nextColour;
            nextColour++;
            if (nextColour > lastColour) nextColour = 0;
        }
        colour = colourProgress * colours[nextColour] + (1 - colourProgress) * colours[currentColour];
    }
}

void Spotlight::SetBuffer()
{
    buffer.colour = colour * strength;
    buffer.isSpot = isSpot;
    buffer.position = model->Position();
    buffer.facing = GetFacing();    // Additional lighting information for spotlights
    buffer.cosHalfAngle = cos(ToRadians(gSpotlightConeAngle / 2)); // --"--
    buffer.viewMatrix = CalculateLightViewMatrix();         // Calculate camera-like matrices for...
    buffer.projectionMatrix = CalculateLightProjectionMatrix();   //...lights to support shadow mapping
}

// Get "camera-like" view matrix for a spotlight
CMatrix4x4 Spotlight::CalculateLightViewMatrix()
{
    return InverseAffine(model->WorldMatrix());
}

// Get "camera-like" projection matrix for a spotlight
CMatrix4x4 Spotlight::CalculateLightProjectionMatrix()
{
    return MakeProjectionMatrix(1.0f, ToRadians(gSpotlightConeAngle)); // Helper function in Utility\GraphicsHelpers.cpp
}

CVector3 Spotlight::GetFacing()
{
    return Normalise(model->WorldMatrix().GetZAxis());
}

// Render the scene from the given light's point of view. Only renders depth buffer
void Spotlight::RenderShadowMap(int numModels, SceneModel* models[])
{
    // Get camera-like matrices from the spotlight, set in the constant buffer and send over to GPU
    gPerFrameConstants.viewMatrix = CalculateLightViewMatrix();
    gPerFrameConstants.projectionMatrix = CalculateLightProjectionMatrix();
    gPerFrameConstants.viewProjectionMatrix = gPerFrameConstants.viewMatrix * gPerFrameConstants.projectionMatrix;
    UpdateConstantBuffer(gPerFrameConstantBuffer, gPerFrameConstants);

    // Indicate that the constant buffer we just updated is for use in the vertex shader (VS) and pixel shader (PS)
    gD3DContext->VSSetConstantBuffers(0, 1, &gPerFrameConstantBuffer); // First parameter must match constant buffer number in the shader 
    gD3DContext->PSSetConstantBuffers(0, 1, &gPerFrameConstantBuffer);

    //// Only render models that cast shadows ////

    // Use special depth-only rendering shaders
    gD3DContext->VSSetShader(gBasicTransformVertexShader, nullptr, 0);
    gD3DContext->PSSetShader(gDepthOnlyPixelShader, nullptr, 0);

    // States - no blending, normal depth buffer and culling
    gD3DContext->OMSetBlendState(gNoBlendingState, nullptr, 0xffffff);
    gD3DContext->OMSetDepthStencilState(gUseDepthBufferState, 0);
    gD3DContext->RSSetState(gCullFrontState);

    // Render models - no state changes required between each object in this situation (no textures used in this step)
    for (int i = 0; i < numModels; i++)
    {
        if (models[i]->renderMode == Default || models[i]->renderMode == TextureFade || models[i]->renderMode == NormalMap || models[i]->renderMode == ParallaxMap || models[i]->renderMode == Bright ||
            models[i]->renderMode == TextureGradient || models[i]->renderMode == CubeMapLight || models[i]->renderMode == CubeMap)
            models[i]->model->Render();
    }
    gD3DContext->VSSetShader(gWiggleVertexShader, nullptr, 0);
    for (int i = 0; i < numModels; i++)
    {
        if (models[i]->renderMode == Wiggle) models[i]->model->Render();
    }
}

void Spotlight::RenderColourMap(int numModels, SceneModel* models[])
{
    // Get camera-like matrices from the spotlight, set in the constant buffer and send over to GPU
    gPerFrameConstants.viewMatrix = CalculateLightViewMatrix();
    gPerFrameConstants.projectionMatrix = CalculateLightProjectionMatrix();
    gPerFrameConstants.viewProjectionMatrix = gPerFrameConstants.viewMatrix * gPerFrameConstants.projectionMatrix;
    UpdateConstantBuffer(gPerFrameConstantBuffer, gPerFrameConstants);

    // Indicate that the constant buffer we just updated is for use in the vertex shader (VS) and pixel shader (PS)
    gD3DContext->VSSetConstantBuffers(0, 1, &gPerFrameConstantBuffer); // First parameter must match constant buffer number in the shader 
    gD3DContext->PSSetConstantBuffers(0, 1, &gPerFrameConstantBuffer);

    /// Transparent models ///
    // States - no blending, normal depth buffer and culling
    gD3DContext->OMSetBlendState(gMultiplicativeBlendingState, nullptr, 0xffffff);
    gD3DContext->OMSetDepthStencilState(gDepthReadOnlyState, 0);
    gD3DContext->RSSetState(gCullNoneState);

    // Shaders
    gD3DContext->VSSetShader(gBasicTransformVertexShader, nullptr, 0);
    gD3DContext->PSSetShader(gAlphaPixelShader, nullptr, 0);

    // Render models
    for (int i = 0; i < numModels; i++)
    {
        if (models[i]->renderMode == AddBlendLight)
        {
            gD3DContext->PSSetShaderResources(0, 1, &models[i]->texture->diffuseSpecularMapSRV);
            models[i]->model->Render();
        }
    }
}

void Spotlight::RenderFromLightPOV(int numModels, SceneModel* models[])
{
    // Setup the viewport to the size of the shadow map texture
    D3D11_VIEWPORT vp;
    vp.Width = static_cast<FLOAT>(shadowMapSize);
    vp.Height = static_cast<FLOAT>(shadowMapSize);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    gD3DContext->RSSetViewports(1, &vp);

    // Select the shadow map texture as the current depth buffer. We will not be rendering any pixel colours
    // Also clear the the shadow map depth buffer to the far distance
    gD3DContext->OMSetRenderTargets(0, nullptr, shadowMapDepthStencil);
    gD3DContext->ClearDepthStencilView(shadowMapDepthStencil, D3D11_CLEAR_DEPTH, 1.0f, 0);

    // Render the scene from the point of view of light (only depth values written)
    RenderShadowMap(numModels, models);

    // Create colour map
    gD3DContext->OMSetRenderTargets(1, &colourMapRenderTarget, shadowMapDepthStencil);
    gD3DContext->ClearRenderTargetView(colourMapRenderTarget, gWhite);

    RenderColourMap(numModels, models);
}

void Pointlight::SetBuffer()
{
    buffer.colour = colour * strength;
    buffer.position = model->Position();
}