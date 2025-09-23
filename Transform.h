#pragma once

#include<DirectXMath.h>

class Transform
{
public:

	Transform();
	~Transform();

	void SetScale(float x, float y, float z);
	void SetScale(DirectX::XMFLOAT3);
	void SetPitchYawRoll(float p, float y, float r);
	void SetPitchYawRoll(DirectX::XMFLOAT3);
	void SetTranslation(float x, float y, float z);
	void SetTranslation(DirectX::XMFLOAT3);

	void ScaleBy(float x, float y, float z);
	void ScaleBy(DirectX::XMFLOAT3);
	void RotateBy(float p, float y, float r);
	void RotateBy(DirectX::XMFLOAT3);
	void TranslateBy(float x, float y, float z);
	void TranslateBy(DirectX::XMFLOAT3);

	DirectX::XMFLOAT3 GetScale();
	DirectX::XMFLOAT3 GetPitchYawRoll();
	DirectX::XMFLOAT3 GetTranslation();

	DirectX::XMFLOAT4X4 GetWorldMatrix();

private:

	bool worldMatrixHasChanged;

	DirectX::XMFLOAT3 scale;
	DirectX::XMFLOAT3 pitchYawRoll;
	DirectX::XMFLOAT3 translation;

	DirectX::XMFLOAT4X4 world;
};

