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

cbuffer BattleConcentrationLineShaderParam : register(b4)
{
	float Progress;
	float Strength;
	float2 TexelSize;
	float2 FocusCenter;
	float FocusRadius;
	float RayCount;
	float Time;
	float Padding;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	float4 color = Texture0.Sample(Sampler0, input.UV);
	float2 pixelDelta = (input.UV - FocusCenter) / TexelSize;
	float distanceFromFocus = length(pixelDelta);
	float radialMask = smoothstep(
		FocusRadius * 0.7,
		max(FocusRadius, 1.0),
		distanceFromFocus
	);
	float angle = atan2(pixelDelta.y, pixelDelta.x);
	float rays = abs(cos(angle * RayCount + Time * 0.18));
	float lineMask = smoothstep(0.72, 0.98, pow(rays, 8.0));
	float fade = sin(saturate(Progress) * 3.14159265);
	float intensity = lineMask * radialMask * fade * Strength * 0.18;
	float3 lineColor = float3(0.2, 0.2, 0.2);
	return float4(color.rgb + lineColor * intensity, color.a);
}
