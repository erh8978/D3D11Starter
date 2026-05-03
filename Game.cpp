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
#include "Sky.h"

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

	// Lights
	std::vector<Light> lights;
	const unsigned int MAX_LIGHTS = 10;

	// Skybox
	std::vector<std::shared_ptr<Sky>> skyboxes;
	unsigned int currentSkyboxIndex = 0;
}

// --------------------------------------------------------
// The constructor is called after the window and graphics API
// are initialized but before the game loop begins
// --------------------------------------------------------
Game::Game()
{
	CreateRootSigAndPipelineState();
	CreateGeometry();
	CreateCameras();
	CreateLights();
	CreateSkybox();
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
// Loads the two basic shaders, then creates the root signature
// and pipeline state object for our very basic demo
// --------------------------------------------------------
void Game::CreateRootSigAndPipelineState()
{
	// Blobs to hold raw shader byte code used in several steps below
	Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderByteCode;
	Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderByteCode;

	// Shaders for post processing
	Microsoft::WRL::ComPtr<ID3DBlob> fullscreenVSByteCode;
	Microsoft::WRL::ComPtr<ID3DBlob> SunRaysPSByteCode;

	// Load shaders
	{
		// Read our compiled vertex shader code into a blob
		// - Essentially just "open the file and plop its contents here"
		D3DReadFileToBlob(FixPath(L"VertexShader.cso").c_str(), vertexShaderByteCode.GetAddressOf());
		D3DReadFileToBlob(FixPath(L"PixelShader.cso").c_str(), pixelShaderByteCode.GetAddressOf());
		D3DReadFileToBlob(FixPath(L"FullscreenVS.cso").c_str(), fullscreenVSByteCode.GetAddressOf());
		D3DReadFileToBlob(FixPath(L"SunRaysPS.cso").c_str(), SunRaysPSByteCode.GetAddressOf());
	}

	// Input layout
	const unsigned int inputElementCount = 4;
	D3D12_INPUT_ELEMENT_DESC inputElements[inputElementCount] = {};
	{
		// Create an input layout that describes the vertex format
		// used by the vertex shader we're using
		//  - This is used by the pipeline to know how to interpret the raw data
		//    sitting inside a vertex buffer

		// Set up the first element - a position, which is 3 float values
		inputElements[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;	// R32 G32 B32 = float3
		inputElements[0].SemanticName = "POSITION";				// Name must match semantic
		inputElements[0].SemanticIndex = 0;						// First POSITION semantic

		// Set up the second element - UV, which is 2 more float values
		inputElements[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElements[1].Format = DXGI_FORMAT_R32G32_FLOAT;		// R32 G32 = float2
		inputElements[1].SemanticName = "TEXCOORD";
		inputElements[1].SemanticIndex = 0;						// First TEXCOORD semantic

		// Third element - surface normal, 3 float values
		inputElements[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElements[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;	// R32 G32 B32 = float3
		inputElements[2].SemanticName = "NORMAL";
		inputElements[2].SemanticIndex = 0;						// First NORMAL semantic

		// Fourth element - tangent vector to normal, 3 float values
		inputElements[3].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElements[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;	// R32 G32 B32 = float3
		inputElements[3].SemanticName = "TANGENT";
		inputElements[3].SemanticIndex = 0;						// First TANGENT semantic
	}

	// Root Signature
	{
		// Define a table of CBV's (constant buffer views)
		D3D12_DESCRIPTOR_RANGE cbvTableVS = {};
		cbvTableVS.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		cbvTableVS.NumDescriptors = 1;
		cbvTableVS.BaseShaderRegister = 0;
		cbvTableVS.RegisterSpace = 0;
		cbvTableVS.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		D3D12_DESCRIPTOR_RANGE cbvTablePS = {};
		cbvTablePS.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		cbvTablePS.NumDescriptors = 1;
		cbvTablePS.BaseShaderRegister = 0;
		cbvTablePS.RegisterSpace = 0;
		cbvTablePS.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		// Define the root parameters
		D3D12_ROOT_PARAMETER rootParams[2] = {};

		// First root param - vertex shader CB
		rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		rootParams[0].DescriptorTable.NumDescriptorRanges = 1;
		rootParams[0].DescriptorTable.pDescriptorRanges = &cbvTableVS;

		// Second root param - pixel shader CB
		rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		rootParams[1].DescriptorTable.NumDescriptorRanges = 1;
		rootParams[1].DescriptorTable.pDescriptorRanges = &cbvTablePS;

		// Create a single static sampler (available to all pixel shaders)
		D3D12_STATIC_SAMPLER_DESC anisoWrap = {};
		anisoWrap.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		anisoWrap.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		anisoWrap.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		anisoWrap.Filter = D3D12_FILTER_ANISOTROPIC;
		anisoWrap.MaxAnisotropy = 16;
		anisoWrap.MaxLOD = D3D12_FLOAT32_MAX;
		anisoWrap.ShaderRegister = 0; // Will be in register(s0) in the shaders
		anisoWrap.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		D3D12_STATIC_SAMPLER_DESC samplers[] = { anisoWrap };

		// Describe and serialize the root signature
		D3D12_ROOT_SIGNATURE_DESC rootSig = {};
		rootSig.Flags = 
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED;
		rootSig.NumParameters = ARRAYSIZE(rootParams);
		rootSig.pParameters = rootParams;
		rootSig.NumStaticSamplers = ARRAYSIZE(samplers);
		rootSig.pStaticSamplers = samplers;

		ID3DBlob* serializedRootSig = 0;
		ID3DBlob* errors = 0;

		D3D12SerializeRootSignature(
			&rootSig,
			D3D_ROOT_SIGNATURE_VERSION_1,
			&serializedRootSig,
			&errors);

		// Check for errors during serialization
		if (errors != 0)
		{
			OutputDebugString((wchar_t*)errors->GetBufferPointer());
		}

		// Actually create the root sig
		Graphics::Device->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(rootSignature.GetAddressOf()));
	}

	// Pipeline state
	{
		// Describe the pipeline state
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

		// -- Input assembler related --
		psoDesc.InputLayout.NumElements = inputElementCount;
		psoDesc.InputLayout.pInputElementDescs = inputElements;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		// Overall primitive topology type (triangle, line, etc.) is set here
		// IASetPrimTop() is still used to set list/strip/adj options

		// Root sig
		psoDesc.pRootSignature = rootSignature.Get();

		// -- Shaders (VS/PS) --
		psoDesc.VS.pShaderBytecode = vertexShaderByteCode->GetBufferPointer();
		psoDesc.VS.BytecodeLength = vertexShaderByteCode->GetBufferSize();
		psoDesc.PS.pShaderBytecode = pixelShaderByteCode->GetBufferPointer();
		psoDesc.PS.BytecodeLength = pixelShaderByteCode->GetBufferSize();

		// -- Render targets --
		psoDesc.NumRenderTargets = NumRenderTargets;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // Color
		psoDesc.RTVFormats[1] = DXGI_FORMAT_R8G8B8A8_UNORM; // Sun visibility
		psoDesc.RTVFormats[2] = DXGI_FORMAT_R8G8B8A8_UNORM; // Normals
		psoDesc.RTVFormats[3] = DXGI_FORMAT_R32_FLOAT;		// Depth
		psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		psoDesc.SampleDesc.Count = 1;
		psoDesc.SampleDesc.Quality = 0;

		// -- States --
		psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
		psoDesc.RasterizerState.DepthClipEnable = true;

		psoDesc.DepthStencilState.DepthEnable = true;
		psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;

		psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
		psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
		psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		// -- Misc --
		psoDesc.SampleMask = 0xffffffff;

		// Create the pipeline state object
		Graphics::Device->CreateGraphicsPipelineState(
			&psoDesc,
			IID_PPV_ARGS(pipelineState.GetAddressOf()));
		
		// Second PSO for post processes

		// -- Shaders (VS/PS) --
		psoDesc.VS.pShaderBytecode = fullscreenVSByteCode->GetBufferPointer();
		psoDesc.VS.BytecodeLength = fullscreenVSByteCode->GetBufferSize();
		psoDesc.PS.pShaderBytecode = SunRaysPSByteCode->GetBufferPointer();
		psoDesc.PS.BytecodeLength = SunRaysPSByteCode->GetBufferSize();

		// -- Render targets --
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[1] = DXGI_FORMAT_UNKNOWN; // Means that we won't be using these 'extra' render targets
		psoDesc.RTVFormats[2] = DXGI_FORMAT_UNKNOWN;
		psoDesc.RTVFormats[3] = DXGI_FORMAT_UNKNOWN;

		// Create the pipeline state object
		Graphics::Device->CreateGraphicsPipelineState(
			&psoDesc,
			IID_PPV_ARGS(fullscreenPSO.GetAddressOf()));
	}

	// Set up the viewport and scissor rectangle
	{
		// Set up the viewport so we render into the correct
		// portion of the render target
		viewport = {};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = (float)Window::Width();
		viewport.Height = (float)Window::Height();
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		// Define a scissor rectangle that defines a portion of
		// the render target for clipping. This is different from
		// a viewport in that it is applied after the pixel shader.
		// We need at least one of these, but we're rendering to
		// the entire window, so it'll be the same size.
		scissorRect = {};
		scissorRect.left = 0;
		scissorRect.top = 0;
		scissorRect.right = Window::Width();
		scissorRect.bottom = Window::Height();
	}

	// Set up the G-Buffer
	// RTV heap
	D3D12_DESCRIPTOR_HEAP_DESC gBufferHeapDesc{};
	gBufferHeapDesc.NumDescriptors = NumRenderTargets;
	gBufferHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	Graphics::Device->CreateDescriptorHeap(&gBufferHeapDesc, IID_PPV_ARGS(GBufferHeap.GetAddressOf()));

	// Get a pointer to the start of the RTV heap so we know where to put RTV descriptors
	D3D12_CPU_DESCRIPTOR_HANDLE gBufferHeapStart = GBufferHeap->GetCPUDescriptorHandleForHeapStart();
	unsigned int descSize = Graphics::Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV); // And the size of an RTV descriptor

	// Create each render target for the G-Buffer
	GBuffer[0].Texture = Graphics::CreateRenderTargetTexture(Window::Width(), Window::Height()); // Colors
	GBuffer[1].Texture = Graphics::CreateRenderTargetTexture(Window::Width(), Window::Height()); // Sun visibility
	GBuffer[2].Texture = Graphics::CreateRenderTargetTexture(Window::Width(), Window::Height()); // Normals
	GBuffer[3].Texture = Graphics::CreateRenderTargetTexture(Window::Width(), Window::Height(), DXGI_FORMAT_R32_FLOAT, DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f)); // Depth

	// Make sure the handles point to the correct, contiguous spots in the RTV heap
	for (unsigned int i = 0; i < NumRenderTargets; i++)
	{
		GBuffer[i].RTV = gBufferHeapStart;
		GBuffer[i].RTV.ptr += descSize * i;
	}

	// Create the RTVs
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Create the RTVs sequentially
	Graphics::Device->CreateRenderTargetView(GBuffer[0].Texture.Get(), &rtvDesc, GBuffer[0].RTV);
	Graphics::Device->CreateRenderTargetView(GBuffer[1].Texture.Get(), &rtvDesc, GBuffer[1].RTV);
	Graphics::Device->CreateRenderTargetView(GBuffer[2].Texture.Get(), &rtvDesc, GBuffer[2].RTV);

	// Change the format for depth
	rtvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	Graphics::Device->CreateRenderTargetView(GBuffer[3].Texture.Get(), &rtvDesc, GBuffer[3].RTV);

	// Create an SRV for each texture
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Reserve 4 SRVs
	for (unsigned int i = 0; i < NumRenderTargets; i++)
	{
		Graphics::ReserveDescriptorHeapSlot(&GBuffer[i].SRV.CPUHandle, &GBuffer[i].SRV.GPUHandle);
	}

	// Make the SRVs
	Graphics::Device->CreateShaderResourceView(GBuffer[0].Texture.Get(), &srvDesc, GBuffer[0].SRV.CPUHandle);
	Graphics::Device->CreateShaderResourceView(GBuffer[1].Texture.Get(), &srvDesc, GBuffer[1].SRV.CPUHandle);
	Graphics::Device->CreateShaderResourceView(GBuffer[2].Texture.Get(), &srvDesc, GBuffer[2].SRV.CPUHandle);

	// Change format for depth
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	Graphics::Device->CreateShaderResourceView(GBuffer[3].Texture.Get(), &srvDesc, GBuffer[3].SRV.CPUHandle);

	// Update indices
	for (unsigned int i = 0; i < NumRenderTargets; i++)
		GBuffer[i].SRV.GPUDescriptorIndex = Graphics::GetDescriptorIndex(GBuffer[i].SRV.GPUHandle);
}


// --------------------------------------------------------
// Creates the geometry we're going to draw
// --------------------------------------------------------
void Game::CreateGeometry()
{
	// Load meshes from .obj files
	meshes.push_back(std::make_shared<Mesh>("Sphere", FixPath(L"../../Assets/Meshes/sphere.obj").c_str()));
	meshes.push_back(std::make_shared<Mesh>("Helix", FixPath(L"../../Assets/Meshes/helix.obj").c_str()));
	meshes.push_back(std::make_shared<Mesh>("Cube", FixPath(L"../../Assets/Meshes/cube.obj").c_str()));

	// Load textures and create materials
	materials.push_back(std::make_shared<Material>(pipelineState.Get()));
	materials[0]->LoadTextureSet(L"bronze");

	materials.push_back(std::make_shared<Material>(pipelineState.Get()));
	materials[1]->LoadTextureSet(L"cobblestone");

	materials.push_back(std::make_shared<Material>(pipelineState.Get()));
	materials[2]->LoadTextureSet(L"scratched");

	// Create gameEntities using those meshes and materials
	entities.push_back(std::make_shared<GameEntity>(meshes[0], materials[0]));
	entities.push_back(std::make_shared<GameEntity>(meshes[1], materials[1]));
	entities.push_back(std::make_shared<GameEntity>(meshes[2], materials[2]));

	// Adjust transforms
	entities[0]->GetTransform()->SetTranslation(-3.0f, 0.0f, 0.0f);
	entities[2]->GetTransform()->SetTranslation(3.0f, 0.0f, 0.0f);
}


// --------------------------------------------------------
// Creates initial camera objects
// --------------------------------------------------------
void Game::CreateCameras()
{
	// Just one camera for now
	cameras.push_back(std::make_shared<Camera>(XMFLOAT3(0.0f, 0.0f, -10.0f), Window::AspectRatio()));
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
// Creates skybox & related PSO/root sig
// --------------------------------------------------------
void Game::CreateSkybox()
{
	// All skyboxes will use the same PSO settings & root signature parameters,
	// So instead of having a method in Sky to do this work, we do it in Game so we can pass the same PSO to any skybox we make

	// Blobs to hold raw shader byte code used in several steps below
	Microsoft::WRL::ComPtr<ID3DBlob> skyboxVertexShaderByteCode;
	Microsoft::WRL::ComPtr<ID3DBlob> skyboxPixelShaderByteCode;

	// Load shaders
	{
		// Read our compiled vertex shader code into a blob
		// - Essentially just "open the file and plop its contents here"
		D3DReadFileToBlob(FixPath(L"SkyboxVS.cso").c_str(), skyboxVertexShaderByteCode.GetAddressOf());
		D3DReadFileToBlob(FixPath(L"SkyboxPS.cso").c_str(), skyboxPixelShaderByteCode.GetAddressOf());
	}

	// Input layout
	const unsigned int inputElementCount = 4;
	D3D12_INPUT_ELEMENT_DESC inputElements[inputElementCount] = {};
	{
		// Create an input layout that describes the vertex format
		// used by the vertex shader we're using
		//  - This is used by the pipeline to know how to interpret the raw data
		//    sitting inside a vertex buffer

		// Set up the first element - a position, which is 3 float values
		inputElements[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;	// R32 G32 B32 = float3
		inputElements[0].SemanticName = "POSITION";				// Name must match semantic
		inputElements[0].SemanticIndex = 0;						// First POSITION semantic

		// Set up the second element - UV, which is 2 more float values
		inputElements[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElements[1].Format = DXGI_FORMAT_R32G32_FLOAT;		// R32 G32 = float2
		inputElements[1].SemanticName = "TEXCOORD";
		inputElements[1].SemanticIndex = 0;						// First TEXCOORD semantic

		// Third element - surface normal, 3 float values
		inputElements[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElements[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;	// R32 G32 B32 = float3
		inputElements[2].SemanticName = "NORMAL";
		inputElements[2].SemanticIndex = 0;						// First NORMAL semantic

		// Fourth element - tangent vector to normal, 3 float values
		inputElements[3].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElements[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;	// R32 G32 B32 = float3
		inputElements[3].SemanticName = "TANGENT";
		inputElements[3].SemanticIndex = 0;						// First TANGENT semantic
	}

	// Pointers that will be used when creating the Sky object
	Microsoft::WRL::ComPtr<ID3D12RootSignature> skyboxRootSig;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> skyboxPSO;

	// Root Signature
	{
		// Define a table of CBV's (constant buffer views)
		D3D12_DESCRIPTOR_RANGE cbvTableVS = {};
		cbvTableVS.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		cbvTableVS.NumDescriptors = 1;
		cbvTableVS.BaseShaderRegister = 0;
		cbvTableVS.RegisterSpace = 0;
		cbvTableVS.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		D3D12_DESCRIPTOR_RANGE cbvTablePS = {};
		cbvTablePS.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		cbvTablePS.NumDescriptors = 1;
		cbvTablePS.BaseShaderRegister = 0;
		cbvTablePS.RegisterSpace = 0;
		cbvTablePS.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		// Define the root parameters
		D3D12_ROOT_PARAMETER rootParams[2] = {};

		// First root param - vertex shader CB
		rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		rootParams[0].DescriptorTable.NumDescriptorRanges = 1;
		rootParams[0].DescriptorTable.pDescriptorRanges = &cbvTableVS;

		// Second root param - pixel shader CB
		rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		rootParams[1].DescriptorTable.NumDescriptorRanges = 1;
		rootParams[1].DescriptorTable.pDescriptorRanges = &cbvTablePS;

		// Create a single static sampler (available to all pixel shaders)
		D3D12_STATIC_SAMPLER_DESC anisoWrap = {};
		anisoWrap.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		anisoWrap.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		anisoWrap.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		anisoWrap.Filter = D3D12_FILTER_ANISOTROPIC;
		anisoWrap.MaxAnisotropy = 16;
		anisoWrap.MaxLOD = D3D12_FLOAT32_MAX;
		anisoWrap.ShaderRegister = 0; // Will be in register(s0) in the shaders
		anisoWrap.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		D3D12_STATIC_SAMPLER_DESC samplers[] = { anisoWrap };

		// Describe and serialize the root signature
		D3D12_ROOT_SIGNATURE_DESC rootSig = {};
		rootSig.Flags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED;
		rootSig.NumParameters = ARRAYSIZE(rootParams);
		rootSig.pParameters = rootParams;
		rootSig.NumStaticSamplers = ARRAYSIZE(samplers);
		rootSig.pStaticSamplers = samplers;

		ID3DBlob* serializedRootSig = 0;
		ID3DBlob* errors = 0;

		D3D12SerializeRootSignature(
			&rootSig,
			D3D_ROOT_SIGNATURE_VERSION_1,
			&serializedRootSig,
			&errors);

		// Check for errors during serialization
		if (errors != 0)
		{
			OutputDebugString((wchar_t*)errors->GetBufferPointer());
		}

		// Actually create the root sig
		Graphics::Device->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(skyboxRootSig.GetAddressOf()));
	}

	// Pipeline state
	{
		// Describe the pipeline state
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

		// -- Input assembler related --
		psoDesc.InputLayout.NumElements = inputElementCount;
		psoDesc.InputLayout.pInputElementDescs = inputElements;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		// Overall primitive topology type (triangle, line, etc.) is set here
		// IASetPrimTop() is still used to set list/strip/adj options

		// Root sig
		psoDesc.pRootSignature = skyboxRootSig.Get();

		// -- Shaders (VS/PS) --
		psoDesc.VS.pShaderBytecode = skyboxVertexShaderByteCode->GetBufferPointer();
		psoDesc.VS.BytecodeLength = skyboxVertexShaderByteCode->GetBufferSize();
		psoDesc.PS.pShaderBytecode = skyboxPixelShaderByteCode->GetBufferPointer();
		psoDesc.PS.BytecodeLength = skyboxPixelShaderByteCode->GetBufferSize();

		// -- Render targets --
		psoDesc.NumRenderTargets = 2;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.RTVFormats[1] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.RTVFormats[2] = DXGI_FORMAT_UNKNOWN;
		psoDesc.RTVFormats[3] = DXGI_FORMAT_UNKNOWN;
		psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		psoDesc.SampleDesc.Count = 1;
		psoDesc.SampleDesc.Quality = 0;

		// -- States --
		psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT; // Different from main PSO: cull front faces, render back faces
		psoDesc.RasterizerState.DepthClipEnable = true;

		psoDesc.DepthStencilState.DepthEnable = true;
		psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; // Different from main PSO: draw even if depth is equal to current depth buffer; the skybox is ALWAYS at max depth
		psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;

		psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
		psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
		psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		// -- Misc --
		psoDesc.SampleMask = 0xffffffff;

		// Create the pipeline state object
		Graphics::Device->CreateGraphicsPipelineState(
			&psoDesc,
			IID_PPV_ARGS(skyboxPSO.GetAddressOf()));
	}

	std::shared_ptr<Mesh> skyboxMesh = std::make_shared<Mesh>("Cube", FixPath(L"../../Assets/Meshes/cube.obj").c_str());

	// Create skybox objects
	// Clouds Blue
	skyboxes.push_back(std::make_shared<Sky>(
		skyboxMesh,
		skyboxPSO,
		FixPath(L"../../Assets/Textures/Clouds Blue/right.png").c_str(),
		FixPath(L"../../Assets/Textures/Clouds Blue/left.png").c_str(),
		FixPath(L"../../Assets/Textures/Clouds Blue/up.png").c_str(),
		FixPath(L"../../Assets/Textures/Clouds Blue/down.png").c_str(),
		FixPath(L"../../Assets/Textures/Clouds Blue/front.png").c_str(),
		FixPath(L"../../Assets/Textures/Clouds Blue/back.png").c_str()));

	// Clouds Pink
	skyboxes.push_back(std::make_shared<Sky>(
		skyboxMesh,
		skyboxPSO,
		FixPath(L"../../Assets/Textures/Clouds Pink/right.png").c_str(),
		FixPath(L"../../Assets/Textures/Clouds Pink/left.png").c_str(),
		FixPath(L"../../Assets/Textures/Clouds Pink/up.png").c_str(),
		FixPath(L"../../Assets/Textures/Clouds Pink/down.png").c_str(),
		FixPath(L"../../Assets/Textures/Clouds Pink/front.png").c_str(),
		FixPath(L"../../Assets/Textures/Clouds Pink/back.png").c_str()));

	// Cold Sunset
	skyboxes.push_back(std::make_shared<Sky>(
		skyboxMesh,
		skyboxPSO,
		FixPath(L"../../Assets/Textures/Cold Sunset/right.png").c_str(),
		FixPath(L"../../Assets/Textures/Cold Sunset/left.png").c_str(),
		FixPath(L"../../Assets/Textures/Cold Sunset/up.png").c_str(),
		FixPath(L"../../Assets/Textures/Cold Sunset/down.png").c_str(),
		FixPath(L"../../Assets/Textures/Cold Sunset/front.png").c_str(),
		FixPath(L"../../Assets/Textures/Cold Sunset/back.png").c_str()));

	// Planet
	skyboxes.push_back(std::make_shared<Sky>(
		skyboxMesh,
		skyboxPSO,
		FixPath(L"../../Assets/Textures/Planet/right.png").c_str(),
		FixPath(L"../../Assets/Textures/Planet/left.png").c_str(),
		FixPath(L"../../Assets/Textures/Planet/up.png").c_str(),
		FixPath(L"../../Assets/Textures/Planet/down.png").c_str(),
		FixPath(L"../../Assets/Textures/Planet/front.png").c_str(),
		FixPath(L"../../Assets/Textures/Planet/back.png").c_str()));

	
}


// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	// Resize the viewport and scissor rectangle
	{
		// Set up the viewport so we render into the correct
		// portion of the render target
		viewport = {};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = (float)Window::Width();
		viewport.Height = (float)Window::Height();
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		// Define a scissor rectangle that defines a portion of
		// the render target for clipping. This is different from
		// a viewport in that it is applied after the pixel shader.
		// We need at least one of these, but we're rendering to
		// the entire window, so it'll be the same size.
		scissorRect = {};
		scissorRect.left = 0;
		scissorRect.top = 0;
		scissorRect.right = Window::Width();
		scissorRect.bottom = Window::Height();
	}

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

	// Increment skybox index when V is pressed
	if (Input::KeyPress('V'))
	{
		currentSkyboxIndex = (currentSkyboxIndex + 1) % skyboxes.size();
	}

	// Update camera's position, angle, etc.
	cameras[currentCameraIndex]->Update(deltaTime);

	for (unsigned int i = 0; i < entities.size(); i++)
	{
		entities[i]->GetTransform()->Rotate(0.0f, 1.0f * deltaTime, 0.0f);
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

	// Clearing the render target
	{
		// Transition the back buffer from present to render target
		D3D12_RESOURCE_BARRIER rb = {};
		rb.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		rb.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		rb.Transition.pResource = currentBackBuffer.Get();
		rb.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		rb.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		rb.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		Graphics::CommandList->ResourceBarrier(1, &rb);

		// Background color for clearing
		float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };

		// Clear the RTV
		Graphics::CommandList->ClearRenderTargetView(
			Graphics::RTVHandles[Graphics::SwapChainIndex()],
			color,
			0, 0); // No scissor rects

		// Clear the depth buffer, too
		Graphics::CommandList->ClearDepthStencilView(
			Graphics::DSVHandle,
			D3D12_CLEAR_FLAG_DEPTH,
			1.0f,	// Max depth = 1.0f
			0,		// Not clearing stencil, but need a value
			0, 0); // No scissor rects

		// Transition and clear GBuffer render targets
		float depthClearColor[4] = { 1.0f, 0.0f, 0.0f, 0.0f };
		for (unsigned int i = 0; i < NumRenderTargets; i++)
		{
			rb.Transition.pResource = GBuffer[i].Texture.Get();
			rb.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
			rb.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
			Graphics::CommandList->ResourceBarrier(1, &rb);

			if (i == 3)
			{
				Graphics::CommandList->ClearRenderTargetView(GBuffer[i].RTV, depthClearColor, 0, 0);
			}
			else
			{
				Graphics::CommandList->ClearRenderTargetView(GBuffer[i].RTV, color, 0, 0);
			}
		}
	}

	// Rendering here!
	{
		// Set overall pipeline state
		Graphics::CommandList->SetPipelineState(pipelineState.Get());

		// CBV/SRV/UAV heap must be bound using SetDescriptorHeaps() before SetGraphicsRootSignature() is called
		Graphics::CommandList->SetDescriptorHeaps(
			1,
			Graphics::CBVSRVDescriptorHeap.GetAddressOf());
		// Root sig (must happen before root descriptor table)
		Graphics::CommandList->SetGraphicsRootSignature(rootSignature.Get());

		// Set up other commands for rendering
		Graphics::CommandList->OMSetRenderTargets(
			NumRenderTargets, // Using multiple render targets
			&GBuffer[0].RTV,
			true,
			&Graphics::DSVHandle);
		Graphics::CommandList->RSSetViewports(1, &viewport);
		Graphics::CommandList->RSSetScissorRects(1, &scissorRect);
		Graphics::CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		

		// Draw

		// Start with game entities
		for (unsigned int i = 0; i < entities.size(); i++)
		{
			// Get data from this entity
			std::shared_ptr<Transform> transform = entities[i]->GetTransform();
			std::shared_ptr<Mesh> mesh = entities[i]->GetMesh();
			std::shared_ptr<Material> material = entities[i]->GetMaterial();
			std::shared_ptr<Camera> currentCamera = cameras[currentCameraIndex];

			// Set the PSO to the one stored in Material
			Graphics::CommandList->SetPipelineState(material->GetPipelineStateObject().Get());

			// Fill out vertex shader constant buffer
			VertexShaderExternalData vsData{};
			vsData.world = transform->GetWorldMatrix();
			vsData.view = currentCamera->GetViewMatrix();
			vsData.projection = currentCamera->GetProjectionMatrix();
			vsData.worldInvTranspose = transform->GetWorldInvTranspose();

			// Put the GPU handle for the VS buffer in the root signature
			D3D12_GPU_DESCRIPTOR_HANDLE vsDataCbvHandle = Graphics::FillNextConstantBufferAndGetGPUDescriptorHandle(&vsData, sizeof(VertexShaderExternalData));
			Graphics::CommandList->SetGraphicsRootDescriptorTable(0, vsDataCbvHandle);

			// Do the same for the PS constant buffer
			PixelShaderExternalData psData{};
			psData.albedoMapIndex = material->GetAlbedoMapIndex();
			psData.normalMapIndex = material->GetNormalMapIndex();
			psData.metalnessIndex = material->GetMetalnessIndex();
			psData.roughnessIndex = material->GetRoughnessIndex();
			psData.uvScale = material->GetUVScale();
			psData.uvOffset = material->GetUVOffset();
			psData.colorTint = material->GetColorTint();
			psData.cameraPos = currentCamera->GetTranslation();

			// memcpy() lights into the buffer data object
			memcpy(&psData.lights, &lights[0], sizeof(Light) * (int)lights.size());

			D3D12_GPU_DESCRIPTOR_HANDLE psDataCbvHandle = Graphics::FillNextConstantBufferAndGetGPUDescriptorHandle(&psData, sizeof(PixelShaderExternalData));
			Graphics::CommandList->SetGraphicsRootDescriptorTable(1, psDataCbvHandle); // Goes in descriptor table index 1, not 0!

			// Get vertex and index buffers from the mesh
			Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer = mesh->GetVertexBuffer();
			D3D12_VERTEX_BUFFER_VIEW vbView = mesh->GetVertexBufferView();

			Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer = mesh->GetIndexBuffer();
			D3D12_INDEX_BUFFER_VIEW ibView = mesh->GetIndexBufferView();

			// And tell the input assembler to use them
			Graphics::CommandList->IASetVertexBuffers(0, 1, &vbView);
			Graphics::CommandList->IASetIndexBuffer(&ibView);

			// Finally, draw!
			Graphics::CommandList->DrawIndexedInstanced(mesh->GetIndexCount(), 1, 0, 0, 0);
		}

		// Once they're done, draw the skybox (to avoid overdraw)
		skyboxes[currentSkyboxIndex]->Draw(cameras[currentCameraIndex]);
	}

	// Go back to the back buffer
	Graphics::CommandList->OMSetRenderTargets(1, &Graphics::RTVHandles[Graphics::SwapChainIndex()], true, &Graphics::DSVHandle);

	// Transition GBuffer textures back so they can be used in the post processing pixel shader
	for (unsigned int i = 0; i < NumRenderTargets; i++)
	{
		D3D12_RESOURCE_BARRIER rb = {};
		rb.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		rb.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		rb.Transition.pResource = GBuffer[i].Texture.Get();
		rb.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		rb.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		rb.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		Graphics::CommandList->ResourceBarrier(1, &rb);
	}

	// Run the SSAO post-process
	{
		Graphics::CommandList->SetPipelineState(fullscreenPSO.Get());
		Graphics::CommandList->SetGraphicsRootSignature(rootSignature.Get());

		// Calculate screenspace sun position
		XMFLOAT4X4 v = cameras[currentCameraIndex]->GetViewMatrix();
		XMFLOAT4X4 p = cameras[currentCameraIndex]->GetProjectionMatrix();

		XMVECTOR sunDirection = XMLoadFloat3(&skyboxes[currentSkyboxIndex]->_sunDir);
		XMMATRIX view = XMLoadFloat4x4(&v);
		XMMATRIX projection = XMLoadFloat4x4(&p);

		XMVECTOR ssp = XMVector3Transform(sunDirection, XMMatrixMultiply(view, projection));

		XMFLOAT4 screenSunPos;
		XMStoreFloat4(&screenSunPos, ssp);
		screenSunPos.x = (screenSunPos.x / screenSunPos.w + 1) / 2;
		screenSunPos.y = (-screenSunPos.y / screenSunPos.w + 1) / 2;
		screenSunPos.z = (screenSunPos.z / screenSunPos.w + 1) / 2;


		SunRaysPixelShaderExternalData sunRaysData = {};
		sunRaysData.colorIndex = GBuffer[0].SRV.GPUDescriptorIndex;
		sunRaysData.sunVisibilityIndex = GBuffer[1].SRV.GPUDescriptorIndex;
		sunRaysData.normalsIndex = GBuffer[2].SRV.GPUDescriptorIndex;
		sunRaysData.depthIndex = GBuffer[3].SRV.GPUDescriptorIndex;
		sunRaysData.screenSunPos = screenSunPos;

		D3D12_GPU_DESCRIPTOR_HANDLE ssaoDataCbvHandle = Graphics::FillNextConstantBufferAndGetGPUDescriptorHandle(&sunRaysData, sizeof(PixelShaderExternalData));
		Graphics::CommandList->SetGraphicsRootDescriptorTable(1, ssaoDataCbvHandle); // Goes in descriptor table index 1, not 0!
		Graphics::CommandList->DrawInstanced(3, 1, 0, 0);
	}

	// Present
	{
		// Transition back to present
		D3D12_RESOURCE_BARRIER rb = {};
		rb.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		rb.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		rb.Transition.pResource = currentBackBuffer.Get();
		rb.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		rb.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		rb.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		Graphics::CommandList->ResourceBarrier(1, &rb);

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



