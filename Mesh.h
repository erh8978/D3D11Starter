#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include<string>

#include "Vertex.h"

// ---------------------------------------------------------------------
// A group of several triangles and the functions needed to render them.
// ---------------------------------------------------------------------
class Mesh
{
public:
	Mesh(Vertex* vertices, unsigned int* indices, unsigned int vertexCount, unsigned int indexCount, std::string name = "Unnamed Mesh");
	~Mesh();

	void Draw();

	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();

	int GetIndexCount();
	int GetVertexCount();

	// Name for ImGUI display
	std::string meshName;

private:
	// ComPointers to vertex and index buffer objects 
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	// How many vertices in the vertex buffer?
	unsigned int vertexBufferCount;

	// How many indices in the index buffer?
	unsigned int indexBufferCount;
};