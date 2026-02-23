#pragma once
#include <DirectXMath.h> 

struct VertexShaderExternalData
{
	DirectX::XMFLOAT4 colorTint; // 16 bytes - color multiplier
	DirectX::XMFLOAT4X4 world; // 64 bytes - world transform matrix
	//float padding;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
};