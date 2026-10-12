/**
 * @file InGameScene.cpp
 * @brief インゲームシーン
 */
#include "stdafx.h"
#include "InGameScene.h"


namespace app
{
	InGameScene::InGameScene()
	{}
	
	
	InGameScene::~InGameScene()
	{}
	
	
	bool InGameScene::Start()
	{
		return true;
	}
	
	
	void InGameScene::Update()
	{}
	
	
	void InGameScene::Render(RenderContext & rc)
	{}
	
	
	bool InGameScene::RequestScene(uint32_t & id, float& waitTime)
	{
		return false;
	}
}