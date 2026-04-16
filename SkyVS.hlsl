#include "ShaderIncludes.hlsli"

cbuffer SkyData : register(b0)
{
    matrix view;
    matrix projection;
}

struct VertexShaderInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float3 tangent : TANGENT;
};

struct VertexToPixel_Sky
{
    float4 position : SV_POSITION;
    float3 sampleDir : DIRECTION;
};

VertexToPixel_Sky main(VertexShaderInput input)
{
    VertexToPixel_Sky output;

    // Remove translation from view matrix so sky stays centered on camera
    matrix viewNoTranslation = view;
    viewNoTranslation._14 = 0;
    viewNoTranslation._24 = 0;
    viewNoTranslation._34 = 0;

    float4 clipPos = mul(float4(input.position, 1.0f), viewNoTranslation);
    clipPos = mul(clipPos, projection);
    
    output.position = clipPos.xyww; // force depth = 1.0
    output.sampleDir = input.position; // cube vertex positions ARE the sample directions
    return output;
}