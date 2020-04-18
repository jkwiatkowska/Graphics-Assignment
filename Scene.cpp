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
const int NUM_TEXTURES = 16;
Texture* gTextures[NUM_TEXTURES];

Texture gStoneTexture      = Texture("StoneDiffuseSpecular.dds");
Texture gCrateTexture      = Texture("CargoA.dds");
Texture gCobbleTexture     = Texture("CobbleDiffuseSpecular.dds", "CobbleNormalHeight.dds");
Texture gLightTexture      = Texture("Flare.jpg");
Texture gWoodTexture       = Texture("WoodDiffuseSpecular.dds", "WoodNormal.dds");
Texture gWallTexture       = Texture("WallDiffuseSpecular.dds", "WallNormalHeight.dds");
Texture gTechTexture       = Texture("TechDiffuseSpecular.dds", "TechNormalHeight.dds");
Texture gPatternTexture    = Texture("PatternDiffuseSpecular.dds", "PatternNormalHeight.dds");
Texture gMetalTexture      = Texture("MetalDiffuseSpecular.dds", "MetalNormal.dds");
Texture gGrassTexture      = Texture("GrassDiffuseSpecular.dds");
Texture gGlassTexture      = Texture("Glass.jpg");
Texture gPortalTexture     = Texture("");
Texture gDecalTexture[3]   = { Texture("acorn.png"), Texture("tank.png"), Texture("wizard.png") };
Texture gBuildingTexture   = Texture("bld-mt.jpg");
Texture gGravelTexture     = Texture("gravel.jpg");

const int NUM_CUBETEXTURES = 4;
Texture* gCubeTextures[NUM_CUBETEXTURES];
Texture gSkyTexture        = Texture("skymap.dds");
Texture gSpaceTexture      = Texture("space.dds");
Texture gCloudsTexture     = Texture("clouds.dds");
Texture gNatureTexture     = Texture("nature.dds");

//--------------------------------------------------------------------------------------
// Scene Data
//--------------------------------------------------------------------------------------
// Addition of Mesh, Model and Camera classes have greatly simplified this section
// Geometry data has gone to Mesh class. Positions, rotations, matrices have gone to Model and Camera classes

// Constants controlling speed of movement/rotation (measured in units per second because we're using frame time)
const float ROTATION_SPEED = 2.0f;  // 2 radians per second for rotation
const float MOVEMENT_SPEED = 50.0f; // 50 units per second for movement (what a unit of length is depends on 3D model - i.e. an artist decision usually)


// Meshes, models and cameras, same meaning as TL-Engine. Meshes prepared in InitGeometry function, Models & camera in InitScene
Mesh* gTeapotMesh;
Mesh* gCrateMesh;
Mesh* gGroundMesh;
Mesh* gLightMesh;
Mesh* gSphereMesh;
Mesh* gTangentSphereMesh;
Mesh* gCubeMesh;
Mesh* gTangentCubeMesh;
Mesh* gQuadMesh;
Mesh* gBuildingMesh;
Mesh* gHillMesh;

const int NUM_MODELS = 40;
SceneModel* gModels[NUM_MODELS];

SceneModel gTeapot = SceneModel(&gStoneTexture);                // 0
SceneModel gCrate = SceneModel(&gCrateTexture);                 // 1
SceneModel gGround = SceneModel(&gCobbleTexture);               // 2

SceneModel gWiggleSphere = SceneModel(&gStoneTexture);          // 3

const int NUM_BRICKS = 14;
SceneModel gBricks[NUM_BRICKS];                                 // 4-17

SceneModel gNormalCube = SceneModel(&gPatternTexture);          // 18
SceneModel gGlassCube = SceneModel(&gGlassTexture);             // 19

SceneModel gPortal(&gPortalTexture);                            // 20

SceneModel gDecal[3];                                           // 21-23

SceneModel gBuilding = SceneModel(&gTechTexture);               // 24
SceneModel gBuilding2 = SceneModel(&gBuildingTexture);          // 25
SceneModel gWoodSphere = SceneModel(&gWoodTexture);             // 26

