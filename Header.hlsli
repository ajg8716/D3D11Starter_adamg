
struct VertexToPixel
{
    float4 screenPosition : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float3 worldPosition : POSITION;
    float3 tangent : TANGENT; // ADD THIS
};