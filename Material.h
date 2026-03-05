#pragma once
#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>

class Material
{
public:
    Material(
        DirectX::XMFLOAT4 colorTint,
        Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader,
        Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader
    );

    // Getters
    DirectX::XMFLOAT4 GetColorTint() const;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> GetVertexShader() const;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> GetPixelShader() const;

    // Setters
    void SetColorTint(DirectX::XMFLOAT4 tint);
    void SetVertexShader(Microsoft::WRL::ComPtr<ID3D11VertexShader> vs);
    void SetPixelShader(Microsoft::WRL::ComPtr<ID3D11PixelShader> ps);

private:
    DirectX::XMFLOAT4 colorTint;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
};

