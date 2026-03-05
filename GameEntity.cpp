#include "GameEntity.h"


GameEntity::GameEntity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material)
	: mesh(mesh), material(material)
{
	// Transform default-constructs itself (position 0,0,0 / rotation 0,0,0 / scale 1,1,1)
}

std::shared_ptr<Mesh> GameEntity::GetMesh()
{
	return mesh;
}

std::shared_ptr<Material> GameEntity::GetMaterial()
{
	return material;
}

void GameEntity::SetMaterial(std::shared_ptr<Material> mat)
{
	this->material = mat;
}

Transform* GameEntity::GetTransform()
{
	return &transform;
}

void GameEntity::Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context)
{
	mesh->Draw(context);
}