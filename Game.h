#pragma once

#include "Mesh.h"
#include "BufferStructs.h"

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

	// Done in Update()
	void StartImGuiUpdate(float deltaTime);
	void BuildCustomUI(float deltaTime);
	
	// Done in Draw()
	void FrameStart();
	void DrawAllMeshes();
	void RenderImGui();
	void FrameEnd();

	// Values that can be changed through ImGui
	float backgroundColor[4] = { 0.4f, 0.6f, 0.75f, 1.0f };
	bool showImGuiDemoWindow = false;
	VertexShaderData vsData = { // Struct that will be passed to GPU via constBuffer
		DirectX::XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f), // colorTint - what color do we want the geometry's colors to be multiplied by?
		DirectX::XMFLOAT3(-0.25f, 0.0f, 0.0f) }; // offset - where should all geomerty be shifted to?

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
};

