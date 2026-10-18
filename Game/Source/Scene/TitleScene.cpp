/***
 * @file TitleScene.cpp
 * @brief タイトルシーン
 */
#include "stdafx.h"
#include "TitleScene.h"
#include "InGameScene.h"
#include "Source/Sound/SoundManager.h"
#include "Source/UI/SoundOptionMenu/SoundOptionMenu.h"


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
        /** サウンドオプションのレイアウトのパス */
        constexpr const char* SOUND_OPTION_LAYOUT_PATH = "Assets/parameter/UI/Sound/SoundOption.json";
    }


    TitleScene::TitleScene()
    {}


    TitleScene::~TitleScene()
    {
        // タイトルのBGMは、タイトルを出るときに止める
        sound::SoundManager::GetInstance()->Stop(sound::EnSoundType::BGM);
    }
    
    
    bool TitleScene::Start()
    {
        // タイトルのスプライトレンダーを作成
        m_titleSceneSpriteRender = std::make_unique<SpriteRender>();

        // タイトルシーンスプライトレンダーを初期化
        m_titleSceneSpriteRender->Init(
            nullptr,
            TITLE_SCENE_FILE_PATH,
            TITLE_SCENE_WIDTH, TITLE_SCENE_HEIGHT
        );

        // 更新
        m_titleSceneSpriteRender->Update();

        // サウンドオプションのUIを作る(最初は閉じている)
        m_soundOptionLayout.Initialize<ui::SoundOptionMenu>(SOUND_OPTION_LAYOUT_PATH);
        m_soundOptionMenu = m_soundOptionLayout.GetMenu<ui::SoundOptionMenu>();

        // タイトルのBGMを鳴らす
        sound::SoundManager::GetInstance()->Play(sound::EnSoundType::BGM, sound::bgm::TITLE);
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

        // サウンドオプションの更新(開いているときだけ、中で入力を受け付ける)
        m_soundOptionLayout.Update();

        // スタートボタンで、サウンドオプションを開く
        // ※ メニューの更新の後に開くので、開いたフレームの入力でメニューが動くことはない
        if (m_soundOptionMenu && !m_soundOptionMenu->IsOpen() && g_pad[0]->IsTrigger(enButtonStart))
        {
            m_soundOptionMenu->Open();
        }
    }
    
    
    void TitleScene::Render(RenderContext& rc)
    {
        if (m_titleSceneSpriteRender)
        {
            // 描画
            m_titleSceneSpriteRender->Draw(rc);
        }

        // 背景の上に重ねる(閉じているときは描画されない)
        m_soundOptionLayout.Render(rc);
    }


    bool TitleScene::RequestScene(uint32_t& id, float& waitTime)
    {
        // サウンドオプションを開いている間は、Aボタンをメニューが使うので遷移しない
        if (m_soundOptionMenu && m_soundOptionMenu->IsOpen())
        {
            return false;
        }

		if (g_pad[0]->IsTrigger(enButtonA))
		{
            sound::SoundManager::GetInstance()->Play(sound::EnSoundType::SE, sound::se::DECIDE);
			id = InGameScene::ID();
			waitTime = 1.0f;
			return true;
		}
        return false;
    }
}