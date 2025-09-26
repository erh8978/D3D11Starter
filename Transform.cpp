#include "Transform.h"
#include<DirectXMath.h>

using namespace DirectX;

Transform::Transform() :
	scale{ XMFLOAT3(1.0f, 1.0f, 1.0f) },
	pitchYawRoll { XMFLOAT3(0.0f, 0.0f, 0.0f) },
	translation { XMFLOAT3(0.0f, 0.0f, 0.0f) }
{
	XMStoreFloat4x4(&world, XMMatrixIdentity());
	XMStoreFloat4x4(&worldInverseTranspose, XMMatrixIdentity());

	worldMatrixHasChanged = true;
}

Transform::~Transform()
{
}

void Transform::SetScale(float x, float y, float z)
{
	scale = XMFLOAT3(x, y, z);
	worldMatrixHasChanged = true;
}

void Transform::SetScale(DirectX::XMFLOAT3 newScale)
{
	scale = newScale;
	worldMatrixHasChanged = true;
}

void Transform::SetPitchYawRoll(float p, float y, float r)
{
	pitchYawRoll = XMFLOAT3(p, y, r);
	worldMatrixHasChanged = true;
}

void Transform::SetPitchYawRoll(DirectX::XMFLOAT3 newPitchYawRoll)
{
	pitchYawRoll = newPitchYawRoll;
	worldMatrixHasChanged = true;
}

void Transform::SetTranslation(float x, float y, float z)
{
	translation = XMFLOAT3(x, y, z);
	worldMatrixHasChanged = true;
}

void Transform::SetTranslation(DirectX::XMFLOAT3 newTranslation)
{
	translation = newTranslation;
	worldMatrixHasChanged = true;
}

void Transform::Scale(float x, float y, float z)
{
	XMFLOAT3 input(x, y, z);

	XMVECTOR original = XMLoadFloat3(&scale);
	XMVECTOR newScale = XMLoadFloat3(&input);

	XMStoreFloat3(&scale, XMVectorAdd(original, newScale));

	worldMatrixHasChanged = true;
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

	XMVECTOR original = XMLoadFloat3(&pitchYawRoll);
	XMVECTOR newRotation = XMLoadFloat3(&input);

	XMStoreFloat3(&pitchYawRoll, XMVectorAdd(original, newRotation));

	worldMatrixHasChanged = true;
}

void Transform::Rotate(DirectX::XMFLOAT3 input)
{
	XMVECTOR original = XMLoadFloat3(&pitchYawRoll);
	XMVECTOR newRotation = XMLoadFloat3(&input);

	XMStoreFloat3(&pitchYawRoll, XMVectorAdd(original, newRotation));

	worldMatrixHasChanged = true;
}

void Transform::MoveAbsolute(float x, float y, float z)
{
	XMFLOAT3 input(x, y, z);

	XMVECTOR original = XMLoadFloat3(&translation);
	XMVECTOR newTranslation = XMLoadFloat3(&input);

	XMStoreFloat3(&translation, XMVectorAdd(original, newTranslation));

	worldMatrixHasChanged = true;
}

void Transform::MoveAbsolute(DirectX::XMFLOAT3 input)
{
	XMVECTOR original = XMLoadFloat3(&translation);
	XMVECTOR newTranslation = XMLoadFloat3(&input);

	XMStoreFloat3(&translation, XMVectorAdd(original, newTranslation));

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
