#include "GameEntity.h"

GameEntity::GameEntity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material)
	: _mesh(mesh), _material(material)
{
	_transform = std::make_shared<Transform>();
}

// Nothing to delete
GameEntity::~GameEntity()
{
}

// Getters
std::shared_ptr<Mesh> GameEntity::GetMesh() { return _mesh; }
std::shared_ptr<Transform> GameEntity::GetTransform() { return _transform; }
std::shared_ptr<Material> GameEntity::GetMaterial() { return _material; }

// Setters
void GameEntity::SetMesh(std::shared_ptr<Mesh> mesh) { _mesh = mesh; }
void GameEntity::SetTransform(std::shared_ptr<Transform> transform) { _transform = transform; }
void GameEntity::SetMaterial(std::shared_ptr<Material> material) { _material = material; }