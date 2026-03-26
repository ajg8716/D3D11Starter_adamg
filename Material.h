#pragma once
#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <unordered_map>
#include "Graphics.h"

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
    DirectX::XMFLOAT2 GetUVScale() const;
    DirectX::XMFLOAT2 GetUVOffset() const;

    // Setters
    void SetColorTint(DirectX::XMFLOAT4 tint);
    void SetVertexShader(Microsoft::WRL::ComPtr<ID3D11VertexShader> vs);
    void SetPixelShader(Microsoft::WRL::ComPtr<ID3D11PixelShader> ps);
    void SetUVScale(DirectX::XMFLOAT2 scale);
    void SetUVOffset(DirectX::XMFLOAT2 offset);

    // Texture/Sampler methods                
    void AddTextureSRV(unsigned int slot, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv);
    void AddSampler(unsigned int slot, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler);
    void BindTexturesAndSamplers(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context);

private:
    DirectX::XMFLOAT4 colorTint;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;

    DirectX::XMFLOAT2 uvScale = { 1.0f, 1.0f };
    DirectX::XMFLOAT2 uvOffset = { 0.0f, 0.0f };
    std::unordered_map<unsigned int, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureSRVs;
    std::unordered_map<unsigned int, Microsoft::WRL::ComPtr<ID3D11SamplerState>> samplers;
};

