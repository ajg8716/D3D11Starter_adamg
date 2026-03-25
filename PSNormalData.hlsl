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
    // Normals range from -1 to 1, but colors range from 0 to 1.
    // Remap: multiply by 0.5 and add 0.5 to shift the range.
    float3 n = normalize(input.normal);
    n = n * 0.5f + 0.5f;
    return float4(n, 1.0f);
}
