#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
#include "Mesh.h"
#include "Camera.h"


class Sky
{
public:
    Sky(
        std::shared_ptr<Mesh> mesh,
        Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler,
        const wchar_t* right,
        const wchar_t* left,
        const wchar_t* up,
        const wchar_t* down,
        const wchar_t* front,
        const wchar_t* back,
        const wchar_t* vsPath,
        const wchar_t* psPath
    );
    ~Sky();

    void Draw(std::shared_ptr<Camera> camera);

private:
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>    skySRV;
    Microsoft::WRL::ComPtr<ID3D11SamplerState>          sampler;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState>       skyRasterState;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState>     skyDepthState;

    std::shared_ptr<Mesh>               mesh;
    Microsoft::WRL::ComPtr<ID3D11VertexShader>  skyVS;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>   skyPS;

    // Constant buffer data for the sky vertex shader
    struct SkyVSData
    {
        DirectX::XMFLOAT4X4 view;
        DirectX::XMFLOAT4X4 projection;
    };
    Microsoft::WRL::ComPtr<ID3D11Buffer> skyVSConstantBuffer;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> skyInputLayout;

    // Helper to create the cube map from 6 image files
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateCubemap(
        const wchar_t* right, const wchar_t* left,
        const wchar_t* up, const wchar_t* down,
        const wchar_t* front, const wchar_t* back
    );
};

