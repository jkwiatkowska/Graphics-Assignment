#include "Light.h"

void Light::SetStrength(float newStrength)
{
    strength = newStrength;
    model->SetScale(pow(strength, 0.7f));
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
void Spotlight::RenderDepthBufferFromLight(int numModels, SceneModel* models[])
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
        if (models[i]->shader == Default || models[i]->shader == TextureFade) models[i]->model->Render();
    }
    gD3DContext->VSSetShader(gWiggleVertexShader, nullptr, 0);
    for (int i = 0; i < numModels; i++)
    {
        if (models[i]->shader == Wiggle) models[i]->model->Render();
    }
}

void Spotlight::RenderFromLightPOV(int numModels, SceneModel* models[])
{
    //// Render from light's point of view ////

    // Setup the viewport to the size of the shadow map texture
    D3D11_VIEWPORT vp;
    vp.Width = static_cast<FLOAT>(gShadowMapSize);
    vp.Height = static_cast<FLOAT>(gShadowMapSize);
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
    RenderDepthBufferFromLight(numModels, models);
}

void Pointlight::SetBuffer()
{
    buffer.colour = colour * strength;
    buffer.position = model->Position();
}