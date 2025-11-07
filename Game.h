#pragma once

#include "Mesh.h"
#include "BufferStructs.h"
#include "GameEntity.h"
#include "Camera.h"
#include "Lights.h"

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
	Microsoft::WRL::ComPtr<ID3D11VertexShader> LoadVertexShader(const WCHAR* shaderPath);
	Microsoft::WRL::ComPtr<ID3D11PixelShader> LoadPixelShader(const WCHAR* shaderPath);
	void CreateGameEntities();
	void CreateStartingCameras();
	void CreateInitialLights();

	// Done in Update()
	void UpdateCameras(float deltaTime);
	void StartImGuiUpdate(float deltaTime);
	void BuildCustomUI(float deltaTime);
	
	// Done in Draw()
	void FrameStart();
	void DrawAllGameEntities(float totalTime);
	void RenderImGui();
	void FrameEnd();

	// Done in OnResize()
	void UpdateAllCameraProjectionMatrices(float aspectRatio);

	// Values that can be changed through ImGui
	DirectX::XMFLOAT3 defaultBgColor = DirectX::XMFLOAT3(0.2f, 0.2f, 0.5f);
	DirectX::XMFLOAT3 backgroundColor = defaultBgColor;
	bool showImGuiDemoWindow = false;

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Shaders and shader-related constructs
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexShaderConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> pixelShaderConstantBuffer;

	// Meshes
	std::vector<std::shared_ptr<Mesh>> meshes;
	// GameEntities
	std::vector<std::shared_ptr<GameEntity>> gameEntities;
	// Cameras
	std::vector<std::shared_ptr<Camera>> cameras;
	int currentCameraIndex = 0;
	// Lights
	std::vector<Light> lights;
};

