//--------------------------------------------------------------------------------------
// Scene geometry and layout preparation
// Scene rendering & update
//--------------------------------------------------------------------------------------

#include "Scene.h"
#include "Mesh.h"
#include "Model.h"
#include "Camera.h"
#include "State.h"
#include "Shader.h"
#include "Input.h"
#include "Common.h"
#include "Light.h"

#include "CVector2.h" 
#include "CVector3.h" 
#include "CMatrix4x4.h"
#include "MathHelpers.h"     // Helper functions for maths
#include "GraphicsHelpers.h" // Helper functions to unclutter the code here

#include "ColourRGBA.h"

#include "Texture.h"

#include <sstream>
#include <memory>


//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------

// DirectX objects controlling textures used in this lab
const int NUM_TEXTURES = 4;
Texture* gTextures[NUM_TEXTURES];

Texture gStoneTexture = Texture("StoneDiffuseSpecular.dds");
Texture gCrateTexture = Texture("CargoA.dds");
Texture gGroundTexture = Texture("CobbleDiffuseSpecular.dds");
Texture gLightTexture = Texture("Flare.jpg");


//--------------------------------------------------------------------------------------
// Scene Data
//--------------------------------------------------------------------------------------
// Addition of Mesh, Model and Camera classes have greatly simplified this section
// Geometry data has gone to Mesh class. Positions, rotations, matrices have gone to Model and Camera classes

// Constants controlling speed of movement/rotation (measured in units per second because we're using frame time)
const float ROTATION_SPEED = 2.0f;  // 2 radians per second for rotation
const float MOVEMENT_SPEED = 50.0f; // 50 units per second for movement (what a unit of length is depends on 3D model - i.e. an artist decision usually)


// Meshes, models and cameras, same meaning as TL-Engine. Meshes prepared in InitGeometry function, Models & camera in InitScene
Mesh* gCharacterMesh;
Mesh* gCrateMesh;
Mesh* gGroundMesh;
Mesh* gLightMesh;
Mesh* gSphereMesh;

const int NUM_NORMAL_MODELS = 3;
SceneModel* gNormalModels[NUM_NORMAL_MODELS];

const int NUM_WIGGLE_MODELS = 1;
SceneModel* gWiggleModels[NUM_WIGGLE_MODELS];

const int NUM_MODELS = NUM_NORMAL_MODELS + NUM_WIGGLE_MODELS;
SceneModel* gModels[NUM_MODELS];

SceneModel gTeapot = SceneModel(&gStoneTexture);
SceneModel gCrate = SceneModel(&gCrateTexture);
SceneModel gGround = SceneModel(&gGroundTexture);

SceneModel gWiggleSphere = SceneModel(&gStoneTexture);

Camera* gCamera;


// Lights
const int NUM_SPOTLIGHTS = 2;
const int MAX_SPOTLIGHTS = 15;

const int NUM_POINTLIGHTS = 2;
const int MAX_POINTLIGHTS = 25;

const int NUM_LIGHTS = NUM_SPOTLIGHTS + NUM_POINTLIGHTS;

Light* gLights[NUM_LIGHTS];
Spotlight gSpotlights[NUM_SPOTLIGHTS];
Pointlight gPointlights[NUM_POINTLIGHTS];

CVector3 gAmbientColour = { 0.01f, 0.1f, 0.25f }; // Background level of light (slightly bluish to match the far background, which is dark blue)
float    gSpecularPower = 256; // Specular power controls shininess - same for all models in this app

ColourRGBA gBackgroundColor = { 0.2f, 0.2f, 0.3f, 1.0f };

// Variables controlling light1's orbiting of the cube
const float gLightOrbit = 20.0f;
const float gLightOrbitSpeed = 0.7f;


//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------
// Variables sent over to the GPU each frame
// The structures are now in Common.h
// IMPORTANT: Any new data you add in C++ code (CPU-side) is not automatically available to the GPU
//            Anything the shaders need (per-frame or per-model) needs to be sent via a constant buffer

PerFrameConstants gPerFrameConstants;      // The constants that need to be sent to the GPU each frame (see common.h for structure)
ID3D11Buffer*     gPerFrameConstantBuffer; // The GPU buffer that will recieve the constants above

PerModelConstants gPerModelConstants;      // As above, but constant that change per-model (e.g. world matrix)
ID3D11Buffer*     gPerModelConstantBuffer; // --"--


