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
    float scale = 6.0f;
    float2 scaledUV = input.uv * scale;
    float checker = fmod(floor(scaledUV.x) + floor(scaledUV.y), 2.0f);
    float4 colorA = float4(0.1f, 0.1f, 0.4f, 1.0f);
    float4 colorB = float4(0.9f, 0.8f, 0.2f, 1.0f);

    float4 patternColor = lerp(colorA, colorB, checker);

    float3 n = normalize(input.normal);
    patternColor.rgb += n.y * 0.15f;

    return patternColor * colorTint;
}
