#pragma once

#include <DirectXMath.h>

// --------------------------------------------------------
// A custom vertex definition
//
// You will eventually ADD TO this, and/or make more of these!
// --------------------------------------------------------
struct Vertex
{
	DirectX::XMFLOAT3 Position;	    // The local position of the vertex
	DirectX::XMFLOAT2 UV;			// The UV (texture) coordinates for this vertex
	DirectX::XMFLOAT3 Normal;		// The surface direction at this vertex
	DirectX::XMFLOAT3 Tangent;		// Tangent to the normal vector
};