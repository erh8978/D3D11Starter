#pragma once

#include <DirectXMath.h>
#include <memory>
#include "Mesh.h"
#include "Transform.h"
#include "Material.h"

class GameEntity
{
public:

	GameEntity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material);
	~GameEntity();

	void Update();
	void Draw();

	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Transform> GetTransform();
	std::shared_ptr<Material> GetMaterial();

	void SetMesh(std::shared_ptr<Mesh> mesh);
	void SetTransform(std::shared_ptr<Transform> transform);
	void SetMaterial(std::shared_ptr<Material> material);

private:

	std::shared_ptr<Mesh> myMesh;
	std::shared_ptr<Transform> myTransform;
	std::shared_ptr<Material> myMaterial;
};