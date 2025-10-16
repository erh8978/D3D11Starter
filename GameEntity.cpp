#include "GameEntity.h"
#include "Graphics.h"
#include <DirectXMath.h>
#include <memory>


GameEntity::GameEntity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material)
{
	myMesh = mesh;
	myMaterial = material;
	myTransform = std::make_shared<Transform>();
}

GameEntity::~GameEntity()
{

}

void GameEntity::Update()
{

}

void GameEntity::Draw()
{
	myMesh->Draw();
}

std::shared_ptr<Mesh> GameEntity::GetMesh()
{
	return myMesh;
}

std::shared_ptr<Transform> GameEntity::GetTransform()
{
	return myTransform;
}

std::shared_ptr<Material> GameEntity::GetMaterial()
{
	return myMaterial;
}

void GameEntity::SetMesh(std::shared_ptr<Mesh> mesh)
{
	myMesh = mesh;
}

void GameEntity::SetTransform(std::shared_ptr<Transform> transform)
{
	myTransform = transform;
}

void GameEntity::SetMaterial(std::shared_ptr<Material> material)
{
	myMaterial = material;
}
