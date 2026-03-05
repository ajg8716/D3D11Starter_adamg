#pragma once
#include <memory>
#include "Mesh.h"
#include "Transform.h"
#include "Material.h"

class GameEntity
{
public:
	GameEntity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material);

	std::shared_ptr<Material> GetMaterial();
	void SetMaterial(std::shared_ptr<Material> material);

	std::shared_ptr<Mesh> GetMesh();
	Transform* GetTransform();

	void Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context);

private:
	std::shared_ptr<Material> material;
	std::shared_ptr<Mesh> mesh;
	Transform transform;
};

