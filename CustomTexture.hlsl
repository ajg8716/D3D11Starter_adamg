#include "ShaderIncludes.hlsli"

Texture2D Texture1 : register(t0);
Texture2D Texture2 : register(t1);
SamplerState BasicSampler : register(s0);

cbuffer ExternalData : register(b0)
{
    float4 colorTint;
    float2 uvScale;
    float2 uvOffset;
}

//struct VertexToPixel
//{
    //float4 screenPosition : SV_POSITION;
    //float3 normal : NORMAL;
    //float2 uv : TEXCOORD;
//};

float4 main(VertexToPixel input) : SV_TARGET
{
    float2 uv = input.uv * uvScale + uvOffset;
    
    float4 color1 = Texture1.Sample(BasicSampler, uv);
    float4 color2 = Texture2.Sample(BasicSampler, uv);
    
    // Checkerboard pattern between the two
    float checker = fmod(floor(input.uv.x * 4) + floor(input.uv.y * 4), 2.0f);
    float4 blended = lerp(color1, color2, checker);
    
    return blended * colorTint;
}