//--------------------------------------------------------------------------------------
// Initialise scene geometry, constant buffers and states
//--------------------------------------------------------------------------------------

// Prepare the geometry required for the scene
// Returns true on success
bool InitGeometry()
{
    // Load mesh geometry data, just like TL-Engine this doesn't create anything in the scene. Create a Model for that.
    // IMPORTANT NOTE: Will only keep the first object from the mesh - multipart objects will have parts missing - see later lab for more robust loader
    try 
    {
        gCharacterMesh = new Mesh("Teapot.x");
        gCrateMesh     = new Mesh("CargoContainer.x");
        gGroundMesh    = new Mesh("Ground.x");
        gLightMesh     = new Mesh("Light.x");
        gSphereMesh    = new Mesh("Sphere.x");
    }
    catch (std::runtime_error e)  // Constructors cannot return error messages so use exceptions to catch mesh errors (fairly standard approach this)
    {
        gLastError = e.what(); // This picks up the error message put in the exception (see Mesh.cpp)
        return false;
    }


    // Load the shaders required for the geometry we will use (see Shader.cpp / .h)
    if (!LoadShaders())
    {
        gLastError = "Error loading shaders";
        return false;
    }

    // Create shadow map textures
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = gShadowMapSize; // Size of the shadow map determines quality / resolution of shadows
    textureDesc.Height = gShadowMapSize;
    textureDesc.MipLevels = 1; // 1 level, means just the main texture, no additional mip-maps. Usually don't use mip-maps when rendering to textures (or we would have to render every level)
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R32_TYPELESS; // The shadow map contains a single 32-bit value [tech gotcha: have to say typeless because depth buffer and shaders see things slightly differently]
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL | D3D10_BIND_SHADER_RESOURCE; // Indicate we will use texture as a depth buffer and also pass it to shaders
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = 0;
    for (int i = 0; i < NUM_SPOTLIGHTS; i++)
    {
        if (FAILED(gD3DDevice->CreateTexture2D(&textureDesc, NULL, &gSpotlights[i].shadowMapTexture)))
        {
            gLastError = "Error creating shadow map texture";
            return false;
        }
    }

    // Create the depth stencil view, i.e. indicate that the texture just created is to be used as a depth buffer
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT; // See "tech gotcha" above. The depth buffer sees each pixel as a "depth" float
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;
    dsvDesc.Flags = 0;
    for (int i = 0; i < NUM_SPOTLIGHTS; i++)
    {
        if (FAILED(gD3DDevice->CreateDepthStencilView(gSpotlights[i].shadowMapTexture, &dsvDesc, &gSpotlights[i].shadowMapDepthStencil)))
        {
            gLastError = "Error creating shadow map depth stencil view";
            return false;
        }
    }

    // We also need to send this texture (resource) to the shaders. To do that we must create a shader-resource "view"
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT; // See "tech gotcha" above. The shaders see textures as colours, so shadow map pixels are not seen as depths
                                           // but rather as "red" floats (one float taken from RGB). Although the shader code will use the value as a depth
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    for (int i = 0; i < NUM_SPOTLIGHTS; i++)
    {
        if (FAILED(gD3DDevice->CreateShaderResourceView(gSpotlights[i].shadowMapTexture, &srvDesc, &gSpotlights[i].shadowMapSRV)))
        {
            gLastError = "Error creating shadow map shader resource view";
            return false;
        }
    }

    // Create GPU-side constant buffers to receive the gPerFrameConstants and gPerModelConstants structures above
    // These allow us to pass data from CPU to shaders such as lighting information or matrices
    // See the comments above where these variable are declared and also the UpdateScene function
    gPerFrameConstantBuffer = CreateConstantBuffer(sizeof(gPerFrameConstants));
    gPerModelConstantBuffer = CreateConstantBuffer(sizeof(gPerModelConstants));
    if (gPerFrameConstantBuffer == nullptr || gPerModelConstantBuffer == nullptr)
    {
        gLastError = "Error creating constant buffers";
        return false;
    }

    //// Load / prepare textures on the GPU ////

    // Load textures and create DirectX objects for them
    // The LoadTexture function requires you to pass a ID3D11Resource* (e.g. &gCubeDiffuseMap), which manages the GPU memory for the
    // texture and also a ID3D11ShaderResourceView* (e.g. &gCubeDiffuseMapSRV), which allows us to use the texture in shaders
    // The function will fill in these pointers with usable data. The variables used here are globals found near the top of the file.
    gTextures[0] = &gStoneTexture;
    gTextures[1] = &gCrateTexture;
    gTextures[2] = &gGroundTexture;
    gTextures[3] = &gLightTexture;
    
    for (int i = 0; i < NUM_TEXTURES; i++)
    {
        if (!LoadTexture(gTextures[i]->name, &gTextures[i]->diffuseSpecularMap, &gTextures[i]->diffuseSpecularMapSRV))
        {
            gLastError = "Error loading textures";
            return false;
        }
    }

  	// Create all filtering modes, blending modes etc. used by the app (see State.cpp/.h)
	if (!CreateStates())
	{
		gLastError = "Error creating states";
		return false;
	}

	return true;
}


