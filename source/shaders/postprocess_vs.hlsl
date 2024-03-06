struct VSQuadOut
{
    float4 position : SV_Position;
    float2 texcoord: TEXCOORD0;
};

VSQuadOut main(uint vertexID: SV_VertexID)
{
    VSQuadOut output;
    output.texcoord = float2((vertexID << 1) & 2, vertexID & 2);
    output.position = float4(output.texcoord * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
    return output;
}