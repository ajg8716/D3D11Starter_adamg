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
