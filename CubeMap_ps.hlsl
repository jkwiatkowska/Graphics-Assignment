TextureCube CubeMap			 : register(t0);

SamplerState TexSampler      : register(s0);

//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------
float4 main(LightingPixelShaderInput input) : SV_Target
{
	float4 textureColour = CubeMap.Sample(TexSampler, input.worldNormal);

	return float4(textureColour, 1.0f);
}