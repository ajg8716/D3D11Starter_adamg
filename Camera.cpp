#include "Camera.h"
#include "Input.h" // ingine's input class

using namespace DirectX;

Camera::Camera(
	float x, float y, float z,
	float moveSpeed,
	float lookSpeed,
	float fov,
	float aspectRatio,
	float nearClip,
	float farClip)
	: moveSpeed(moveSpeed),
	lookSpeed(lookSpeed),
	fov(fov),
	nearClip(nearClip),
	farClip(farClip)
{
	transform.SetPosition(x, y, z);

	XMStoreFloat4x4(&viewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&projectionMatrix, XMMatrixIdentity());

	UpdateViewMatrix();
	UpdateProjectionMatrix(aspectRatio);
}

Camera::~Camera(){}

/// <summary>
/// called every frame - processes input and updates the view matrix accordingly
/// </summary>
/// <param name="deltaTime"></param>
void Camera::Update(float deltaTime)
{
	ProcessInput(deltaTime);
	UpdateViewMatrix();
}

void Camera::UpdateViewMatrix()
{
	// get direction vectors from the transform
	XMFLOAT3 pos = transform.GetPosition();
	XMFLOAT3 forward = transform.GetForward();
	XMFLOAT3 up = transform.GetUp();

	XMMATRIX view = XMMatrixLookToLH(
		XMLoadFloat3(&pos),
		XMLoadFloat3(&forward),
		XMLoadFloat3(&up));

	XMStoreFloat4x4(&viewMatrix, view);
}

void Camera::UpdateProjectionMatrix(float aspectRatio)
{
	XMMATRIX proj = XMMatrixPerspectiveFovLH(fov, aspectRatio, nearClip, farClip);
	XMStoreFloat4x4(&projectionMatrix, proj);
}

void Camera::ProcessInput(float deltaTime)
{
	// -- Keyboard input --
	float speed = moveSpeed * deltaTime;
	
	// Movement
	if (Input::KeyDown('W')) transform.MoveRelative(0, 0, speed);
	if (Input::KeyDown('S')) transform.MoveRelative(0, 0, -speed);
	if (Input::KeyDown('A')) transform.MoveRelative(-speed, 0, 0);
	if (Input::KeyDown('D')) transform.MoveRelative(speed, 0, 0);
	if (Input::KeyDown(VK_SPACE)) transform.MoveRelative(0, speed, 0);
	if (Input::KeyDown('X')) transform.MoveRelative(0, -speed, 0);

	// --- Mouse look (only while right mouse button is held) ---
	if (Input::MouseRightDown())
	{
		float mouseDX = Input::GetMouseXDelta() * lookSpeed * deltaTime;
		float mouseDY = Input::GetMouseYDelta() * lookSpeed * deltaTime;

		// Yaw (left/right) and pitch (up/down)
		transform.Rotate(mouseDY, mouseDX, 0.0f);

		// Clamp pitch to prevent flipping upside down
		XMFLOAT3 rot = transform.GetPitchYawRoll();
		if (rot.x > XM_PIDIV2 - 0.01f) transform.SetRotation(XM_PIDIV2 - 0.01f, rot.y, rot.z);
		if (rot.x < -XM_PIDIV2 + 0.01f) transform.SetRotation(-XM_PIDIV2 + 0.01f, rot.y, rot.z);
	}
}

// ----------------------------------------
// Getters
// ----------------------------------------
XMFLOAT4X4 Camera::GetViewMatrix() { return viewMatrix; }
XMFLOAT4X4 Camera::GetProjectionMatrix() { return projectionMatrix; }
Transform& Camera::GetTransform() { return transform; }
float Camera::GetFOV() { return fov; }
float Camera::GetMoveSpeed() { return moveSpeed; }
float Camera::GetLookSpeed() { return lookSpeed; }
