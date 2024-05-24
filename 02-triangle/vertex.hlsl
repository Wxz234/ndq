cbuffer PerFrame : register(b0)
{
    float4x4 viewProjectionMatrix;
    float3 lightDirection;
    float pad;
};

float4 main( float4 pos : POSITION ) : SV_POSITION
{
	return float4(lightDirection,1);
}