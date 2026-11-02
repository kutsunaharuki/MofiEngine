/***
 * @file TitleScene.cpp
 * @brief タイトルシーン
 */
#include "stdafx.h"
#include "TitleScene.h"
#include "InGameScene.h"
#include "Source/Sound/SoundManager.h"
#include "Source/UI/SoundOptionMenu/SoundOptionMenu.h"
#include "Source/UI/TitleEventMenu/TitleEventMenu.h"


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
        /** タイトルイベントのレイアウトパス */
        constexpr const char* TITLE_EVENT_LAYOUT_PATH = "Assets/parameter/UI/TitleEventItem/TitleEventItem.json";
    }


    TitleScene::TitleScene()
		: m_titleSceneSpriteRender(nullptr)
		, m_soundOptionMenu(nullptr)
        , m_titleEventMenu(nullptr)
        , m_isRequestInGame(false)
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
        
        // タイトルイベントのUIを作る(最初は閉じている)
        m_titleEventLayout.Initialize<ui::TitleEventMenu>(TITLE_EVENT_LAYOUT_PATH);
        m_titleEventMenu = m_titleEventLayout.GetMenu<ui::TitleEventMenu>();

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

        // サウンドオプションを開いている間は、タイトルのメニューは動かさない
        if (m_soundOptionMenu && m_soundOptionMenu->IsOpen()) return;

        // タイトルイベントメニューの更新
        m_titleEventMenu->Update();

        // タイトルテキストタイプを宣言
        ui::TitleEventMenu::TitleTextType decidedType;
        if (m_titleEventMenu && m_titleEventMenu->ConsumeDecided(decidedType))
        {
            switch (decidedType)
            {
                case ui::TitleEventMenu::TitleTextType::Start: m_isRequestInGame = true;  break;
                case ui::TitleEventMenu::TitleTextType::Sound: m_soundOptionMenu->Open(); break;
                case ui::TitleEventMenu::TitleTextType::End:   PostQuitMessage(0);        break;
                default: break;
            }
        }
    }
    
    
    void TitleScene::Render(RenderContext& rc)
    {
        if (m_titleSceneSpriteRender)
        {
            // 描画
            m_titleSceneSpriteRender->Draw(rc);
        }

        // タイトルイベントを描画(テキスト群)
        m_titleEventLayout.Render(rc);
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

        // インゲームに移る
        if (m_isRequestInGame)
        {
            id = InGameScene::ID();
            waitTime = 1.0f;
            return true;
        }
		return false;
    }
}