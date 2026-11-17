Texture2D Texture0 : register(t0);
SamplerState Sampler0 : register(s0);

struct PS_INPUT
{
	float4 Position : SV_POSITION;
	float4 Diffuse : COLOR0;
	float4 Specular : COLOR1;
	float2 UV : TEXCOORD0;
	float2 SubUV : TEXCOORD1;
};

cbuffer BattleBlurShaderParam : register(b4)
{
	float Progress;
	float Strength;
	float2 TexelSize;
	float2 FocusCenter;
	float FocusRadius;
	float Padding;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	float4 center = Texture0.Sample(Sampler0, input.UV);
	float focusDistance = length((input.UV - FocusCenter) / TexelSize);
	float radialBlur = saturate((focusDistance - FocusRadius) / max(FocusRadius, 1.0));
	float blurAmount = saturate(Progress * radialBlur);
	float2 x = float2(TexelSize.x * Strength, 0.0);
	float2 y = float2(0.0, TexelSize.y * Strength);
	float2 diagonal = float2(TexelSize.x, TexelSize.y) * Strength;
	float4 blurred = center * 0.16;
	blurred += Texture0.Sample(Sampler0, input.UV + x) * 0.12;
	blurred += Texture0.Sample(Sampler0, input.UV - x) * 0.12;
	blurred += Texture0.Sample(Sampler0, input.UV + y) * 0.12;
	blurred += Texture0.Sample(Sampler0, input.UV - y) * 0.12;
	blurred += Texture0.Sample(Sampler0, input.UV + diagonal) * 0.09;
	blurred += Texture0.Sample(Sampler0, input.UV - diagonal) * 0.09;
	blurred += Texture0.Sample(Sampler0, input.UV + float2(-diagonal.x, diagonal.y)) * 0.09;
	blurred += Texture0.Sample(Sampler0, input.UV + float2(diagonal.x, -diagonal.y)) * 0.09;
	return lerp(center, blurred, blurAmount);
}
