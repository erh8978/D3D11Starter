#include "Sky.h"
#include "Graphics.h"
#include "WICTextureLoader.h"
#include "BufferStructs.h"

using namespace DirectX;

Sky::Sky(
	std::shared_ptr<Mesh> mesh,
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pso,
	const wchar_t* right,
	const wchar_t* left,
	const wchar_t* up,
	const wchar_t* down,
	const wchar_t* front,
	const wchar_t* back,
	DirectX::XMFLOAT3 sunDir)
{
	_mesh = mesh;
	_pipelineState = pso;
	_sunDir = sunDir;

	_skyTextureIndex = Graphics::CreateCubemap(right, left, up, down, front, back);
}

Sky::~Sky()
{
	// Nothing needs to be deleted
	// Desturctor included anyways for good practice
}

void Sky::Draw(std::shared_ptr<Camera> camera)
{
	// Set the PSO
	Graphics::CommandList->SetPipelineState(_pipelineState.Get());

	// Fill constant buffer with necessary data
	SkyboxVertexShaderExternalData vsData = {};
	vsData.projection = camera->GetProjectionMatrix();
	vsData.view = camera->GetViewMatrix();

	// Get the GPU address to it and add it to the root signature
	D3D12_GPU_DESCRIPTOR_HANDLE vsDataCbvHandle = Graphics::FillNextConstantBufferAndGetGPUDescriptorHandle(&vsData, sizeof(SkyboxVertexShaderExternalData));
	Graphics::CommandList->SetGraphicsRootDescriptorTable(0, vsDataCbvHandle);

	// Now do it for the skybox's pixel shader data
	SkyboxPixelShaderExternalData psData = {};
	psData.skyTextureIndex = _skyTextureIndex;
	psData.sunDir = _sunDir;

	// Get GPU address and add it to the root signature, this time in a different root param
	D3D12_GPU_DESCRIPTOR_HANDLE psDataCbvHandle = Graphics::FillNextConstantBufferAndGetGPUDescriptorHandle(&psData, sizeof(SkyboxPixelShaderExternalData));
	Graphics::CommandList->SetGraphicsRootDescriptorTable(1, psDataCbvHandle);

	// Get vertex and index buffers from the mesh
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer = _mesh->GetVertexBuffer();
	D3D12_VERTEX_BUFFER_VIEW vbView = _mesh->GetVertexBufferView();

	Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer = _mesh->GetIndexBuffer();
	D3D12_INDEX_BUFFER_VIEW ibView = _mesh->GetIndexBufferView();

	// And tell the input assembler to use them
	Graphics::CommandList->IASetVertexBuffers(0, 1, &vbView);
	Graphics::CommandList->IASetIndexBuffer(&ibView);

	// Finally, draw!
	Graphics::CommandList->DrawIndexedInstanced(_mesh->GetIndexCount(), 1, 0, 0, 0);
}