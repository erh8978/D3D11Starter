#include "Transform.h"
#include<DirectXMath.h>

using namespace DirectX;

Transform::Transform() :
	scale{ XMFLOAT3(1.0f, 1.0f, 1.0f) },
	pitchYawRoll { XMFLOAT3(0.0f, 0.0f, 0.0f) },
	translation { XMFLOAT3(0.0f, 0.0f, 0.0f) },
	right { XMFLOAT3(1.0f, 0.0f, 0.0f) },
	up { XMFLOAT3(0.0f, 1.0f, 0.0f) },
	forward { XMFLOAT3(0.0f, 0.0f, 1.0f) }
{
	XMStoreFloat4x4(&world, XMMatrixIdentity());
	XMStoreFloat4x4(&worldInverseTranspose, XMMatrixIdentity());

	worldMatrixHasChanged = true;
	rotationHasChanged = true;
}

Transform::~Transform()
{
}

void Transform::SetScale(float x, float y, float z)
{
	XMFLOAT3 input(x, y, z);

	// Other function does the same thing, so just call it to prevent code duplication
	SetScale(input);
}

void Transform::SetScale(DirectX::XMFLOAT3 newScale)
{
	scale = newScale;
	worldMatrixHasChanged = true;
}

void Transform::SetPitchYawRoll(float p, float y, float r)
{
	XMFLOAT3 input(p, y, r);

	// Other function does the same thing, so just call it to prevent code duplication
	SetPitchYawRoll(input);
}

void Transform::SetPitchYawRoll(DirectX::XMFLOAT3 newPitchYawRoll)
{
	pitchYawRoll = newPitchYawRoll;
	worldMatrixHasChanged = true;
	rotationHasChanged = true;
}

void Transform::SetTranslation(float x, float y, float z)
{
	XMFLOAT3 input(x, y, z);
	
	// Other function does the same thing, so just call it to prevent code duplication
	SetTranslation(input);
}

void Transform::SetTranslation(DirectX::XMFLOAT3 newTranslation)
{
	translation = newTranslation;
	worldMatrixHasChanged = true;
}

void Transform::Scale(float x, float y, float z)
{
	XMFLOAT3 input(x, y, z);

	// Other function does the same thing, so just call it to prevent code duplication
	Scale(input);
}

void Transform::Scale(DirectX::XMFLOAT3 input)
{
	XMVECTOR original = XMLoadFloat3(&scale);
	XMVECTOR newScale = XMLoadFloat3(&input);

	XMStoreFloat3(&scale, XMVectorAdd(original, newScale));

	worldMatrixHasChanged = true;
}

void Transform::Rotate(float p, float y, float r)
{
	XMFLOAT3 input(p, y, r);

	// Other function does the same thing, so just call it to prevent code duplication
	Rotate(input);
}

void Transform::Rotate(DirectX::XMFLOAT3 input)
{
	XMVECTOR original = XMLoadFloat3(&pitchYawRoll);
	XMVECTOR newRotation = XMLoadFloat3(&input);

	XMStoreFloat3(&pitchYawRoll, XMVectorAdd(original, newRotation));

	worldMatrixHasChanged = true;
	rotationHasChanged = true;
}

void Transform::MoveAbsolute(float x, float y, float z)
{
	XMFLOAT3 input(x, y, z);

	// Other function does the same thing, so just call it to prevent code duplication
	MoveAbsolute(input);
}

void Transform::MoveAbsolute(DirectX::XMFLOAT3 input)
{
	XMVECTOR original = XMLoadFloat3(&translation);
	XMVECTOR newTranslation = XMLoadFloat3(&input);

	XMStoreFloat3(&translation, XMVectorAdd(original, newTranslation));

	worldMatrixHasChanged = true;
}

void Transform::MoveRelative(float x, float y, float z)
{
	XMFLOAT3 offset(x, y, z);
	
	// Other function does the same thing, so just call it to prevent code duplication
	MoveRelative(offset);
}

void Transform::MoveRelative(DirectX::XMFLOAT3 offset)
{
	// Create a quaternion representing the current rotation
	XMVECTOR rotationQuaternion = XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&pitchYawRoll));

	// Load XMVECTOR from offset XMFLOAT3, rotate it by the quaternion, and add the original translation
	XMVECTOR tr = XMVectorAdd(XMLoadFloat3(&translation), XMVector3Rotate(XMLoadFloat3(&offset), rotationQuaternion));

	// Store new value into translation
	XMStoreFloat3(&translation, tr);

	worldMatrixHasChanged = true;
}

DirectX::XMFLOAT3 Transform::GetScale()
{
	return scale;
}

DirectX::XMFLOAT3 Transform::GetPitchYawRoll()
{
	return pitchYawRoll;
}

DirectX::XMFLOAT3 Transform::GetTranslation()
{
	return translation;
}

DirectX::XMFLOAT3 Transform::GetRight()
{
	if (rotationHasChanged)
	{
		RecalculateDirectionVectors();
	}

	return right;
}

DirectX::XMFLOAT3 Transform::GetUp()
{
	if (rotationHasChanged)
	{
		RecalculateDirectionVectors();
	}

	return up;
}

DirectX::XMFLOAT3 Transform::GetForward()
{
	if (rotationHasChanged)
	{
		RecalculateDirectionVectors();
	}

	return forward;
}

DirectX::XMFLOAT4X4 Transform::GetWorldMatrix()
{
	if (worldMatrixHasChanged) {
		
		XMMATRIX w = XMMatrixIdentity();

		XMMATRIX s = XMMatrixScalingFromVector(XMLoadFloat3(&scale));
		XMMATRIX r = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&pitchYawRoll));
		XMMATRIX t = XMMatrixTranslationFromVector(XMLoadFloat3(&translation));

		// Step 1: Scale
		w = XMMatrixMultiply(w, s);
		// Step 2: Rotate
		w = XMMatrixMultiply(w, r);
		// Step 3: Translate
		w = XMMatrixMultiply(w, t);

		// Store the result
		XMStoreFloat4x4(&world, w);
		XMStoreFloat4x4(&worldInverseTranspose,
			XMMatrixInverse(0, XMMatrixTranspose(w)));

		// And reset the change flag
		worldMatrixHasChanged = false;
	}

	return world;
}

DirectX::XMFLOAT4X4 Transform::GetWorldInvTranspose()
{
	// Use GetWorldMatrix without storing the result to update worldInvTranspose if necessary.
	GetWorldMatrix();

	return worldInverseTranspose;
}

void Transform::RecalculateDirectionVectors()
{
	// Reset vectors to world right, up, and forward
	right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	forward = XMFLOAT3(0.0f, 0.0f, 1.0f);

	// Create a quaternion representing the current rotation
	XMVECTOR rotationQuaternion = XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&pitchYawRoll));

	// Load XMVECTORs from XMFLOAT3s, then rotate those by the quaternion
	XMVECTOR r = XMVector3Rotate(XMLoadFloat3(&right), rotationQuaternion);
	XMVECTOR u = XMVector3Rotate(XMLoadFloat3(&up), rotationQuaternion);
	XMVECTOR f = XMVector3Rotate(XMLoadFloat3(&forward), rotationQuaternion);

	// Store new values into original XMFLOAT3s
	XMStoreFloat3(&right, r);
	XMStoreFloat3(&up, u);
	XMStoreFloat3(&forward, f);

	rotationHasChanged = false;
}