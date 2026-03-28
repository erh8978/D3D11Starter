#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"
#include "Camera.h"
#include "Transform.h"
#include "Mesh.h"
#include "Material.h"
#include "GameEntity.h"
#include "BufferStructs.h"
#include "RayTracing.h"

#include <DirectXMath.h>
#include <vector>
#include <memory>

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// For the DirectX Math library
using namespace DirectX;

namespace
{
	// Cameras
	std::vector<std::shared_ptr<Camera>> cameras;
	int currentCameraIndex = 0;

	// Meshes
	std::vector<std::shared_ptr<Mesh>> meshes;

	// Materials
	std::vector<std::shared_ptr<Material>> materials;

	// GameEntities
	std::vector<std::shared_ptr<GameEntity>> entities;
	const int NUM_ENTITIES = 10;

	// Lights
	std::vector<Light> lights;
	const unsigned int MAX_LIGHTS = 10;
}

// --------------------------------------------------------
// The constructor is called after the window and graphics API
// are initialized but before the game loop begins
// --------------------------------------------------------
Game::Game()
{
	// Initialize ray tracing
	RayTracing::Initialize(
		Window::Width(),
		Window::Height(),
		FixPath(L"RayTracing.cso"));

	CreateGeometry();
	CreateCameras();
	CreateLights();
}


// --------------------------------------------------------
// Clean up memory or objects created by this class
// 
// Note: Using smart pointers means there probably won't
//       be much to manually clean up here!
// --------------------------------------------------------
Game::~Game()
{
	// Wait for the GPU before we shut down
	Graphics::WaitForGPU();
}


// --------------------------------------------------------
// Creates the geometry we're going to draw
// --------------------------------------------------------
void Game::CreateGeometry()
{
	// Load meshes from .obj files
	meshes.push_back(std::make_shared<Mesh>("Sphere", FixPath(L"../../Assets/Meshes/sphere.obj").c_str()));
	meshes.push_back(std::make_shared<Mesh>("Cube", FixPath(L"../../Assets/Meshes/cube.obj").c_str()));

	// Create materials, and gameEntities using those meshes and materials
	for (unsigned int i = 0; i < NUM_ENTITIES; i++)
	{
		// Make a material based on number of entities
		materials.push_back(std::make_shared<Material>(pipelineState.Get(), XMFLOAT3((float)(i / NUM_ENTITIES), (float)((NUM_ENTITIES - i) / NUM_ENTITIES), (float)sin(i) / 2.0f + 0.5f)));

		// Make a sphere gameEntity from that material
		entities.push_back(std::make_shared<GameEntity>(meshes[0], materials[i]));

		float entitySpace = 1.0f;
		float totalEntitySpace = NUM_ENTITIES * entitySpace;
		float subtractFromXandZ = totalEntitySpace / 2.0f;
		float addToXandZ = entitySpace * i;

		entities[i]->GetTransform()->SetTranslation(XMFLOAT3(-NUM_ENTITIES / 2.0f + i, 0.0f, -NUM_ENTITIES / 2.0f + i));
	}

	// Make a floor under all spheres
	materials.push_back(std::make_shared<Material>(pipelineState.Get()));
	materials[materials.size() - 1]->SetColorTint(XMFLOAT3(0.4f, 0.4f, 0.4f));
	entities.push_back(std::make_shared<GameEntity>(meshes[1], materials[materials.size() - 1]));
	entities[entities.size() - 1]->GetTransform()->SetScale(NUM_ENTITIES * 2.0f, 1.0f, NUM_ENTITIES * 2.0f);
	entities[entities.size() - 1]->GetTransform()->SetTranslation(XMFLOAT3(0.0f, -2.0f, 0.0f));

	// Create entity data buffer once all entities are created
	RayTracing::CreateEntityDataBuffer(entities);

	// Once we have all of the BLASes ready, we can make a TLAS
	RayTracing::CreateTopLevelAccelerationStructureForScene(entities);

	// Finalize any initialization and wait for the GPU before proceeding to the game loop
	Graphics::CloseAndExecuteCommandList();
	Graphics::WaitForGPU();
	Graphics::ResetAllocatorAndCommandList(Graphics::SwapChainIndex());
}


