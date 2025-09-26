#pragma once

#include<DirectXMath.h>

class Transform
{
public:

	Transform();
	~Transform();

	void SetScale(float x, float y, float z);
	void SetScale(DirectX::XMFLOAT3 newScale);
	void SetPitchYawRoll(float p, float y, float r);
	void SetPitchYawRoll(DirectX::XMFLOAT3 newPitchYawRoll);
	void SetTranslation(float x, float y, float z);
	void SetTranslation(DirectX::XMFLOAT3 newTranslation);

	void Scale(float x, float y, float z);
	void Scale(DirectX::XMFLOAT3 input);
	void Rotate(float p, float y, float r);
	void Rotate(DirectX::XMFLOAT3 input);
	void MoveAbsolute(float x, float y, float z);
	void MoveAbsolute(DirectX::XMFLOAT3 input);

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
	DirectX::XMFLOAT4X4 worldInverseTranspose;
};

