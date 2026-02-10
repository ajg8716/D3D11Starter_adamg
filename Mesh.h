#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include "Vertex.h"
#include "Graphics.h"

class Mesh
{
public: 
	// constructor 
	Mesh(
		Vertex* vertices,
		int vertexCount,
		unsigned int* indices,
		int indexCount,
		Microsoft::WRL::ComPtr<ID3D11Device> device
	);
	// destructor
	~Mesh();

	// getters
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer() const;
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer() const;
	int GetIndexCount() const;
	int GetVertexCount() const;

	// draw method
	void Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context);

private:
	// buffers
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	// counts 
	int indexCount;
	int vertexCount;
};

