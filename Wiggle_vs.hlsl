#include "Common.hlsli" // Shaders can also use include files - note the extension


//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------
LightingPixelShaderInput main(BasicVertex modelVertex)
{
    LightingPixelShaderInput output;

    float4 modelPosition = float4(modelVertex.position, 1);
    float4 worldPosition = mul(gWorldMatrix, modelPosition);

    float4 modelNormal = float4(modelVertex.normal, 0);
    float4 worldNormal = mul(gWorldMatrix, modelNormal);

    // Distortion
    worldPosition.x += sin(modelPosition.y + gWiggle * 10) * 0.8f;
    worldPosition.y += sin(modelPosition.z + gWiggle * 10) * 0.8f;
    worldPosition.z += sin(modelPosition.x + gWiggle * 10) * 0.8f;
    // Y position
    worldPosition.y += (sin(gWiggle) + 1) * 3;
    // Size
    worldPosition += worldNormal * sin(gWiggle);

    float4 viewPosition = mul(gViewMatrix, worldPosition);
    output.projectedPosition = mul(gProjectionMatrix, viewPosition);

    output.worldNormal = worldNormal.xyz;
    output.worldPosition = worldPosition.xyz;

    output.uv = modelVertex.uv;

    return output; 
}
