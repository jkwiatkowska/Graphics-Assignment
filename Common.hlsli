//--------------------------------------------------------------------------------------
// Common include file for all shaders
//--------------------------------------------------------------------------------------
// Using include files to define the type of data passed between the shaders


//--------------------------------------------------------------------------------------
// Shader input / output
//--------------------------------------------------------------------------------------

// The structure below describes the vertex data to be sent into the vertex shader.
struct BasicVertex
{
    float3 position : position;
    float3 normal   : normal;
    float2 uv       : uv;
};


// This structure describes what data the lighting pixel shader receives from the vertex shader.
// The projected position is a required output from all vertex shaders - where the vertex is on the screen
// The world position and normal at the vertex are sent to the pixel shader for the lighting equations.
// The texture coordinates (uv) are passed from vertex shader to pixel shader unchanged to allow textures to be sampled
struct LightingPixelShaderInput
{
    float4 projectedPosition : SV_Position; // This is the position of the pixel to render, this is a required input
                                            // to the pixel shader and so it uses the special semantic "SV_Position"
                                            // because the shader needs to identify this important information
    
    float3 worldPosition : worldPosition;   // The world position and normal of each vertex is passed to the pixel...
    float3 worldNormal   : worldNormal;     //...shader to calculate per-pixel lighting. These will be interpolated
                                            // automatically by the GPU (rasterizer stage) so each pixel will know
                                            // its position and normal in the world - required for lighting equations
    
    float2 uv : uv; // UVs are texture coordinates. The artist specifies for every vertex which point on the texture is "pinned" to that vertex.
};



// This structure is similar to the one above but for the light models, which aren't themselves lit
struct SimplePixelShaderInput
{
    float4 projectedPosition : SV_Position;
    float2 uv : uv;
};

struct Spotlight
{
    float3   position; // 3 floats: x, y z
    float    padding;        // Pad above variable to float4 (HLSL requirement - copied in the the C++ version of this structure)
    float3   colour;
    float    padding2;
    float3   facing;           // Spotlight facing direction (normal)
    float    cosHalfAngle;     // cos(Spot light cone angle / 2). Precalculate in C++ the spotlight angle in this form to save doing in the shader
    float4x4 viewMatrix;       // For shadow mapping we treat lights like cameras so we need camera matrices for them (prepared on the C++ side)
    float4x4 projectionMatrix; // --"--
};

struct Pointlight
{
    float3   position; // 3 floats: x, y z
    float    padding;        // Pad above variable to float4 (HLSL requirement - copied in the the C++ version of this structure)
    float3   colour;
    float    padding2;
};

//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------

// These structures are "constant buffers" - a way of passing variables over from C++ to the GPU
// They are called constants but that only means they are constant for the duration of a single GPU draw call.
// These "constants" correspond to variables in C++ that we will change per-model, or per-frame etc.

// In this exercise the matrices used to position the camera are updated from C++ to GPU every frame along with lighting information
// These variables must match exactly the gPerFrameConstants structure in Scene.cpp
cbuffer PerFrameConstants : register(b0) // The b0 gives this constant buffer the number 0 - used in the C++ code
{
    float4x4 gViewMatrix;
    float4x4 gProjectionMatrix;
    float4x4 gViewProjectionMatrix; // The above two matrices multiplied together to combine their effects

    float gSpotlightNumber;
    float3 pad;
    Spotlight gSpotlights[25];


    float gPointlightNumber;
    float3 pad2;
    Pointlight gPointlights[25];

    float3   gAmbientColour;
    float    gSpecularPower;

    float3   gCameraPosition;
    float    padding5;
}
// Note constant buffers are not structs: we don't use the name of the constant buffer, these are really just a collection of global variables (hence the 'g')



// If we have multiple models then we need to update the world matrix from C++ to GPU multiple times per frame because we
// only have one world matrix here. Because this data is updated more frequently it is kept in a different buffer for better performance.
// We also keep other data that changes per-model here
// These variables must match exactly the gPerModelConstants structure in Scene.cpp
cbuffer PerModelConstants : register(b1) // The b1 gives this constant buffer the number 1 - used in the C++ code
{
    float4x4 gWorldMatrix;

    float3   gObjectColour;
    float    padding6;  // See notes on padding in structure above
}