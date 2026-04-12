#ifndef __GGP_SHADER_INCLUDES__
#define __GGP_SHADER_INCLUDES__
#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2

// Struct representing data from vertex shader to pixel shader
struct VertexToPixel
{
    float4 screenPosition : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float3 worldPosition : POSITION;
    float3 tangent : TANGENT;
};


struct Light
{
    int Type;
    float3 Direction;
    float Range;
    float3 Position;
    float Intensity;
    float3 Color;
    float SpotInnerAngle;
    float SpotOuterAngle;
    float2 Padding;
};

// Diffuse calculation
float3 Diffuse(float3 normal, float3 lightDir, float3 lightColor, float intensity, float3 surfaceColor)
{
    float diff = saturate(dot(normal, normalize(-lightDir)));
    return diff * lightColor * intensity * surfaceColor;
}

// Specular calculation (Phong)
float3 Specular(float3 normal, float3 lightDir, float3 worldPos, float3 camPos, float3 lightColor, float intensity)
{
    float3 V = normalize(camPos - worldPos);
    float3 R = reflect(normalize(lightDir), normal);
    float spec = pow(saturate(dot(R, V)), 128.0f);
    return spec * lightColor * intensity;
}

// Directional light
float3 DirectionalLight(Light light, float3 normal, float3 worldPos, float3 camPos, float3 surfaceColor)
{
    float3 diffuse = Diffuse(normal, light.Direction, light.Color, light.Intensity, surfaceColor);
    float3 specular = Specular(normal, light.Direction, worldPos, camPos, light.Color, light.Intensity);
    return diffuse + specular;
}

float Attenuate(Light light, float3 worldPos)
{
    float dist = distance(light.Position, worldPos);
    float att = saturate(1.0f - (dist * dist / (light.Range * light.Range)));
    return att * att;
}

// Point light
float3 PointLight(Light light, float3 normal, float3 worldPos, float3 camPos, float3 surfaceColor)
{
    // direction from the surface to light
    float3 lightDir = normalize(worldPos - light.Position);
    
    float3 diffuse = Diffuse(normal, lightDir, light.Color, light.Intensity, surfaceColor);
    float3 specular = Specular(normal, lightDir, worldPos, camPos, light.Color, light.Intensity);
    
    //attenuate both
    float att = Attenuate(light, worldPos);
    return (diffuse + specular) * att;
}

float3 SpotLight(Light light, float3 normal, float3 worldPos, float3 camPos, float3 surfaceColor)
{
    float3 lightDir = normalize(worldPos - light.Position);
    
    float3 diffuse = Diffuse(normal, lightDir, light.Color, light.Intensity, surfaceColor);
    float3 specular = Specular(normal, lightDir, worldPos, camPos, light.Color, light.Intensity);
    float att = Attenuate(light, worldPos);
    
    //cone attenuation
    float cosOuter = cos(light.SpotOuterAngle);
    float cosInner = cos(light.SpotInnerAngle);
    float cosAngle = dot(normalize(light.Direction), lightDir);
    float spotAtt = saturate((cosAngle - cosOuter) / (cosInner - cosOuter));
    
    return (diffuse + specular) * att * spotAtt;
}

#endif