#include "GameEntity.h"

GameEntity::GameEntity(std::shared_ptr<Mesh> mesh)
{
	_mesh = mesh;
	_transform = std::make_shared<Transform>();
}

// Nothing to delete
GameEntity::~GameEntity()
{
}

std::shared_ptr<Mesh> GameEntity::GetMesh()
{
	return _mesh;
}

std::shared_ptr<Transform> GameEntity::GetTransform()
{
	return _transform;
}

void GameEntity::SetMesh(std::shared_ptr<Mesh> mesh)
{
	_mesh = mesh;
}

void GameEntity::SetTransform(std::shared_ptr<Transform> transform)
{
	_transform = transform;
}