// Prepare the scene
// Returns true on success
bool InitScene()
{
    //// Set up camera ////

    gCamera = new Camera();
    gCamera->SetPosition({ 15, 30,-70 });
    gCamera->SetRotation({ ToRadians(13), 0, 0 });

    //// Set up scene ////

    gTeapot.model = new Model(gCharacterMesh);
    gNormalModels[0] = &gTeapot;

    gCrate.model     = new Model(gCrateMesh);
    gNormalModels[1] = &gCrate;
    
    gGround.model    = new Model(gGroundMesh);
    gNormalModels[2] = &gGround;

    gWiggleSphere.model = new Model(gSphereMesh);
    gWiggleModels[0] = &gWiggleSphere;

    int modelIndex = 0;
    for (int i = 0; i < NUM_NORMAL_MODELS; i++)
    {
        gModels[modelIndex] = gNormalModels[i];
        modelIndex++;
    }
    for (int i = 0; i < NUM_WIGGLE_MODELS; i++)
    {
        gModels[modelIndex] = gWiggleModels[i];
        modelIndex++;
    }

	// Initial positions
	gTeapot.model->SetPosition({ 15, 0, 0 });
    gTeapot.model->SetScale(1);
    gTeapot.model->SetRotation({ 0, ToRadians(215.0f), 0 });

	gCrate.model->SetPosition({ 40, 0, 30 });
	gCrate.model->SetScale(6);
	gCrate.model->SetRotation({ 0.0f, ToRadians(-20.0f), 0.0f });

    gWiggleSphere.model->SetPosition({ 0, 6, -5 });
    gWiggleSphere.model->SetScale(0.3f);

    // Light set-up
    int lightIndex = 0;
    for (int i = 0; i < NUM_SPOTLIGHTS; ++i)
    {
        gSpotlights[i].texture = &gLightTexture;
        gSpotlights[i].model = new Model(gLightMesh);

        gLights[lightIndex] = &gSpotlights[i];
        lightIndex++;
    }

    for (int i = 0; i < NUM_POINTLIGHTS; ++i)
    {
        gPointlights[i].texture = &gLightTexture;
        gPointlights[i].model = new Model(gLightMesh);

        gLights[lightIndex] = &gSpotlights[i];
        lightIndex++;
    }

    // Orbiting spotlight
    gSpotlights[0].colour = { 0.8f, 0.8f, 1.0f };
    gSpotlights[0].SetStrength(10);
    gSpotlights[0].model->SetPosition({ 30, 15, 0 });
    gSpotlights[0].model->FaceTarget(gTeapot.model->Position());


    // Sun (fake directional light)
    gSpotlights[1].colour = { 1.0f, 0.8f, 0.2f };
    gSpotlights[1].SetStrength(100);
    gSpotlights[1].model->SetPosition({ -130, 80, 285 });
    gSpotlights[1].model->FaceTarget({ 0, 10, 0 });
    gSpotlights[1].isSpot = false;

    // Flickering light
    gPointlights[0].colour = { 0.2f, 0.7f, 1.0f };
    gPointlights[0].SetStrength(25);
    gPointlights[0].model->SetPosition({ 20, 15, 12 });
    gPointlights[0].model->FaceTarget(gCamera->Position());

    // Colour changing light
    gPointlights[1].colour = { 1.0f, 0.0f, 0.24f };
    gPointlights[1].SetStrength(25);
    gPointlights[1].model->SetPosition({ -20, 18, 10 });
    gPointlights[1].model->FaceTarget(gCamera->Position());

    return true;
}


