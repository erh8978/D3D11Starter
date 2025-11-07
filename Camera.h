#pragma once
#include "Camera.h"
#include "Input.h"
#include "Transform.h"

#include <DirectXMath.h>

class Camera
{
public:

	Camera(DirectX::XMFLOAT3 initialPosition,
		float aspectRatio,
		float fieldOfViewDegrees = 45.0f,
		float initialMovementSpeed = 1.0f,
		float initialMouseLookSpeed = 0.005f,
		float farClipPlane = 1000.0f,
		float nearClipPlane = 0.0001f);
	~Camera();

	DirectX::XMFLOAT4X4 GetViewMatrix();
	DirectX::XMFLOAT4X4 GetProjectionMatrix();
	DirectX::XMFLOAT3 GetTranslation();
	DirectX::XMFLOAT3 GetPitchYawRoll();
	float GetFovDegrees();

	void SetTranslation(DirectX::XMFLOAT3);
	void SetPitchYawRoll(DirectX::XMFLOAT3);

	void UpdateProjectionMatrix(float aspectRatio);

	void Update(float deltaTime);

private:

	Transform myTransform;

	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;

	bool viewMatrixHasChanged;
	bool projectionMatrixHasChanged;

	float fieldOfViewRadians;
	float nearClipDistance;
	float farClipDistance;
	float movementSpeed;
	float mouseLookSpeed;
	bool isPerspective;

	void UpdateViewMatrix();
};
