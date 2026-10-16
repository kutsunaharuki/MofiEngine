#include "stdafx.h"
#include "Game.h"
#include "Source/Scene/SceneManager.h"


Game::Game()
{}


Game::~Game()
{
	app::SceneManager::GetInstance()->DestoryInstance();
}


bool Game::Start()
{
	app::SceneManager::CreateInstance();
	return true;
}


void Game::Update()
{
	app::SceneManager::GetInstance()->Update();
}


void Game::Render(RenderContext& rc)
{
	app::SceneManager::GetInstance()->Render(rc);
}