// constant buffer for external data from c++
cbuffer ExternalData : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
}

float4 main(float3 position : POSITION) : SV_POSITION
{
    matrix wvp = mul(projection, mul(view, world));
    return mul(wvp, float4(position, 1.0f));
}