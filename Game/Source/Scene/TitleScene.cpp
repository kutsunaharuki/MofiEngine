#include "stdafx.h"
#include "TitleScene.h"


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
    
    
    void TitleScene::Render(RenderContext & rc)
    {}
    
    
    bool TitleScene::RequestScene(uint32_t & id, float& waitTime)
    {
        return false;
    }
}