SceneModel gHill = SceneModel(&gGrassTexture, &gGravelTexture); // 27
const int NUM_LANDSPHERES = 7;
SceneModel gLandSpheres[NUM_LANDSPHERES];                       // 28-34

SceneModel gSky = SceneModel(&gSpaceTexture, &gCloudsTexture);          // 35
SceneModel gCubeMapTeapot = SceneModel(&gSkyTexture, &gCloudsTexture);  // 36
SceneModel gCubeMapSphere[3];                                           // 37-39

Camera* gCamera;

// Lights
const int NUM_SPOTLIGHTS = 3;
const int MAX_SPOTLIGHTS = 15;

const int NUM_POINTLIGHTS = 3;
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
        gTeapotMesh         = new Mesh("Teapot.x");
        gCrateMesh          = new Mesh("CargoContainer.x");
        gGroundMesh         = new Mesh("Ground.x", true);
        gLightMesh          = new Mesh("Light.x");
        gSphereMesh         = new Mesh("Sphere.x");
        gTangentSphereMesh  = new Mesh("Sphere.x", true);
        gCubeMesh           = new Mesh("Cube.x");
        gTangentCubeMesh    = new Mesh("Cube.x", true);
        gQuadMesh           = new Mesh("Portal.x");
        gBuildingMesh       = new Mesh("Building03.x");
        gHillMesh           = new Mesh("Hills.x");
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
    textureDesc.Width = gSpotlights[0].shadowMapSize; // Size of the shadow map determines quality / resolution of shadows
    textureDesc.Height = gSpotlights[0].shadowMapSize;
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

    // Colour maps
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.BindFlags = D3D10_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE; // IMPORTANT: Indicate we will use texture as render target, and pass it to shaders
    for (int i = 0; i < NUM_SPOTLIGHTS; i++)
    {
        if (FAILED(gD3DDevice->CreateTexture2D(&textureDesc, NULL, &gSpotlights[i].colourMapTexture)))
        {
            gLastError = "Error creating shadow map texture";
            return false;
        }

        if (FAILED(gD3DDevice->CreateRenderTargetView(gSpotlights[i].colourMapTexture, NULL, &gSpotlights[i].colourMapRenderTarget)))
        {
            gLastError = "Error creating colour map render target view";
            return false;
        }
    }

    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    for (int i = 0; i < NUM_SPOTLIGHTS; i++)
    {
        if (FAILED(gD3DDevice->CreateShaderResourceView(gSpotlights[i].colourMapTexture, &srvDesc, &gSpotlights[i].colourMapSRV)))
        {
            gLastError = "Error creating colour map shader resource view";
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
    gTextures[0]  = &gStoneTexture;
    gTextures[1]  = &gCrateTexture;
    gTextures[2]  = &gCobbleTexture;
    gTextures[3]  = &gLightTexture;
    gTextures[4]  = &gWoodTexture;
    gTextures[5]  = &gWallTexture;
    gTextures[6]  = &gTechTexture;
    gTextures[7]  = &gPatternTexture;
    gTextures[8]  = &gMetalTexture;
    gTextures[9]  = &gGrassTexture;
    gTextures[10] = &gGlassTexture;
    gTextures[11] = &gBuildingTexture;
    gTextures[12] = &gGravelTexture;
    for (int i = 0; i < 3; i++) gTextures[13 + i] = &gDecalTexture[i];
    
    for (int i = 0; i < NUM_TEXTURES; i++)
    {
        if (!LoadTexture(gTextures[i]->name, &gTextures[i]->diffuseSpecularMap, &gTextures[i]->diffuseSpecularMapSRV))
        {
            gLastError = "Error loading textures";
            return false;
        }
        if (gTextures[i]->normalName != "" && !LoadTexture(gTextures[i]->normalName, &gTextures[i]->normalMap, &gTextures[i]->normalMapSRV))
        {
            gLastError = "Error loading normal textures";
            return false;
        }
    }

    textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;

    gCubeTextures[0] = &gSkyTexture;
    gCubeTextures[1] = &gSpaceTexture;
    gCubeTextures[2] = &gCloudsTexture;
    gCubeTextures[3] = &gNatureTexture;
    for (int i = 0; i < NUM_CUBETEXTURES; i++)
    {
        if (!LoadTexture(gCubeTextures[i]->name, &gCubeTextures[i]->diffuseSpecularMap, &gCubeTextures[i]->diffuseSpecularMapSRV))
        {
            gLastError = "Error loading cube textures";
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
    gCamera->SetPosition({ 10, 56, -118 });
    gCamera->SetRotation({ ToRadians(8.5f), ToRadians(-2), 0 });

    //// Set up scene ////

    // Teapot
    gTeapot.model = new Model(gTeapotMesh);
    gTeapot.model->SetPosition({ 15, 0, -5 });
    gTeapot.model->SetScale(1.2f);
    gTeapot.model->SetRotation({ 0, ToRadians(215.0f), 0 });
    
    gModels[0] = &gTeapot;

    // Crate
    gCrate.model     = new Model(gCrateMesh);
    gCrate.model->SetPosition({ 40, 0, 30 });
    gCrate.model->SetScale(6);
    gCrate.model->SetRotation({ 0.0f, ToRadians(-20.0f), 0.0f });
    
    gModels[1] = &gCrate;
    
    // Ground
    gGround.model    = new Model(gGroundMesh);
    gGround.renderMode = ParallaxMap;
    gGround.model->SetScale(0.8f);
    gModels[2] = &gGround;

    // Wiggle sphere
    gWiggleSphere.model = new Model(gSphereMesh);
    gWiggleSphere.renderMode = Wiggle;
    gWiggleSphere.model->SetPosition({ 0, 6, -5 });
    gWiggleSphere.model->SetScale(0.3f);
    
    gModels[3] = &gWiggleSphere;

    // Bricks
    const int brickRow = 5;
    const int fadeBrick[NUM_BRICKS] = 
    { 
        false, false, true, false, false,
        false, true, false, false, true,
        false, true, false, false
    };
    for (int i = 0; i < NUM_BRICKS; i++)
    {
        gBricks[i] = SceneModel(&gWallTexture, &gPatternTexture);
        gBricks[i].model = new Model(gCubeMesh);
        if (fadeBrick[i]) gBricks[i].renderMode = TextureFade;

        int x = i % brickRow;
        int y = (i - x) / brickRow;
        if (y % 2 == 0)
        {
            x *= 10;
            x += 5;
        }
        else x *= 10;
        y *= 10;

        gBricks[i].model->SetPosition({ 5 + (float)x, 5 + (float)y, 130 });

        gModels[4 + i] = &gBricks[i];
    }

    // Normal mapping cube
    gNormalCube.model = new Model(gTangentCubeMesh);
    gNormalCube.renderMode = NormalMap;
    gNormalCube.model->SetPosition({ -20, 4, 10 });
    gNormalCube.model->SetRotation({ 0, -70, 0 });
    gNormalCube.model->SetScale(0.8f);

    gModels[18] = &gNormalCube;

    // Glass cube
    gGlassCube.model = new Model(gCubeMesh);
    gGlassCube.renderMode = AddBlendLight;
    gGlassCube.model->SetPosition({ 1, 6.1f, 30 });
    gGlassCube.model->SetRotation({ 0, -2, 0 });
    gGlassCube.model->SetScale(1.2f);
    
    gModels[19] = &gGlassCube;

    // Portal
    gPortal.model = new Model(gQuadMesh);
    gPortal.model->SetPosition({ -20, 15, 70 });
    gPortal.model->SetRotation({ 0, 40, 0 });
    gPortal.renderMode = None;

    gModels[20] = &gPortal;

    //Decals
    for (int i = 0; i < 3; i++)
    {
        gDecal[i] = SceneModel(&gDecalTexture[i]);
        gDecal[i].model = new Model(gQuadMesh);
        gDecal[i].model->SetScale({ 0.24f, 0.4f, 1 });
;       gDecal[i].renderMode = AlphBlend;

        gModels[21 + i] = &gDecal[i];
    }
    gDecal[2].model->SetPosition({ 18, 9, 124.8f });     // Wizard 
    gDecal[1].model->SetPosition({ 40, 10, 124.8f });    // Tank
    gDecal[0].model->SetPosition({ 28.4f, 14, 124.8f }); // Acorn
    gDecal[0].model->SetScale({ 0.25f, 0.3f, 1 });

    // Buildings
    gBuilding.model = new Model(gBuildingMesh);
    gBuilding.model->SetPosition({ -60, 0, 105 });
    gBuilding.model->SetRotation({ 0, -2, 0 });
    gBuilding.model->SetScale(0.7f);
    gBuilding.renderMode = Bright;

    gModels[24] = &gBuilding;

    gBuilding2.model = new Model(gBuildingMesh);
    gBuilding2.model->SetPosition({ -66, 0, 70 });
    gBuilding2.model->SetRotation({ 0, 0, 0 });
    gBuilding2.model->SetScale(0.7f);
    gBuilding2.renderMode = Ghost;

    gModels[25] = &gBuilding2;

    // Wood sphere
    gWoodSphere.model = new Model(gTangentSphereMesh);
    gWoodSphere.model->SetPosition({ 15, 3, 34 });
    gWoodSphere.model->SetScale(0.3f);
    gWoodSphere.renderMode = NormalMap;

    gModels[26] = &gWoodSphere;

    // Hill
    gHill.model = new Model(gHillMesh);
    gHill.model->SetScale(3.5f);
    gHill.model->SetPosition({ -65, -15, -20 });
    gHill.renderMode = TextureGradient;

    gModels[27] = &gHill;

    // Land spheres
    for (int i = 0; i < NUM_LANDSPHERES; i++)
    {
        gLandSpheres[i] = SceneModel(&gGrassTexture, &gGravelTexture);
        gLandSpheres[i].model = new Model(gSphereMesh);
        gLandSpheres[i].renderMode = TextureGradient;

        gModels[28 + i] = &gLandSpheres[i];
    }
    gLandSpheres[0].model->SetScale(2.5f);
    gLandSpheres[0].model->SetPosition({ 110, -5, 50 });
    gLandSpheres[1].model->SetScale(1.7f);
    gLandSpheres[1].model->SetPosition({ 90, -1, 120 });
    gLandSpheres[2].model->SetScale(1.1f);
    gLandSpheres[2].model->SetPosition({ 130, 25, 140 });

    gLandSpheres[3].model->SetScale(1.8f);
    gLandSpheres[3].model->SetPosition({ -70, 0, 30 });
    gLandSpheres[4].model->SetScale(0.8f);
    gLandSpheres[4].model->SetPosition({ -50, 0, -5 });

    gLandSpheres[5].model->SetScale(3.4f);
    gLandSpheres[5].model->SetPosition({ -30, 0, 255 });
    gLandSpheres[6].model->SetScale(1.5f);
    gLandSpheres[6].model->SetPosition({ 15, 40, 310 });
    
    // Sky sphere
    gSky.model = new Model(gSphereMesh);
    gSky.renderMode = CubeMapAnimated;
    gSky.model->SetScale(115);
    gSky.model->SetPosition({ 0, -20, 0 });

    gModels[35] = &gSky;

    // Cubemap objects
    gCubeMapTeapot.model = new Model(gTeapotMesh);
    gCubeMapTeapot.renderMode = CubeMapAnimated;
    gCubeMapTeapot.model->SetPosition({ 35, 30, 130 });

    gModels[36] = &gCubeMapTeapot;

    for (int i = 0; i < 3; i++)
    {
        gCubeMapSphere[i].model = new Model(gSphereMesh);
        gCubeMapSphere[i].renderMode = CubeMap;

        gModels[37+i] = &gCubeMapSphere[i];
    }
    gCubeMapSphere[0].texture = &gNatureTexture;
    gCubeMapSphere[1].texture = &gSpaceTexture;
    gCubeMapSphere[2].texture = &gSkyTexture;
   
    gCubeMapSphere[0].model->SetPosition({ 70, 25, 140 });

    gCubeMapSphere[1].model->SetScale(0.72f);
    gCubeMapSphere[1].model->SetPosition({ 54, 45, 143 });

    gCubeMapSphere[2].model->SetScale(0.52f);
    gCubeMapSphere[2].model->SetPosition({ 68.5f, 61, 138 });

    //// Set up lights ////
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

    // Far light
    gSpotlights[1].colour = { 0.6f, 0.9f, 0.8f };
    gSpotlights[1].SetStrength(90);
    gSpotlights[1].model->SetPosition({ -120, 200, 475 });
    gSpotlights[1].model->FaceTarget({ 0, 0, -100 });
    gSpotlights[1].isSpot = false;
    gSpotlights[1].gSpotlightConeAngle = 120;

    gPortalTexture.diffuseSpecularMapSRV = gSpotlights[1].colourMapSRV;

    // Colour changing light
    gSpotlights[2].colour = { 1.0f, 0.0f, 0.24f };
    gSpotlights[2].SetStrength(45);
    gSpotlights[2].model->SetPosition({ -15, 10, 30 });
    gSpotlights[2].model->FaceTarget(gGlassCube.model->Position());
    gSpotlights[2].MakeRainbow();

    // Flickering lights
    gPointlights[0].colour = { 0.2f, 0.7f, 1.0f };
    gPointlights[0].SetStrength(10);
    gPointlights[0].model->SetPosition({ -66, 100, 73.5f });
    gPointlights[0].model->FaceTarget(gCamera->Position());
    gPointlights[0].MakeFlicker();

    gPointlights[1].colour = { 0.9f, 0.1f, 0.5f };
    gPointlights[1].SetStrength(10);
    gPointlights[1].model->SetPosition({ -62.8f, 100, 103.5f });
    gPointlights[1].MakeFlicker();

    // Pointlights
    gPointlights[2].colour = { 0.2f, 0.8f, 0.9f };
    gPointlights[2].SetStrength(15);
    gPointlights[2].model->SetPosition({ 4, 28, 115 });

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

    delete gLightMesh;       gLightMesh      = nullptr;
    delete gGroundMesh;      gGroundMesh     = nullptr;
    delete gCrateMesh;       gCrateMesh      = nullptr;
    delete gTeapotMesh;      gTeapotMesh     = nullptr;
    delete gSphereMesh;      gSphereMesh     = nullptr;
    delete gCubeMesh;        gCubeMesh       = nullptr;
    delete gTangentCubeMesh; gTangentCubeMesh = nullptr;
    delete gBuildingMesh;    gBuildingMesh = nullptr;
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
    gD3DContext->VSSetShader(gDefaultVertexShader, nullptr, 0);
    gD3DContext->PSSetShader(gDefaultPixelShader,  nullptr, 0);
    
    // States - no blending, normal depth buffer and culling
    gD3DContext->OMSetBlendState(gNoBlendingState, nullptr, 0xffffff);
    gD3DContext->OMSetDepthStencilState(gUseDepthBufferState, 0);
    gD3DContext->RSSetState(gCullBackState);

    // Select the approriate textures and sampler to use in the pixel shader
    gD3DContext->PSSetSamplers(0, 1, &gAnisotropic4xSampler);

    // Render model - it will update the model's world matrix and send it to the GPU in a constant buffer, then it will call
    // the Mesh render function, which will set up vertex & index buffer before finally calling Draw on the GPU
    for (int i = 0; i < NUM_MODELS; i++)
    {
        if (gModels[i]->renderMode == Default)
        {
            gD3DContext->PSSetShaderResources(0, 1, &gModels[i]->texture->diffuseSpecularMapSRV);
            gModels[i]->model->Render();
        }
    }

    gD3DContext->PSSetShader(gBrightPixelShader, nullptr, 0);
    for (int i = 0; i < NUM_MODELS; i++)
    {
        if (gModels[i]->renderMode == Bright)
        {
            gD3DContext->PSSetShaderResources(0, 1, &gModels[i]->texture->diffuseSpecularMapSRV);
            gModels[i]->model->Render();
        }
    }

    gD3DContext->PSSetShader(gTexFadePixelShader, nullptr, 0);
    for (int i = 0; i < NUM_MODELS; i++)
    {
        if (gModels[i]->renderMode == TextureFade)
        {
            gD3DContext->PSSetShaderResources(0, 1, &gModels[i]->texture->diffuseSpecularMapSRV);
            gD3DContext->PSSetShaderResources(4, 1, &gModels[i]->texture2->diffuseSpecularMapSRV);
            gModels[i]->model->Render();
        }
    }

    gD3DContext->PSSetShader(gTextureGradientPixelShader, nullptr, 0);
    for (int i = 0; i < NUM_MODELS; i++)
    {
        if (gModels[i]->renderMode == TextureGradient || gModels[i]->renderMode == TexGradientNS)
        {
            gD3DContext->PSSetShaderResources(0, 1, &gModels[i]->texture->diffuseSpecularMapSRV);
            gD3DContext->PSSetShaderResources(4, 1, &gModels[i]->texture2->diffuseSpecularMapSRV);
            gModels[i]->model->Render();
        }
    }

    gD3DContext->VSSetShader(gWiggleVertexShader, nullptr, 0);
    gD3DContext->PSSetShader(gWigglePixelShader, nullptr, 0);
    for (int i = 0; i < NUM_MODELS; i++)
    {
        if (gModels[i]->renderMode == Wiggle)
        {
            gD3DContext->PSSetShaderResources(0, 1, &gModels[i]->texture->diffuseSpecularMapSRV);
            gModels[i]->model->Render();
        }
    }

    gD3DContext->VSSetShader(gNormalMappingVertexShader, nullptr, 0);
    gD3DContext->PSSetShader(gNormalMappingPixelShader, nullptr, 0);
    for (int i = 0; i < NUM_MODELS; i++)
    {
        if (gModels[i]->renderMode == NormalMap)
        {
            gD3DContext->PSSetShaderResources(0, 1, &gModels[i]->texture->diffuseSpecularMapSRV);
            gD3DContext->PSSetShaderResources(1, 1, &gModels[i]->texture->normalMapSRV);
            gModels[i]->model->Render();
        }
    }

    gD3DContext->PSSetShader(gParallaxMappingPixelShader, nullptr, 0);
    for (int i = 0; i < NUM_MODELS; i++)
    {
        if (gModels[i]->renderMode == ParallaxMap)
        {
            gD3DContext->PSSetShaderResources(0, 1, &gModels[i]->texture->diffuseSpecularMapSRV);
            gD3DContext->PSSetShaderResources(1, 1, &gModels[i]->texture->normalMapSRV);
            gModels[i]->model->Render();
        }
    }

    gD3DContext->PSSetSamplers(0, 1, &gCubeMapSampler);
    gD3DContext->RSSetState(gCullNoneState);
    gD3DContext->PSSetShader(gCubeMapPixelShader, nullptr, 0);
    for (int i = 0; i < NUM_MODELS; i++)
    {
        if (gModels[i]->renderMode == CubeMap)
        {
            gD3DContext->PSSetShaderResources(0, 1, &gModels[i]->texture->diffuseSpecularMapSRV);
            gModels[i]->model->Render();
        }
    }

    gD3DContext->PSSetShader(gCubeMapLightPixelShader, nullptr, 0);
    for (int i = 0; i < NUM_MODELS; i++)
    {
        if (gModels[i]->renderMode == CubeMapLight)
        {
            gD3DContext->PSSetShaderResources(0, 1, &gModels[i]->texture->diffuseSpecularMapSRV);
            gModels[i]->model->Render();
        }
    }

    gD3DContext->PSSetShader(gCubeMapAnimatedPixelShader, nullptr, 0);
    for (int i = 0; i < NUM_MODELS; i++)
    {
        if (gModels[i]->renderMode == CubeMapAnimated)
        {
            gD3DContext->PSSetShaderResources(0, 1, &gModels[i]->texture->diffuseSpecularMapSRV);
            gD3DContext->PSSetShaderResources(4, 1, &gModels[i]->texture2->diffuseSpecularMapSRV);
            gModels[i]->model->Render();
        }
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

    //// Render transparent objects ////

    gD3DContext->PSSetShader(gAlphaPixelShader, nullptr, 0);
    for (int i = 0; i < NUM_MODELS; i++)
    {
        if (gModels[i]->renderMode == AddBlend)
        {
            gD3DContext->PSSetShaderResources(0, 1, &gModels[i]->texture->diffuseSpecularMapSRV);
            gModels[i]->model->Render();
        }
    }

    gD3DContext->OMSetBlendState(gAlphaBlendingState, nullptr, 0xffffff);
    for (int i = 0; i < NUM_MODELS; i++)
    {
        if (gModels[i]->renderMode == AlphBlend)
        {
            gD3DContext->PSSetShaderResources(0, 1, &gModels[i]->texture->diffuseSpecularMapSRV);
            gModels[i]->model->Render();
        }
    }

    gD3DContext->OMSetBlendState(gMultiplicativeBlendingState, nullptr, 0xffffff);
    for (int i = 0; i < NUM_MODELS; i++)
    {
        if (gModels[i]->renderMode == MultBlend)
        {
            gD3DContext->PSSetShaderResources(0, 1, &gModels[i]->texture->diffuseSpecularMapSRV);
            gModels[i]->model->Render();
        }
    }
    
    gD3DContext->VSSetShader(gDefaultVertexShader, nullptr, 0);
    gD3DContext->PSSetShader(gAlphaLightingPixelShader, nullptr, 0);
    gD3DContext->OMSetBlendState(gAdditiveBlendingState, nullptr, 0xffffff);
    for (int i = 0; i < NUM_MODELS; i++)
    {
        if (gModels[i]->renderMode == AddBlendLight || gModels[i]->renderMode == Ghost)
        {
            gD3DContext->PSSetShaderResources(0, 1, &gModels[i]->texture->diffuseSpecularMapSRV);
            gModels[i]->model->Render();
        }
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

    gPerFrameConstants.parallaxDepth = 0.08f;

    //***************************************//
    //// Render from light's point of view ////
    
    for (int i = 0; i < NUM_SPOTLIGHTS; i++)
    {
        gSpotlights[i].RenderFromLightPOV(NUM_MODELS, gModels);
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
    ID3D11ShaderResourceView* colourMaps[NUM_SPOTLIGHTS];
    
    for (int i = 0; i < NUM_SPOTLIGHTS; i++)
    {
        shadowMaps[i] = gSpotlights[i].shadowMapSRV;
        colourMaps[i] = gSpotlights[i].colourMapSRV;
    }

    gD3DContext->PSSetShaderResources(10, NUM_SPOTLIGHTS, shadowMaps);
    gD3DContext->PSSetShaderResources(30, NUM_SPOTLIGHTS, colourMaps);
    gD3DContext->PSSetSamplers(1, 1, &gPointSampler);

    // Render the scene for the main window
    RenderSceneFromCamera(gCamera);

    // Unbind shadow maps from shaders - prevents warnings from DirectX when we try to render to the shadow maps again next frame
    for (unsigned int i = 0; i < NUM_SPOTLIGHTS; i++)
    {
        shadowMaps[i] = nullptr;
        colourMaps[i] = nullptr;
    }

    gD3DContext->PSSetShaderResources(10, NUM_SPOTLIGHTS, shadowMaps);
    gD3DContext->PSSetShaderResources(30, NUM_SPOTLIGHTS, colourMaps);

    //*****************************//
    // Temporary demonstration code for visualising the light's view of the scene
    //ColourRGBA white = {1,1,1};
    //gD3DContext->ClearRenderTargetView(gBackBufferRenderTarget, &white.r);
    //gSpotlights[2].RenderShadowMap(NUM_MODELS, gModels);
    //gD3DContext->ClearRenderTargetView(gBackBufferRenderTarget, &white.r);
    //gSpotlights[2].RenderColourMap(NUM_MODELS, gModels);
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
    // Light effects
    for (int i = 0; i < NUM_SPOTLIGHTS; i++)
    {
        gSpotlights[i].Update(frameTime);
    }
    for (int i = 0; i < NUM_POINTLIGHTS; i++)
    {
        gPointlights[i].Update(frameTime);
    }

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