/***
 * @file TitleScene.cpp
 * @brief タイトルシーン
 */
#include "stdafx.h"
#include "TitleScene.h"
#include "InGameScene.h"


namespace app
{
    namespace
    {
        /** タイトルシーンのスプライトパス */
        constexpr const char* TITLE_SCENE_FILE_PATH = "Assets/spriteData/Scene/TitleScene.DDS";
        /** タイトルシーンの幅 */
        constexpr float TITLE_SCENE_WIDTH = 1920.0f;
        /** タイトルシーンの高さ */
        constexpr float TITLE_SCENE_HEIGHT = 1080.0f;
    }


    TitleScene::TitleScene()
    {}
    
    
    TitleScene::~TitleScene()
    {}
    
    
    bool TitleScene::Start()
    {
        // タイトルのスプライトレンダーを作成
        m_titleSceneSpriteRender = std::make_unique<SpriteRender>();

        // タイトルシーンスプライトレンダーを初期化
        m_titleSceneSpriteRender->Init(
            nullptr,
            TITLE_SCENE_FILE_PATH,
            TITLE_SCENE_HEIGHT, TITLE_SCENE_WIDTH
        );

        // 更新
        m_titleSceneSpriteRender->Update();
        return true;
    }
    
    
    void TitleScene::Update()
    {
        // トランスフォームを更新
        m_transform.UpdateTransform();

        // タイトルシーンのスプライトレンダーのtransformを設定
        m_titleSceneSpriteRender->SetTRS(
            m_transform.GetWorldPosition(),
            m_transform.GetWorldRotation(),
            m_transform.GetWorldScale()
        );

        // タイトルのスプライトレンダーを更新
        m_titleSceneSpriteRender->Update();
    }
    
    
    void TitleScene::Render(RenderContext& rc)
    {
        if (m_titleSceneSpriteRender)
        {
            // 描画
            m_titleSceneSpriteRender->Draw(rc);
        }
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