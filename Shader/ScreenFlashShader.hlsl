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

cbuffer BattleScreenFlashShaderParam : register(b4)
{
	float4 Color;
	float Progress;
	float3 Padding;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	float4 screen = Texture0.Sample(Sampler0, input.UV);
	float4 flashed = float4(
		lerp(screen.rgb, Color.rgb, Progress * Color.a),
		screen.a
	);
	return float4(flashed.rgb, screen.a);
}
