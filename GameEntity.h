#pragma once

#include <DirectXMath.h>
#include <memory>
#include "Mesh.h"
#include "Transform.h"

class GameEntity
{
public:

	GameEntity(std::shared_ptr<Mesh> mesh);
	~GameEntity();

	void Update();
	void Draw();

	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Transform> GetTransform();

private:

	std::shared_ptr<Mesh> myMesh;
	std::shared_ptr<Transform> myTransform;
};