// --------------------------------------------------------
// Creates initial camera objects
// --------------------------------------------------------
void Game::CreateCameras()
{
	// Just one camera for now
	cameras.push_back(std::make_shared<Camera>(XMFLOAT3(0.0f, NUM_ENTITIES + 5.0f, -NUM_ENTITIES - 5.0f), Window::AspectRatio()));
	cameras[0]->SetPitchYawRoll(XMFLOAT3(XMConvertToRadians(45.0f), 0.0f, 0.0f));
}


// --------------------------------------------------------
// Creates initial lights
// --------------------------------------------------------
void Game::CreateLights()
{
	lights.push_back(Light::Directional(XMFLOAT3(0.0f, 0.0f, 1.0f), 1.0f, XMFLOAT3(1.0f, 1.0f, 1.0f))); // White directional light from camera's direction
	lights.push_back(Light::Point(XMFLOAT3(10.0f, 0.0f, -3.0f), 0.5f, XMFLOAT3(0.7f, 0.08f, 0.56f), 20.0f)); // Magenta point light to the right and towards camera
	lights.push_back(Light::Point(XMFLOAT3(-3.0f, -2.0f, 5.0f), 0.5f, XMFLOAT3(1.0f, 1.0f, 1.0f), 10.0f)); // White point light to the left and down, away from camera
	lights.push_back(Light::Point(XMFLOAT3(0.0f, 0.0f, 0.0f), 0.5f, XMFLOAT3(0.05f, 0.05f, 1.0f), 5.0f)); // Blue point light that starts in the center (will move up and down)
}


// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	// Resize ray tracing output texture
	RayTracing::ResizeOutputUAV(Window::Width(), Window::Height());

	// Update aspect ratio of all cameras
	for (unsigned int i = 0; i < cameras.size(); i++)
	{
		cameras[i]->UpdateProjectionMatrix(Window::AspectRatio());
	}
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();

	// Update camera's position, angle, etc.
	cameras[currentCameraIndex]->Update(deltaTime);

	for (unsigned int i = 0; i < entities.size() - 1; i++)
	{
		XMFLOAT3 currentTranslation = entities[i]->GetTransform()->GetTranslation();

		switch (i % 2)
		{
		case 0:
			entities[i]->GetTransform()->SetTranslation(currentTranslation.x, currentTranslation.y, (float)sin(totalTime + cos(i)) * entities.size());
			break;
		case 1:
			entities[i]->GetTransform()->SetTranslation((float)sin(totalTime + cos(i)) * entities.size(), currentTranslation.y, currentTranslation.z);
			break;
		}
	}

	// Move light 4 up and down
	lights[3].Position.y = (float)sin(totalTime / 2.0f) * 2.5f;
}


// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Grab the current back buffer for this frame
	Microsoft::WRL::ComPtr<ID3D12Resource> currentBackBuffer = Graphics::BackBuffers[Graphics::SwapChainIndex()];

	// Ray tracing - Recreate the TLAS and then trace it
	{
		RayTracing::CreateTopLevelAccelerationStructureForScene(entities);
		RayTracing::Raytrace(cameras[0], currentBackBuffer);
	}

	// Present
	{
		// Must occur BEFORE present
		Graphics::CloseAndExecuteCommandList();

		// Present the current back buffer and move to the next one
		bool vsync = Graphics::VsyncState();
		Graphics::SwapChain->Present(
			vsync ? 1 : 0,
			vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING);
		Graphics::AdvanceSwapChainIndex();

		// Wait for the GPU to be done and then reset the command list & allocator
		Graphics::ResetAllocatorAndCommandList(Graphics::SwapChainIndex());
	}
}



