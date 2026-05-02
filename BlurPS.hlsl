// Gaussian Blur
cbuffer BlurCB : register(b0)
{
    int blurRadius; // 0 = no blur; slider max ~10
    float2 texelSize; // (1/width, 1/height)
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
    // Radius of 0 -> identity copy, zero cost
    if (blurRadius <= 0)
        return sceneTexture.Sample(clampSampler, input.uv);

    float4 colorSum = float4(0, 0, 0, 0);
    float weightSum = 0.0f;

    // sigma heuristic: half the radius so weights fall off nicely
    float sigma = max((float) blurRadius * 0.5f, 0.5f);

    for (int y = -blurRadius; y <= blurRadius; y++)
    {
        for (int x = -blurRadius; x <= blurRadius; x++)
        {
            float2 offset = float2((float) x, (float) y) * texelSize;
            float2 sampleUV = saturate(input.uv + offset);

            // Gaussian weight based on 2-D distance from center
            float dist = sqrt((float) (x * x + y * y));
            float weight = exp(-(dist * dist) / (2.0f * sigma * sigma));

            colorSum += sceneTexture.Sample(clampSampler, sampleUV) * weight;
            weightSum += weight;
        }
    }

    return colorSum / weightSum;
}