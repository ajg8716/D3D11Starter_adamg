#include "Material.h"

Material::Material(
    DirectX::XMFLOAT4 colorTint,
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader,
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader)
    : colorTint(colorTint),
    vertexShader(vertexShader),
    pixelShader(pixelShader)
{
}

DirectX::XMFLOAT4 Material::GetColorTint() const { return colorTint; }
Microsoft::WRL::ComPtr<ID3D11VertexShader> Material::GetVertexShader() const { return vertexShader; }
Microsoft::WRL::ComPtr<ID3D11PixelShader> Material::GetPixelShader() const { return pixelShader; }

void Material::SetColorTint(DirectX::XMFLOAT4 tint) { colorTint = tint; }
void Material::SetVertexShader(Microsoft::WRL::ComPtr<ID3D11VertexShader> vs) { vertexShader = vs; }
void Material::SetPixelShader(Microsoft::WRL::ComPtr<ID3D11PixelShader> ps) { pixelShader = ps; }

DirectX::XMFLOAT2 Material::GetUVScale()  const { return uvScale; }
DirectX::XMFLOAT2 Material::GetUVOffset() const { return uvOffset; }
void Material::SetUVScale(DirectX::XMFLOAT2 scale) { uvScale = scale; }
void Material::SetUVOffset(DirectX::XMFLOAT2 offset) { uvOffset = offset; }

void Material::AddTextureSRV(unsigned int slot, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv)
{
    textureSRVs[slot] = srv;
}

void Material::AddSampler(unsigned int slot, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler)
{
    samplers[slot] = sampler;
}

void Material::BindTexturesAndSamplers(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context)
{
    for (auto& t : textureSRVs)
        Graphics::Context->PSSetShaderResources(t.first, 1, t.second.GetAddressOf());
    for (auto& s : samplers)
        Graphics::Context->PSSetSamplers(s.first, 1, s.second.GetAddressOf());
}