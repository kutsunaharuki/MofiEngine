#include "stdafx.h"
#include "Game.h"
#include "Source/Scene/SceneManager.h"
#include "Source/Sound/SoundManager.h"
#include "Source/Util/ParameterManager.h"


Game::Game()
{}


Game::~Game()
{
	// シーンが SoundManager / ParameterManager を使うので、シーンを先に破棄する
	app::SceneManager::DestoryInstance();
	app::sound::SoundManager::DestroyInstance();
	app::util::ParameterManager::DestroyInstance();
}


bool Game::Start()
{
	// シーンの Start で使うので、SoundManager と ParameterManager を先に作る
	app::util::ParameterManager::CreateInstance();
	app::sound::SoundManager::CreateInstance();
	app::SceneManager::CreateInstance();
	return true;
}


void Game::Update()
{
	// 調整用パラメーターのホットリロード(JSON が保存されていたら読み直す)
	app::util::ParameterManager::GetInstance()->Update();

	app::SceneManager::GetInstance()->Update();
}


void Game::Render(RenderContext& rc)
{
	app::SceneManager::GetInstance()->Render(rc);
}