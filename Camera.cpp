#include "Camera.h"

#include <algorithm>

using namespace DirectX;

Camera::Camera(DirectX::XMFLOAT3 initialPosition,
	float aspectRatio,
	float fieldOfViewDegrees,
	float initialMovementSpeed,
	float initialMouseLookSpeed,
	float farClipPlane,
	float nearClipPlane) :
	projectionMatrixHasChanged { false },
	viewMatrixHasChanged { false }
{
	// Initialize transform with starting position
	myTransform = Transform();
	myTransform.SetTranslation(initialPosition);

	fieldOfViewRadians = XMConvertToRadians(fieldOfViewDegrees);
	movementSpeed = initialMovementSpeed;
	mouseLookSpeed = initialMouseLookSpeed;
	farClipDistance = farClipPlane;
	nearClipDistance = nearClipPlane;

	isPerspective = true;

	// Do initial matrix updates
	UpdateViewMatrix();
	UpdateProjectionMatrix(aspectRatio);
}

Camera::~Camera()
{

}

DirectX::XMFLOAT4X4 Camera::GetViewMatrix()
{
	return viewMatrix;
}

DirectX::XMFLOAT4X4 Camera::GetProjectionMatrix()
{
	return projectionMatrix;
}

DirectX::XMFLOAT3 Camera::GetTranslation()
{
	return myTransform.GetTranslation();
}

DirectX::XMFLOAT3 Camera::GetPitchYawRoll()
{
	return myTransform.GetPitchYawRoll();
}

float Camera::GetFovDegrees()
{
	return XMConvertToDegrees(fieldOfViewRadians);
}

void Camera::SetPitchYawRoll(XMFLOAT3 input)
{
	myTransform.SetPitchYawRoll(input);
}

void Camera::SetTranslation(XMFLOAT3 input)
{
	myTransform.SetTranslation(input);
}

void Camera::Update(float deltaTime)
{
	float currentMoveSpeed = movementSpeed;
	if (Input::KeyDown(VK_SHIFT)) { currentMoveSpeed *= 5.0f; } // Speed up
	if (Input::KeyDown(VK_CONTROL)) { currentMoveSpeed *= 0.5f; } // Slow down

	// Horizontal movement should be relative to forward vector
	if (Input::KeyDown('W')) { myTransform.MoveRelative(XMFLOAT3(0.0f, 0.0f, currentMoveSpeed * deltaTime)); } // Forward
	if (Input::KeyDown('S')) { myTransform.MoveRelative(XMFLOAT3(0.0f, 0.0f, -currentMoveSpeed * deltaTime)); } // Backward
	if (Input::KeyDown('D')) { myTransform.MoveRelative(XMFLOAT3(currentMoveSpeed * deltaTime, 0.0f, 0.0f)); } // Right
	if (Input::KeyDown('A')) { myTransform.MoveRelative(XMFLOAT3(-currentMoveSpeed * deltaTime, 0.0f, 0.0f)); } // Left
	
	// Vertical movement should be absolute
	if (Input::KeyDown('E')) { myTransform.MoveAbsolute(XMFLOAT3(0.0f, currentMoveSpeed * deltaTime, 0.0f)); } // Up
	if (Input::KeyDown('Q')) { myTransform.MoveAbsolute(XMFLOAT3(0.0f, -currentMoveSpeed * deltaTime, 0.0f)); } // Down

	// Control camera rotation with mouse movement
	if (Input::MouseLeftDown()) {
		float cursorMovementX = Input::GetMouseXDelta() * mouseLookSpeed;
		float cursorMovementY = Input::GetMouseYDelta() * mouseLookSpeed;
		
		myTransform.Rotate(cursorMovementY, cursorMovementX, 0.0f);

		float currentPitch = std::clamp(myTransform.GetPitchYawRoll().x, XMConvertToRadians(-90.0f), XMConvertToRadians(90.0f));
		float currentYaw = myTransform.GetPitchYawRoll().y;
		
		myTransform.SetPitchYawRoll(currentPitch, currentYaw, 0.0f);
	}

	UpdateViewMatrix();
}

void Camera::UpdateProjectionMatrix(float aspectRatio)
{
	// Create a perspective matrix, using left-handed math
	XMMATRIX proj = XMMatrixPerspectiveFovLH(fieldOfViewRadians, aspectRatio, nearClipDistance, farClipDistance);

	XMStoreFloat4x4(&projectionMatrix, proj);
}

void Camera::UpdateViewMatrix()
{
	XMFLOAT3 up(0.0f, 1.0f, 0.0f);
	XMFLOAT3 translation = myTransform.GetTranslation();
	XMFLOAT3 forward = myTransform.GetForward();

	// Create a view matrix using the transform
	XMMATRIX view = XMMatrixLookToLH(XMLoadFloat3(&translation), XMLoadFloat3(&forward), XMLoadFloat3(&up));

	XMStoreFloat4x4(&viewMatrix, view);
}
