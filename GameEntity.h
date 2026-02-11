#pragma once

#include <memory>

#include "Mesh.h"
#include "Transform.h"

class GameEntity
{
public:
	GameEntity(std::shared_ptr<Mesh> mesh);
	~GameEntity();

	// Getters and setters
	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Transform> GetTransform();

	void SetMesh(std::shared_ptr<Mesh> mesh);
	void SetTransform(std::shared_ptr<Transform> transform);

private:
	std::shared_ptr<Mesh> _mesh;
	std::shared_ptr<Transform> _transform;
};

