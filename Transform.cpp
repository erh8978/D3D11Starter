#include "Transform.h"
#include<DirectXMath.h>

using namespace DirectX;

Transform::Transform() :
	scale{ XMFLOAT3(1.0f, 1.0f, 1.0f) },
	pitchYawRoll { XMFLOAT3(0.0f, 0.0f, 0.0f) },
	translation { XMFLOAT3(0.0f, 0.0f, 0.0f) },
	world { XMFLOAT4X4() }
{
	worldMatrixHasChanged = true;
}

Transform::~Transform()
{
}

void Transform::SetScale(float x, float y, float z)
{
}

void Transform::SetScale(DirectX::XMFLOAT3)
{
}

void Transform::SetPitchYawRoll(float p, float y, float r)
{
}

void Transform::SetPitchYawRoll(DirectX::XMFLOAT3)
{
}

void Transform::SetTranslation(float x, float y, float z)
{
}

void Transform::SetTranslation(DirectX::XMFLOAT3)
{
}

void Transform::ScaleBy(float x, float y, float z)
{
}

void Transform::ScaleBy(DirectX::XMFLOAT3)
{
}

void Transform::RotateBy(float p, float y, float r)
{
}

void Transform::RotateBy(DirectX::XMFLOAT3)
{
}

void Transform::TranslateBy(float x, float y, float z)
{
}

void Transform::TranslateBy(DirectX::XMFLOAT3)
{
}

DirectX::XMFLOAT3 Transform::GetScale()
{
	return DirectX::XMFLOAT3();
}

DirectX::XMFLOAT3 Transform::GetPitchYawRoll()
{
	return DirectX::XMFLOAT3();
}

DirectX::XMFLOAT3 Transform::GetTranslation()
{
	return DirectX::XMFLOAT3();
}

DirectX::XMFLOAT4X4 Transform::GetWorldMatrix()
{
	return DirectX::XMFLOAT4X4();
}
