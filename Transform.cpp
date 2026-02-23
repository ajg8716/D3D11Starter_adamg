#include "Transform.h"

using namespace DirectX;

Transform::Transform()
	: position(0, 0, 0),
	rotation(0, 0, 0),
	scale(1, 1, 1),
	dirty(true)
{
	XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&worldInverseTransposeMatrix, XMMatrixIdentity());
}

// Setters
// -----------------------------------------------------------------
void Transform::SetPosition(float x, float y, float z)
{
	position = XMFLOAT3(x, y, z);
	dirty = true;
}

void Transform::SetPosition(XMFLOAT3 position)
{
	this->position = position;
	dirty = true;
}

void Transform::SetRotation(float pitch, float yaw, float roll)
{
	rotation = XMFLOAT3(pitch, yaw, roll);
	dirty = true;
}

void Transform::SetRotation(XMFLOAT3 rotation)
{
	this->rotation = rotation;
	dirty = true;
}

void Transform::SetScale(float x, float y, float z)
{
	scale = XMFLOAT3(x, y, z);
	dirty = true;
}

void Transform::SetScale(XMFLOAT3 scale)
{
	this->scale = scale;
	dirty = true;
}

// Getters
// ---------------------------------------------------------------------


XMFLOAT3 Transform::GetPosition()
{
	return position;
}

XMFLOAT3 Transform::GetPitchYawRoll()
{
	return rotation;
}

XMFLOAT3 Transform::GetScale()
{
	return scale;
}

XMFLOAT4X4 Transform::GetWorldMatrix()
{
	if (dirty) UpdateMatrices();
	return worldMatrix;
}

XMFLOAT4X4 Transform::GetWorldInverseTransposeMatrix()
{
	if (dirty) UpdateMatrices();
	return worldInverseTransposeMatrix;
}

// Transformers
// -------------------------------------------------------------

void Transform::MoveAbsolute(float x, float y, float z)
{
	position.x += x;
	position.y += y;
	position.z += z;
	dirty = true;
}

void Transform::MoveRelative(float x, float y, float z)
{
	// put te desired movement in a vector
	XMVECTOR movement = XMVectorSet(x, y, z, 0.0f);

	// crate a quaternion from the current pitch/yaw/roll
	XMVECTOR rotQuat = XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);

	// rotate the movement direction by the transform's orientation
	XMVECTOR relativeMovement = XMVector3Rotate(movement, rotQuat);

	//add the rotated movement to the current position
	XMVECTOR pos = XMLoadFloat3(&position);
	XMStoreFloat3(&position, XMVectorAdd(pos, relativeMovement));
	dirty = true;
}

void Transform::MoveRelative(XMFLOAT3 offset)
{
	// Just call the other MoveRelative function to avoid code duplication
	MoveRelative(offset.x, offset.y, offset.z);
}

XMFLOAT3 Transform::GetRight() {
	// The right vector in world space is the local right vector (1, 0, 0) rotated by the transform's orientation
	XMVECTOR worldRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR rotQuat = XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
	XMFLOAT3 result;
	XMStoreFloat3(&result, XMVector3Rotate(worldRight, rotQuat));
	return result;
}

XMFLOAT3 Transform::GetUp() {
	// The up vector in world space is the local up vector (0, 1, 0) rotated by the transform's orientation
	XMVECTOR worldUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR rotQuat = XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
	XMFLOAT3 result;
	XMStoreFloat3(&result, XMVector3Rotate(worldUp, rotQuat));
	return result;
}

XMFLOAT3 Transform::GetForward() {
	// The forward vector in world space is the local forward vector (0, 0, 1) rotated by the transform's orientation
	XMVECTOR worldForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	XMVECTOR rotQuat = XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
	XMFLOAT3 result;
	XMStoreFloat3(&result, XMVector3Rotate(worldForward, rotQuat));
	return result;
}

void Transform::MoveAbsolute(XMFLOAT3 offset)
{
	position.x += offset.x;
	position.y += offset.y;
	position.z += offset.z;
	dirty = true;
}

void Transform::Rotate(float pitch, float yaw, float roll)
{
	rotation.x += pitch;
	rotation.y += yaw;
	rotation.z += roll;
	dirty = true;
}

void Transform::Rotate(XMFLOAT3 rotation)
{
	this->rotation.x += rotation.x;
	this->rotation.y += rotation.y;
	this->rotation.z += rotation.z;
	dirty = true;
}

void Transform::Scale(float x, float y, float z)
{
	scale.x *= x;
	scale.y *= y;
	scale.z *= z;
	dirty = true;
}

void Transform::Scale(XMFLOAT3 scale)
{
	this->scale.x *= scale.x;
	this->scale.y *= scale.y;
	this->scale.z *= scale.z;
	dirty = true;
}

// Private
// ---------------------------------------------------

void Transform::UpdateMatrices()
{
	XMMATRIX s = XMMatrixScaling(scale.x, scale.y, scale.z);
	XMMATRIX r = XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
	XMMATRIX t = XMMatrixTranslation(position.x, position.y, position.z);

	XMMATRIX world = s * r * t;

	XMStoreFloat4x4(&worldMatrix, world);
	XMStoreFloat4x4(&worldInverseTransposeMatrix,
		XMMatrixInverse(0, XMMatrixTranspose(world)));

	dirty = false;
}