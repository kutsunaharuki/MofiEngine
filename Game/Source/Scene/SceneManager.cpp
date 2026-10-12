/**
 * @file SceneManager.cpp
 * @brief シーンを一元管理するクラス
 */
#include "stdafx.h"
#include "SceneManager.h"
#include "InGameScene.h"
#include "ResultScene.h"
#include "TitleScene.h"


namespace app
{
    SceneManager::SceneManager()
        : m_currentScene(nullptr)
    {
        AddSceneMap<app::InGameScene>();
        AddSceneMap<app::ResultScene>();
        AddSceneMap<app::TitleScene>();

        CreateScene(app::TitleScene::ID());
    }

    
    SceneManager::~SceneManager()
    {

    }


    void SceneManager::Update()
    {}


    void SceneManager::Render(RenderContext & rc)
    {}
    
    
    void SceneManager::CreateScene(const uint32_t id)
    {}
}