Texture2D tex : register(t0);
SamplerState samp : register(s0);

struct PSInput
{
    float4 position : SV_Position;
    float2 texcoord: TEXCOORD0;
};

float4 main(PSInput input) : SV_Target
{
    float4 color = tex.Sample(samp, input.texcoord);
    return color;
}
