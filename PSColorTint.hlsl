cbuffer ExternalData : register(b0)
{
    float4 colorTint;
    float4x4 worldMatrix;
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
}

struct VertexToPixel
{
    float4 screenPosition : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

float4 main(VertexToPixel input) : SV_TARGET
{
    return colorTint;
}
