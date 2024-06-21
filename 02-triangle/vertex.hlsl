struct PSInput
{
    float4 position : SV_POSITION;
};

PSInput main(float4 position : POSITION)
{
    PSInput result;

    result.position = position;
    
    return result;
}