// Release the geometry and scene resources created above
void ReleaseResources()
{
    ReleaseStates();

    for (int i = 0; i < NUM_TEXTURES; i++)
    {
        gTextures[i]->~Texture();
    }

    if (gPerModelConstantBuffer)  gPerModelConstantBuffer->Release();
    if (gPerFrameConstantBuffer)  gPerFrameConstantBuffer->Release();

    ReleaseShaders();

    // See note in InitGeometry about why we're not using unique_ptr and having to manually delete
    for (int i = 0; i < NUM_SPOTLIGHTS; ++i)
    {
        delete gSpotlights[i].model;  
        gSpotlights[i].model = nullptr;
    }
    for (int i = 0; i < NUM_POINTLIGHTS; ++i)
    {
        delete gPointlights[i].model;  
        gPointlights[i].model = nullptr;
    }
    delete gCamera;    gCamera    = nullptr;

    for (int i = 0; i < NUM_MODELS; i++)
    {
        gModels[i]->~SceneModel();
    }

    delete gLightMesh;     gLightMesh     = nullptr;
    delete gGroundMesh;    gGroundMesh    = nullptr;
    delete gCrateMesh;     gCrateMesh     = nullptr;
    delete gCharacterMesh; gCharacterMesh = nullptr;
    delete gSphereMesh;    gSphereMesh    = nullptr;
}



//--------------------------------------------------------------------------------------
// Scene Rendering
//--------------------------------------------------------------------------------------

// Render everything in the scene from the given camera
// This code is common between rendering the main scene and rendering the scene in the portal
// See RenderScene function below
void RenderSceneFromCamera(Camera* camera)
{
    // Set camera matrices in the constant buffer and send over to GPU
    gPerFrameConstants.viewMatrix           = camera->ViewMatrix();
    gPerFrameConstants.projectionMatrix     = camera->ProjectionMatrix();
    gPerFrameConstants.viewProjectionMatrix = camera->ViewProjectionMatrix();
    UpdateConstantBuffer(gPerFrameConstantBuffer, gPerFrameConstants);

    // Indicate that the constant buffer we just updated is for use in the vertex shader (VS) and pixel shader (PS)
    gD3DContext->VSSetConstantBuffers(0, 1, &gPerFrameConstantBuffer); // First parameter must match constant buffer number in the shader 
    gD3DContext->PSSetConstantBuffers(0, 1, &gPerFrameConstantBuffer);


    //// Render lit models ////

    // Select which shaders to use next
    gD3DContext->VSSetShader(gPixelLightingVertexShader, nullptr, 0);
    gD3DContext->PSSetShader(gPixelLightingPixelShader,  nullptr, 0);
    
    // States - no blending, normal depth buffer and culling
    gD3DContext->OMSetBlendState(gNoBlendingState, nullptr, 0xffffff);
    gD3DContext->OMSetDepthStencilState(gUseDepthBufferState, 0);
    gD3DContext->RSSetState(gCullBackState);

    // Select the approriate textures and sampler to use in the pixel shader
    gD3DContext->PSSetSamplers(0, 1, &gAnisotropic4xSampler);

    // Render model - it will update the model's world matrix and send it to the GPU in a constant buffer, then it will call
    // the Mesh render function, which will set up vertex & index buffer before finally calling Draw on the GPU
    for (int i = 0; i < NUM_NORMAL_MODELS; i++)
    {
        gD3DContext->PSSetShaderResources(0, 1, &gNormalModels[i]->texture->diffuseSpecularMapSRV);
        gNormalModels[i]->model->Render();
    }

    gD3DContext->VSSetShader(gWiggleVertexShader, nullptr, 0);
    gD3DContext->PSSetShader(gWigglePixelShader, nullptr, 0);
    for (int i = 0; i < NUM_WIGGLE_MODELS; i++)
    {
        gD3DContext->PSSetShaderResources(0, 1, &gWiggleModels[i]->texture->diffuseSpecularMapSRV);
        gWiggleModels[i]->model->Render();
    }

    //// Render lights ////

    // Select which shaders to use next
    gD3DContext->VSSetShader(gBasicTransformVertexShader, nullptr, 0);
    gD3DContext->PSSetShader(gLightModelPixelShader,      nullptr, 0);

    // Select the texture and sampler to use in the pixel shader
    gD3DContext->PSSetShaderResources(0, 1, &gLightTexture.diffuseSpecularMapSRV); // First parameter must match texture slot number in the shaer
    gD3DContext->PSSetSamplers(0, 1, &gAnisotropic4xSampler);

    // States - additive blending, read-only depth buffer and no culling (standard set-up for blending
    gD3DContext->OMSetBlendState(gAdditiveBlendingState, nullptr, 0xffffff);
    gD3DContext->OMSetDepthStencilState(gDepthReadOnlyState, 0);
    gD3DContext->RSSetState(gCullNoneState);

    // Render all the lights in the arrays
    for (int i = 0; i < NUM_LIGHTS; ++i)
    {
        gPerModelConstants.objectColour = gLights[i]->colour; // Set any per-model constants apart from the world matrix just before calling render (light colour here)
        gLights[i]->model->Render();
    }
    for (int i = 0; i < NUM_POINTLIGHTS; ++i)
    {
        gPerModelConstants.objectColour = gPointlights[i].colour; // Set any per-model constants apart from the world matrix just before calling render (light colour here)
        gPointlights[i].model->Render();
    }
}


