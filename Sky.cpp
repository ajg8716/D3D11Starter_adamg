#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

#include "Sky.h"
#include "Graphics.h"
#include "PathHelpers.h"
#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>

using namespace DirectX;

Sky::Sky(
    std::shared_ptr<Mesh> mesh,
    Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler,
    const wchar_t* right, const wchar_t* left,
    const wchar_t* up, const wchar_t* down,
    const wchar_t* front, const wchar_t* back,
    const wchar_t* vsPath, const wchar_t* psPath)
    : mesh(mesh), sampler(sampler)
{
    // Load the 6 faces into a cube map SRV
    skySRV = CreateCubemap(right, left, up, down, front, back);

    // Load sky vertex shader
    {
        ID3DBlob* blob = nullptr;
        D3DReadFileToBlob(FixPath(vsPath).c_str(), &blob);
        Graphics::Device->CreateVertexShader(
            blob->GetBufferPointer(), blob->GetBufferSize(),
            0, skyVS.GetAddressOf());

        D3D11_INPUT_ELEMENT_DESC skyLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        Graphics::Device->CreateInputLayout(
            skyLayout, 1,
            blob->GetBufferPointer(), blob->GetBufferSize(),
            skyInputLayout.GetAddressOf());

        if (FAILED(hr) || !skyInputLayout)
            OutputDebugStringW(L"Sky input layout FAILED\n");

        blob->Release();
    }

    // Load sky pixel shader
    {
        ID3DBlob* blob = nullptr;
        D3DReadFileToBlob(FixPath(psPath).c_str(), &blob);
        Graphics::Device->CreatePixelShader(
            blob->GetBufferPointer(), blob->GetBufferSize(),
            0, skyPS.GetAddressOf());
        blob->Release();
    }

    // Rasterizer state: draw back faces (we're inside the cube)
    D3D11_RASTERIZER_DESC rastDesc = {};
    rastDesc.FillMode = D3D11_FILL_SOLID;
    rastDesc.CullMode = D3D11_CULL_FRONT;  // cull FRONT so inside of cube is visible
    Graphics::Device->CreateRasterizerState(&rastDesc, skyRasterState.GetAddressOf());

    // Depth stencil state: pass depth test when depth == 1.0 (far plane)
    D3D11_DEPTH_STENCIL_DESC depthDesc = {};
    depthDesc.DepthEnable = true;
    depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL; // sky passes at z == 1.0
    Graphics::Device->CreateDepthStencilState(&depthDesc, skyDepthState.GetAddressOf());

    // Constant buffer for sky VS (view + projection matrices)
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = sizeof(SkyVSData);
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    Graphics::Device->CreateBuffer(&cbDesc, nullptr, skyVSConstantBuffer.GetAddressOf());
}

Sky::~Sky() {}

void Sky::Draw(std::shared_ptr<Camera> camera)
{
    // Save the current input layout before overriding it
    Microsoft::WRL::ComPtr<ID3D11InputLayout> previousLayout;
    Graphics::Context->IAGetInputLayout(previousLayout.GetAddressOf());

    // Set sky-specific render states
    Graphics::Context->RSSetState(skyRasterState.Get());
    Graphics::Context->OMSetDepthStencilState(skyDepthState.Get(), 0);

    // Set sky shaders
    Graphics::Context->VSSetShader(skyVS.Get(), 0, 0);
    Graphics::Context->PSSetShader(skyPS.Get(), 0, 0);

    // Bind cube map texture and sampler to pixel shader
    Graphics::Context->PSSetShaderResources(0, 1, skySRV.GetAddressOf());
    Graphics::Context->PSSetSamplers(0, 1, sampler.GetAddressOf());

    // Upload view + projection matrices to the sky VS constant buffer
    // Note: strip translation from view matrix so sky never moves
    SkyVSData data = {};
    data.view = camera->GetViewMatrix();
    data.view._14 = 0; data.view._24 = 0; data.view._34 = 0;
    data.projection = camera->GetProjectionMatrix();

    D3D11_MAPPED_SUBRESOURCE mapped = {};
    Graphics::Context->Map(skyVSConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, &data, sizeof(SkyVSData));
    Graphics::Context->Unmap(skyVSConstantBuffer.Get(), 0);

    Graphics::Context->VSSetConstantBuffers(0, 1, skyVSConstantBuffer.GetAddressOf());
    Graphics::Context->IASetInputLayout(skyInputLayout.Get());

    // Draw the cube mesh
    mesh->Draw(Graphics::Context);

    // Restore default render states so regular geometry draws correctly
    Graphics::Context->RSSetState(nullptr);
    Graphics::Context->OMSetDepthStencilState(nullptr, 0);
    Graphics::Context->IASetInputLayout(previousLayout.Get()); // restore main layout
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Sky::CreateCubemap(
    const wchar_t* right, const wchar_t* left,
    const wchar_t* up, const wchar_t* down,
    const wchar_t* front, const wchar_t* back)
{
    // Load 6 textures from files into a temporary array
    const wchar_t* files[6] = { right, left, up, down, front, back };
    ID3D11Texture2D* textures[6] = {};
    D3D11_TEXTURE2D_DESC faceDesc = {};

    for (int i = 0; i < 6; i++)
    {
        ID3D11Resource* res = nullptr;
        DirectX::CreateWICTextureFromFileEx(
            Graphics::Device.Get(), Graphics::Context.Get(),
            FixPath(files[i]).c_str(),
            0,
            D3D11_USAGE_STAGING,          // CPU-readable staging resource
            0,                            // no bind flags (staging)
            D3D11_CPU_ACCESS_READ,
            0,
            WIC_LOADER_FORCE_RGBA32,      // ensure consistent format
            &res, nullptr);

        // Cast to Texture2D and grab its description
        textures[i] = static_cast<ID3D11Texture2D*>(res);
        if (i == 0) textures[0]->GetDesc(&faceDesc);
    }

    // Create the actual cube map texture (6 array slices, IsCubemap = true)
    D3D11_TEXTURE2D_DESC cubeDesc = {};
    cubeDesc.Width = faceDesc.Width;
    cubeDesc.Height = faceDesc.Height;
    cubeDesc.MipLevels = 1;
    cubeDesc.ArraySize = 6;
    cubeDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    cubeDesc.SampleDesc.Count = 1;
    cubeDesc.Usage = D3D11_USAGE_DEFAULT;
    cubeDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    cubeDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> cubeTexture;
    Graphics::Device->CreateTexture2D(&cubeDesc, nullptr, cubeTexture.GetAddressOf());

    // Copy each face from its staging texture into the correct cube map slice
    for (int i = 0; i < 6; i++)
    {
        // Map the staging texture to read its pixel data
        D3D11_MAPPED_SUBRESOURCE mapped = {};
        Graphics::Context->Map(textures[i], 0, D3D11_MAP_READ, 0, &mapped);

        // Copy into cube map subresource (slice i, mip 0)
        UINT subresource = D3D11CalcSubresource(0, i, 1);
        Graphics::Context->UpdateSubresource(
            cubeTexture.Get(), subresource, nullptr,
            mapped.pData, mapped.RowPitch, 0);

        Graphics::Context->Unmap(textures[i], 0);
        textures[i]->Release();
    }

    // Create the shader resource view for the cube map
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = cubeDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.TextureCube.MipLevels = 1;
    srvDesc.TextureCube.MostDetailedMip = 0;

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
    Graphics::Device->CreateShaderResourceView(cubeTexture.Get(), &srvDesc, srv.GetAddressOf());
    return srv;
}
