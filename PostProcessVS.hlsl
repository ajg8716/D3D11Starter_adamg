// PostProcessVS.hlsl
// Generates a full-screen triangle from vertex ID alone.
// No vertex buffer or input layout is needed.

struct VertexToPixel
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

VertexToPixel main(uint id : SV_VertexID)
{
    VertexToPixel output;
    output.uv = float2((id << 1) & 2, id & 2);
    output.position = float4(output.uv * float2(2, -2) + float2(-1, 1), 0, 1);
    return output;
}
