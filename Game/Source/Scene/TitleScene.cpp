/***
 * @file TitleScene.cpp
 * @brief タイトルシーン
 */
#include "stdafx.h"
#include "TitleScene.h"
#include "InGameScene.h"


namespace app
{
    TitleScene::TitleScene()
    {}
    
    
    TitleScene::~TitleScene()
    {}
    
    
    bool TitleScene::Start()
    {
        return true;
    }
    
    
    void TitleScene::Update()
    {}
    
    
    void TitleScene::Render(RenderContext& rc)
    {

    }
    
    
    bool TitleScene::RequestScene(uint32_t& id, float& waitTime)
    {
		if (g_pad[0]->IsTrigger(enButtonA))
		{
			id = InGameScene::ID();
			waitTime = 1.0f;
			return true;
		}
        return false;
    }
}