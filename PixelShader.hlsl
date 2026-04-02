#include "ShaderIncludes.hlsli"

Texture2D SurfaceTexture : register(t0);
SamplerState BasicSampler : register(s0);

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
    input.normal = normalize(input.normal);
    
    float2 uv = input.uv * uvScale + uvOffset;
    float4 surfaceColor = SurfaceTexture.Sample(BasicSampler, uv);

    float3 ambient = ambientColor * surfaceColor.rgb;
    
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
    float3 totalLight = ambient;
    
    // loop through all lights
    for (int i = 0; i < 5; i++)
    {
        switch (lights[i].Type)
        {
            case LIGHT_TYPE_DIRECTIONAL:
                totalLight += DirectionalLight(lights[i], input.normal,
                input.worldPosition, cameraPosition, surfaceColor.rgb);
                break;
            case LIGHT_TYPE_POINT:
                totalLight += PointLight(lights[i], input.normal,
                input.worldPosition, cameraPosition, surfaceColor.rgb);
                break;
            case LIGHT_TYPE_SPOT:
                totalLight += SpotLight(lights[i], input.normal,
                input.worldPosition, cameraPosition, surfaceColor.rgb);
                break;
        }
    }
    
        return float4(totalLight, 1);
}