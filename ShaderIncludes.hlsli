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
    float4 shadowPos : SHADOW_POSITION;
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
    bool CastsShadows;
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



static const float PI = 3.14159265359f;

// Normal Distribution Function (GGX/Trowbridge-Reitz)
float D_GGX(float3 normal, float3 halfVec, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = saturate(dot(normal, halfVec));
    float denom = (NdotH * NdotH) * (a2 - 1.0f) + 1.0f;
    return a2 / (PI * denom * denom);
}

// Fresnel (Schlick approximation)
float3 F_Schlick(float3 v, float3 halfVec, float3 f0)
{
    float VdotH = saturate(dot(v, halfVec));
    return f0 + (1.0f - f0) * pow(1.0f - VdotH, 5.0f);
}

// Geometry / Shadowing (Schlick-GGX)
float G_SchlickGGX(float3 normal, float3 dir, float roughness)
{
    float k = (roughness + 1.0f) * (roughness + 1.0f) / 8.0f;
    float NdotD = saturate(dot(normal, dir));
    return NdotD / (NdotD * (1.0f - k) + k);
}

float G_Smith(float3 normal, float3 v, float3 l, float roughness)
{
    return G_SchlickGGX(normal, v, roughness) * G_SchlickGGX(normal, l, roughness);
}

// Full Cook-Torrance specular BRDF
float3 CookTorrance(float3 normal, float3 v, float3 l, float roughness, float3 f0, out float3 kS)
{
    float3 h = normalize(v + l);
    float3 F = F_Schlick(v, h, f0);
    float D = D_GGX(normal, h, roughness);
    float G = G_Smith(normal, v, l, roughness);

    kS = F;
    float NdotL = saturate(dot(normal, l));
    float NdotV = saturate(dot(normal, v));
    float denom = 4.0f * NdotV * NdotL + 0.0001f;
    return (D * F * G) / denom;
}

// PBR directional light
float3 DirectionalLightPBR(Light light, float3 normal, float3 worldPos, float3 camPos,
    float3 albedo, float roughness, float metalness)
{
    float3 V = normalize(camPos - worldPos);
    float3 L = normalize(-light.Direction);
    float3 f0 = lerp(float3(0.04f, 0.04f, 0.04f), albedo, metalness);

    float3 kS;
    float3 spec = CookTorrance(normal, V, L, roughness, f0, kS);
    float3 kD = (1.0f - kS) * (1.0f - metalness);

    float NdotL = saturate(dot(normal, L));
    return (kD * albedo / PI + spec) * light.Color * light.Intensity * NdotL;
}

// PBR point light
float3 PointLightPBR(Light light, float3 normal, float3 worldPos, float3 camPos,
    float3 albedo, float roughness, float metalness)
{
    float3 V = normalize(camPos - worldPos);
    float3 L = normalize(light.Position - worldPos);
    float3 f0 = lerp(float3(0.04f, 0.04f, 0.04f), albedo, metalness);

    float3 kS;
    float3 spec = CookTorrance(normal, V, L, roughness, f0, kS);
    float3 kD = (1.0f - kS) * (1.0f - metalness);

    float NdotL = saturate(dot(normal, L));
    float att = Attenuate(light, worldPos);
    return (kD * albedo / PI + spec) * light.Color * light.Intensity * NdotL * att;
}

// PBR spot light
float3 SpotLightPBR(Light light, float3 normal, float3 worldPos, float3 camPos,
    float3 albedo, float roughness, float metalness)
{
    float3 L = normalize(light.Position - worldPos);
    float cosOuter = cos(light.SpotOuterAngle);
    float cosInner = cos(light.SpotInnerAngle);
    float cosAngle = dot(normalize(light.Direction), -L);
    float spotAtt = saturate((cosAngle - cosOuter) / (cosInner - cosOuter));
    float att = Attenuate(light, worldPos);

    float3 V = normalize(camPos - worldPos);
    float3 f0 = lerp(float3(0.04f, 0.04f, 0.04f), albedo, metalness);

    float3 kS;
    float3 spec = CookTorrance(normal, V, L, roughness, f0, kS);
    float3 kD = (1.0f - kS) * (1.0f - metalness);

    float NdotL = saturate(dot(normal, L));
    return (kD * albedo / PI + spec) * light.Color * light.Intensity * NdotL * att * spotAtt;
}

#endif