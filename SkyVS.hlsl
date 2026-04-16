// SkyVS.hlsl
cbuffer SkyData : register(b0)
{
    matrix view;
    matrix projection;
}

float4 main(float3 pos : POSITION) : SV_POSITION
{
    float4 clipPos = mul(mul(float4(pos, 0.0f), view), projection);
    return clipPos.xyww; // force depth to 1.0 (far plane)
}