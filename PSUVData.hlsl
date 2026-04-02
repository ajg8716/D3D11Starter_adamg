#include "ShaderIncludes.hlsli"

cbuffer ExternalData : register(b0)
{
    float4 colorTint;
    float4x4 worldMatrix;
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
}

//struct VertexToPixel
//{
    //float4 screenPosition : SV_POSITION;
    //float3 normal : NORMAL;
    //float2 uv : TEXCOORD;
//};

float4 main(VertexToPixel input) : SV_TARGET
{
    // Visualize UV coordinates as color:
    // U maps to Red, V maps to Green, Blue is always 0
    return float4(input.uv, 0.0f, 1.0f);
}
