#pragma once
#include <memory>
#include "Mesh.h"
#include "Transform.h"

class GameEntity
{
public:
	GameEntity(std::shared_ptr<Mesh> mesh);

	std::shared_ptr<Mesh> GetMesh();
	Transform* GetTransform();

	void Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context);

private:
	std::shared_ptr<Mesh> mesh;
	Transform transform;
};

