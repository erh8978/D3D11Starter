#pragma once

#include <memory>

#include "Mesh.h"
#include "Transform.h"
#include "Material.h"

class GameEntity
{
public:
	GameEntity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material);
	~GameEntity();

	// Getters
	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Transform> GetTransform();
	std::shared_ptr<Material> GetMaterial();

	// Setters
	void SetMesh(std::shared_ptr<Mesh> mesh);
	void SetTransform(std::shared_ptr<Transform> transform);
	void SetMaterial(std::shared_ptr<Material> material);

private:
	// Using shared pointers because there might be many entities with the same mesh, material, etc.
	std::shared_ptr<Mesh> _mesh;
	std::shared_ptr<Transform> _transform;
	std::shared_ptr<Material> _material;
};

