#include "Mesh.h"


Mesh::Mesh(
	Vertex* vertices,
	int vertexCount,
	unsigned int* indices,
	int indexCount,
	Microsoft::WRL::ComPtr<ID3D11Device> device)
	:vertexCount(vertexCount), indexCount(indexCount)
{
	// create the vertex buffer
	D3D11_BUFFER_DESC vbd = {};
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * vertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initialVertexData = {};
	initialVertexData.pSysMem = vertices;

	device->CreateBuffer(&vbd, &initialVertexData, vertexBuffer.GetAddressOf());

	// create the index buffer
	D3D11_BUFFER_DESC ibd = {};
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(unsigned int) * indexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initialIndexData = {};
	initialIndexData.pSysMem = indices;

	device->CreateBuffer(&ibd, &initialIndexData, indexBuffer.GetAddressOf());
}

// destructor - empty because ComPtrs handle cleanup
Mesh::~Mesh() {

}

// getters
Microsoft::WRL::ComPtr<ID3D11Buffer> Mesh::GetVertexBuffer() const 
{
	return vertexBuffer;
}
Microsoft::WRL::ComPtr<ID3D11Buffer> Mesh::GetIndexBuffer() const
{
	return indexBuffer;
}

int Mesh::GetIndexCount() const 
{
	return indexCount;
}

int Mesh::GetVertexCount() const 
{
	return vertexCount;
}

// draw method - sets the vertex and index buffer, then draws
void Mesh::Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context)
{
	// set the vertex buffer
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);

	// set the index buffer
	context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// draw the mesh using the index buffer
	context->DrawIndexed(indexCount, 0, 0);
}