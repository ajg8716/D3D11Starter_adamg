#pragma once
#include <DirectXMath.h> 
#include "lights.h"

struct VertexShaderExternalData
{
	//DirectX::XMFLOAT4 colorTint; // 16 bytes - color multiplier
	DirectX::XMFLOAT4X4 world; // 64 bytes - world transform matrix
	DirectX::XMFLOAT4X4 worldInvTranspose;
	//float padding;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
	DirectX::XMFLOAT4X4 lightView;
	DirectX::XMFLOAT4X4 lightProjection;
};

struct PixelShaderExternalData
{
	DirectX::XMFLOAT4 colorTint; // 16 bytes - color multiplier
	DirectX::XMFLOAT2 uvScale;
	DirectX::XMFLOAT2 uvOffset;
	DirectX::XMFLOAT3 ambientColor;
	float padding;
	DirectX::XMFLOAT3 cameraPosition;
	float padding2;
	Light lights[5];
};