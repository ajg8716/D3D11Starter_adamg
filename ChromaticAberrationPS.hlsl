// Chromatic Aberration post-process
//
// Simulates lens colour fringing: each RGB channel is sampled at a
// slightly different UV position.  The offset grows toward screen edges,
// matching real optical dispersion.
//
// Constant buffer layout must match ChromaCB in Game.cpp exactly.

cbuffer ChromaCB : register(b0)
{
    float strength; // 0 = no effect; slider max ~0.03
    float2 texelSize; // (1/width, 1/height)  (kept for parity; not used here)
    float padding;
};

Texture2D sceneTexture : register(t0);
SamplerState clampSampler : register(s0);

struct VertexToPixel
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float4 main(VertexToPixel input) : SV_TARGET
{
    if (strength <= 0.0f)
        return sceneTexture.Sample(clampSampler, input.uv);

    // Direction from screen centre; magnitude increases toward corners
    float2 dir = input.uv - float2(0.5f, 0.5f);

    // R shifted outward, B shifted inward, G stays centred
    float r = sceneTexture.Sample(clampSampler, saturate(input.uv + dir * strength)).r;
    float g = sceneTexture.Sample(clampSampler, input.uv).g;
    float b = sceneTexture.Sample(clampSampler, saturate(input.uv - dir * strength)).b;
    float a = sceneTexture.Sample(clampSampler, input.uv).a;

    return float4(r, g, b, a);
}
