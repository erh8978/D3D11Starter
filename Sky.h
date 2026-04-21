#pragma once
#include "Mesh.h"
#include "Camera.h"

#include <wrl/client.h>
#include <d3d12.h>
#include <memory>

class Sky
{
public:
	Microsoft::WRL::ComPtr<ID3D12PipelineState> _pipelineState;
	unsigned int _skyTextureIndex;

	//Microsoft::WRL::ComPtr<ID3D11SamplerState> _samplerState;
	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _SRV;
	//Microsoft::WRL::ComPtr<ID3D11RasterizerState> _rasterizerState;
	//Microsoft::WRL::ComPtr<ID3D11DepthStencilState> _depthStencilState;
	//Microsoft::WRL::ComPtr<ID3D11VertexShader> _vertexShader;
	//Microsoft::WRL::ComPtr<ID3D11PixelShader> _pixelShader;
	std::shared_ptr<Mesh> _mesh;

	Sky(std::shared_ptr<Mesh> mesh,
		Microsoft::WRL::ComPtr<ID3D12PipelineState> pso,
		const wchar_t* right,
		const wchar_t* left,
		const wchar_t* up,
		const wchar_t* down,
		const wchar_t* front,
		const wchar_t* back);

	~Sky();

	void Draw(std::shared_ptr<Camera> camera);
};