#include "ShaderIncludes.hlsli"

Texture2D SurfaceTexture : register(t0);
SamplerState BasicSampler : register(s0);
Texture2D Albedo : register(t0);
Texture2D NormalMap : register(t1);
Texture2D RoughnessMap : register(t2);
Texture2D MetalnessMap : register(t3);
Texture2D ShadowMap : register(t4);
SamplerComparisonState ShadowSampler : register(s1);


cbuffer ExternalData : register(b0)
{
    float4 colorTint;
    float2 uvScale;
    float2 uvOffset;
    float3 ambientColor;
    float padding;
    float3 cameraPosition;
    float padding2;
    Light lights[5];
}

// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
//struct VertexToPixel
//{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	//float4 screenPosition	: SV_POSITION;
	//float3 normal			: NORMAL;
    //float2 uv				: TEXCOORD;
//};

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	// Just return the input color
	// - This color (like most values passing through the rasterizer) is 
	//   interpolated for each pixel between the corresponding vertices 
	//   of the triangle we're rendering
    //return float4(input.uv, 0, 1);
	
    //float3 n = normalize(input.normal);
    //n = n * 0.5f + 0.5f;
    //return float4(n, 1);
    //return float4(cameraPosition, 1);
	
    //input.normal = normalize(input.normal);
    //return float4(input.normal, 1);
    
    //return float4(dirLight1.Color, 1);
    //input.normal = normalize(input.normal);
    
    
    
    // build TBN matrix 
    float3 N = normalize(input.normal);
    float3 T = normalize(input.tangent);
    T = normalize(T - dot(T, N) * N); // make tangent orthogonal to normal
    float3 B = cross(T, N);
    float3x3 TBN = float3x3(T, B, N);    
    
    float2 uv = input.uv * uvScale + uvOffset;
    float3 normalFromMap = NormalMap.Sample(BasicSampler, uv).rgb * 2.0f - 1.0f;
    
    input.normal = normalize(mul(normalFromMap, TBN));
    
    float4 surfaceColor = SurfaceTexture.Sample(BasicSampler, uv); 
    surfaceColor = float4(pow(abs(surfaceColor.rgb), 2.2f), surfaceColor.a);  

    float roughness = RoughnessMap.Sample(BasicSampler, uv).r;
    float metalness = MetalnessMap.Sample(BasicSampler, uv).r;
    
    //shadow calculations
    float2 shadowUV = input.shadowPos.xy / input.shadowPos.w;
    shadowUV = shadowUV * 0.5f + 0.5f;
    shadowUV.y = 1.0f - shadowUV.y; 
    float depthFromLight = input.shadowPos.z / input.shadowPos.w;
    float shadowAmount = ShadowMap.SampleCmpLevelZero(ShadowSampler, shadowUV, depthFromLight - 0.005f);
    
    
    // Diffuse for directional light
    //float3 lightDir = normalize(-dirLight1.Direction);
    //float diffuse = saturate(dot(input.normal, lightDir));
    //float3 diffuseColor = diffuse * dirLight1.Color * dirLight1.Intensity * surfaceColor.rgb;
    
    //Specular
    //float3 V = normalize(cameraPosition - input.worldPosition); //direction to camera
    //float3 R = reflect(normalize(dirLight1.Direction), input.normal); //reflection vector
    //float spec = pow(saturate(dot(R, V)), 128.0f); // 128 = shininess exponent
    //float3 specularColor = spec * dirLight1.Color * dirLight1.Intensity;
    
    
    // final result - not tinting specular by surface color
    //float3 totalLight = ambient + diffuseColor + specularColor;
    float3 totalLight = float3(0,0,0);
    
    // loop through all lights
    for (int i = 0; i < 5; i++)
    {
        switch (lights[i].Type)
        {
            case LIGHT_TYPE_DIRECTIONAL:
                totalLight += DirectionalLightPBR(lights[i], input.normal,
                input.worldPosition, cameraPosition, surfaceColor.rgb, roughness, metalness) * (lights[i].CastsShadows ? shadowAmount : 1.0f);
                break;
            case LIGHT_TYPE_POINT:
                totalLight += PointLightPBR(lights[i], input.normal,
                input.worldPosition, cameraPosition, surfaceColor.rgb, roughness, metalness);
                break;
            case LIGHT_TYPE_SPOT:
                totalLight += SpotLightPBR(lights[i], input.normal,
                input.worldPosition, cameraPosition, surfaceColor.rgb, roughness, metalness);
                break;
        }
    }
    
    // Gama correction - convert from linear space to gamma space for correct display on monitors
    float3 gammaCorrect = pow(abs(totalLight), 1.0f / 2.2f);
    return float4(gammaCorrect, 1);
}