#include "GameEntity.h"
#include <DirectXMath.h>
#include <memory>


GameEntity::GameEntity(std::shared_ptr<Mesh> mesh)
{
	myMesh = mesh;
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