#pragma once
#include <DirectXMath.h>
#include "Transform.h"
#include <memory>

class Camera {
public: 
	Camera(
		float x, float y, float z,		// position
		float moveSpeed,				// units per second
		float lookSpeed,				// mouse sensitivity
		float fov,						// field of view in radians
		float aspectRatio,				// width / height of the viewport
		float nearClip = 0.01f,
		float farClip = 100.0f
	);

	~Camera();

	//Update methods
	void Update(float deltaTime);
	void UpdateViewMatrix();
	void UpdateProjectionMatrix(float aspectRatio);

	//Getters
	DirectX::XMFLOAT4X4 GetViewMatrix();
	DirectX::XMFLOAT4X4 GetProjectionMatrix();
	Transform& GetTransform();
	float GetFOV();
	float GetMoveSpeed();
	float GetLookSpeed();

private:
	Transform transform;

	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;

	float moveSpeed;
	float lookSpeed;
	float fov;
	float nearClip;
	float farClip;

	void ProcessInput(float deltaTime);
};