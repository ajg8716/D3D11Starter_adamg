TextureCube skyTexture : register(t0);
SamplerState skySampler : register(s0);

struct VertexToPixel_Sky
{
    float4 position : SV_POSITION;
    float3 sampleDir : DIRECTION;
};

float4 main(VertexToPixel_Sky input) : SV_TARGET
{
    return skyTexture.Sample(skySampler, input.sampleDir);
}