// Rendering the scene now renders everything twice. First it renders the scene for the portal into a texture.
// Then it renders the main scene using the portal texture on a model.
void RenderScene()
{
    //// Common settings ////

    // Set up the light information in the constant buffer
    gPerFrameConstants.spotlightNumber = (float)NUM_SPOTLIGHTS;

    for (int i = 0; i < NUM_SPOTLIGHTS; i++)
    {
        gSpotlights[i].SetBuffer();
        gPerFrameConstants.spotlights[i] = gSpotlights[i].buffer;
    }

    gPerFrameConstants.pointlightNumber = (float)NUM_POINTLIGHTS;

    for (int i = 0; i < NUM_POINTLIGHTS; i++)
    {
        gPointlights[i].SetBuffer();
        gPerFrameConstants.pointlights[i] = gPointlights[i].buffer;
    }

    gPerFrameConstants.ambientColour  = gAmbientColour;
    gPerFrameConstants.specularPower  = gSpecularPower;
    gPerFrameConstants.cameraPosition = gCamera->Position();

    //***************************************//
    //// Render from light's point of view ////
    
    for (int i = 0; i < NUM_SPOTLIGHTS; i++)
    {
        gSpotlights[i].RenderFromLightPOV(NUM_NORMAL_MODELS, gNormalModels, NUM_WIGGLE_MODELS, gWiggleModels);
    }

    //// Main scene rendering ////

    // Set the back buffer as the target for rendering and select the main depth buffer.
    // When finished the back buffer is sent to the "front buffer" - which is the monitor.
    gD3DContext->OMSetRenderTargets(1, &gBackBufferRenderTarget, gDepthStencil);

    // Clear the back buffer to a fixed colour and the depth buffer to the far distance
    gD3DContext->ClearRenderTargetView(gBackBufferRenderTarget, &gBackgroundColor.r);
    gD3DContext->ClearDepthStencilView(gDepthStencil, D3D11_CLEAR_DEPTH, 1.0f, 0);

    // Setup the viewport to the size of the main window
    D3D11_VIEWPORT vp;
    vp.Width  = static_cast<FLOAT>(gViewportWidth);
    vp.Height = static_cast<FLOAT>(gViewportHeight);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    gD3DContext->RSSetViewports(1, &vp);

    // Set shadow maps in shaders
    // First parameter is the "slot", must match the Texture2D declaration in the HLSL code
    // In this app the diffuse map uses slot 0, the shadow maps use slots 1 onwards. If we were using other maps (e.g. normal map) then
    // we might arrange things differently

    ID3D11ShaderResourceView* shadowMaps[NUM_SPOTLIGHTS];
    
    for (int i = 0; i < NUM_SPOTLIGHTS; i++)
    {
        shadowMaps[i] = gSpotlights[i].shadowMapSRV;
    }

    gD3DContext->PSSetShaderResources(2, NUM_SPOTLIGHTS, shadowMaps);
    gD3DContext->PSSetSamplers(1, 1, &gPointSampler);

    // Render the scene for the main window
    RenderSceneFromCamera(gCamera);

    // Unbind shadow maps from shaders - prevents warnings from DirectX when we try to render to the shadow maps again next frame
    for (unsigned int i = 0; i < NUM_SPOTLIGHTS; i++)
    {
        shadowMaps[i] = nullptr;
    }

    gD3DContext->PSSetShaderResources(2, NUM_SPOTLIGHTS, shadowMaps);


    //*****************************//
    // Temporary demonstration code for visualising the light's view of the scene
    //ColourRGBA white = {1,1,1};
    //gD3DContext->ClearRenderTargetView(gBackBufferRenderTarget, &white.r);
    //gSpotlights[0].RenderDepthBufferFromLight(NUM_MODELS, gModels);
    //*****************************//


    //// Scene completion ////

    // When drawing to the off-screen back buffer is complete, we "present" the image to the front buffer (the screen)
    gSwapChain->Present(0, 0);
}


