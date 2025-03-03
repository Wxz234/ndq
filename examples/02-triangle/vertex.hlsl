[RootSignature("RootFlags(0)")]
float4 main(uint vertexId : SV_VertexID) : SV_POSITION
{
    float2 positions[3] = { float2(0.0f, 0.5f), float2(0.5f, -0.5f), float2(-0.5f, -0.5f) };
    return float4(positions[vertexId], 0.0f, 1.0f);
}