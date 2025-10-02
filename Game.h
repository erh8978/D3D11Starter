#pragma once

#include "Mesh.h"
#include "BufferStructs.h"
#include "GameEntity.h"
#include "Camera.h"

#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
#include <vector>

class Game
{
public:
	// Basic OOP setup
	Game();
	~Game();
	Game(const Game&) = delete; // Remove copy constructor
	Game& operator=(const Game&) = delete; // Remove copy-assignment operator

	// Primary functions
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);
	void OnResize();

private:

	// Initialization helper methods - feel free to customize, combine, remove, etc.
	void LoadShaders();
	void CreateGeometry();
	void InitializeConstantBuffer();
	void CreateStartingCameras();

	// Done in Update()
	void UpdateCameras(float deltaTime);
	void StartImGuiUpdate(float deltaTime);
	void BuildCustomUI(float deltaTime);
	
	// Done in Draw()
	void FrameStart();
	void DrawAllGameEntities();
	void SendDataToConstantBuffer(DirectX::XMFLOAT4X4 worldMatrix, DirectX::XMFLOAT4X4 projectionMatrix, DirectX::XMFLOAT4X4 viewMatrix);
	void RenderImGui();
	void FrameEnd();

	// Done in OnResize()
	void UpdateAllCameraProjectionMatrices(float aspectRatio);

	// Values that can be changed through ImGui
	float backgroundColor[4] = { 0.4f, 0.6f, 0.75f, 1.0f };
	bool showImGuiDemoWindow = false;
	DirectX::XMFLOAT4 colorTint = DirectX::XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	VertexShaderData vsData; // Struct that will be passed to GPU via constBuffer

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Shaders and shader-related constructs
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> constBuffer;

	// Meshes
	std::vector<std::shared_ptr<Mesh>> meshes;
	// GameEntities
	std::vector<std::shared_ptr<GameEntity>> gameEntities;
	// Cameras
	std::vector<std::shared_ptr<Camera>> cameras;
	int currentCameraIndex = 0;
};