//--------------------------------------------------------------------------------------
// Scene Update
//--------------------------------------------------------------------------------------

// Update models and camera. frameTime is the time passed since the last frame
void UpdateScene(float frameTime)
{
    // Flickering light
    const float flickerSpeed = 16;
    static const float strengthMax = gPointlights[0].strength;
    static float currentStrength = gPointlights[0].strength;
    static bool flickerDown = true;
    if (flickerDown)
    {
        currentStrength -= frameTime * flickerSpeed;
        if (currentStrength < 0)
        {
            currentStrength = 0;
            flickerDown = false;
        }
    }
    else
    {
        currentStrength += frameTime * flickerSpeed;
        if (currentStrength > strengthMax)
        {
            currentStrength = strengthMax;
            flickerDown = true;
        }
    }
    gPointlights[0].SetStrength(currentStrength);

    // Colour changing light
    const CVector3 rainbow[7]
    {
        { 1.0f,  0.0f, 0.24f },
        { 1.0f,  0.4f, 0.0f  },
        { 0.9f,  1.0f, 0.0f  },
        { 0.0f,  1.0f, 0.58f },
        { 0.0f,  1.0f, 1.0f  },
        { 0.0f,  0.5f, 1.0f  },
        { 0.83f, 0.0f, 1.0f  }
    };
    const float colourSpeed = 1;
    static int currentColour = 0;
    static int nextColour = 1;
    static float colourProgress = 0;

    colourProgress += colourSpeed * frameTime;
    if (colourProgress > 1)
    {
        colourProgress -= 1.0f;
        currentColour = nextColour;
        nextColour++;
        if (nextColour > 6) nextColour = 0;
    }
    gPointlights[1].colour = colourProgress * rainbow[nextColour] + (1 - colourProgress) * rainbow[currentColour];

    // Wiggle effect
    static float wiggle = 0;
    wiggle += frameTime;
    gPerFrameConstants.wiggle = wiggle;

	// Control sphere (will update its world matrix)
	gTeapot.model->Control(frameTime, Key_I, Key_K, Key_J, Key_L, Key_U, Key_O, Key_Period, Key_Comma );

    // Orbit the light - a bit of a cheat with the static variable [ask the tutor if you want to know what this is]
	static float rotate = 0.0f;
    static bool go = true;
	gSpotlights[0].model->SetPosition( gTeapot.model->Position() + CVector3{ cos(rotate) * gLightOrbit, 10, sin(rotate) * gLightOrbit } );
	gSpotlights[0].model->FaceTarget(gTeapot.model->Position());
    if (go)  rotate -= gLightOrbitSpeed * frameTime;
    if (KeyHit(Key_1))  go = !go;

	// Control camera (will update its view matrix)
	gCamera->Control(frameTime, Key_Up, Key_Down, Key_Left, Key_Right, Key_W, Key_S, Key_A, Key_D );

    // Show frame time / FPS in the window title //
    const float fpsUpdateTime = 0.5f; // How long between updates (in seconds)
    static float totalFrameTime = 0;
    static int frameCount = 0;
    totalFrameTime += frameTime;
    ++frameCount;
    if (totalFrameTime > fpsUpdateTime)
    {
        // Displays FPS rounded to nearest int, and frame time (more useful for developers) in milliseconds to 2 decimal places
        float avgFrameTime = totalFrameTime / frameCount;
        std::ostringstream frameTimeMs;
        frameTimeMs.precision(2);
        frameTimeMs << std::fixed << avgFrameTime * 1000;
        std::string windowTitle = "CO2409 Assignment - Frame Time: " + frameTimeMs.str() +
                                  "ms, FPS: " + std::to_string(static_cast<int>(1 / avgFrameTime + 0.5f));
        SetWindowTextA(gHWnd, windowTitle.c_str());
        totalFrameTime = 0;
        frameCount = 0;
    }
}