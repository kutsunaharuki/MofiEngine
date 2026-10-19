#include "stdafx.h"
#include "Game.h"
#include "Source/Scene/SceneManager.h"
#include "Source/Sound/SoundManager.h"


Game::Game()
{}


Game::~Game()
{
	// シーンが SoundManager を使うので、シーンを先に破棄する
	app::SceneManager::DestoryInstance();
	app::sound::SoundManager::DestroyInstance();
}


bool Game::Start()
{
	// シーンの Start で音を鳴らすので、SoundManager を先に作る
	app::sound::SoundManager::CreateInstance();
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