//------------------------------------------------------------------------------------------------
struct vs_input_t
{
	float3 localPosition : POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
	float3 localNormal : NORMAL;
};

//------------------------------------------------------------------------------------------------
struct v2p_t
{
	float4 position : SV_Position;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float4 worldPosition : POSITION;
};

struct SpotLight
{
	float3 lightPosition;
	float lightConeHalfAngle;
	float3 lightDirection;
	float4 lightColor;
};

cbuffer LightingConstants : register(b1)
{
	float3 SunDirection;
	float SunIntensity;
	float AmbientIntensity;
	SpotLight SpotLightInfo;
};

//------------------------------------------------------------------------------------------------
cbuffer CameraConstants : register(b2)
{
	float4x4 ProjectionMatrix;
	float4x4 ViewMatrix;
};

//------------------------------------------------------------------------------------------------
cbuffer ModelConstants : register(b3)
{
	float4x4 ModelMatrix;
	float4 ModelColor;
};

cbuffer EffectConstants : register(b8)
{
	
};

//------------------------------------------------------------------------------------------------
Texture2D diffuseTexture : register(t0);
SamplerState diffuseSampler : register(s0);

float GetSpotLightIntensity(float3 position)
{
	float minCos = cos(SpotLightInfo.lightConeHalfAngle);
	float maxCos = (minCos + 1.f) / 2.f;
	float3 lightVector = normalize(SpotLightInfo.lightPosition - position);
	float angleBetweenPointAndLight = dot(SpotLightInfo.lightDirection, -lightVector);
	return smoothstep(minCos, maxCos, angleBetweenPointAndLight);
}

//------------------------------------------------------------------------------------------------
v2p_t VertexMain(vs_input_t input)
{
	float4 worldNormal = mul(ModelMatrix, float4(input.localNormal, 0));
	float4 localPosition = float4(input.localPosition, 1);
	float4 worldPosition = mul(ModelMatrix, localPosition);
	float4 viewPosition = mul(ViewMatrix, worldPosition);
	float4 clipPosition = mul(ProjectionMatrix, viewPosition);


	v2p_t v2p;
	v2p.position = clipPosition;
	v2p.color = (input.color * ModelColor);
	v2p.uv = input.uv;
	v2p.normal = worldNormal;
	v2p.worldPosition = worldPosition;
	return v2p;
}

//------------------------------------------------------------------------------------------------
float4 PixelMain(v2p_t input) : SV_Target0
{
	float4 diffuseColor = diffuseTexture.Sample(diffuseSampler, input.uv);
	float4 color = (diffuseColor * input.color);
	return float4(color);
}
