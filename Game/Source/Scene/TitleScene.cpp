/***
 * @file TitleScene.cpp
 * @brief タイトルシーン
 */
#include "stdafx.h"
#include "TitleScene.h"
#include "InGameScene.h"
#include "Source/Sound/SoundManager.h"
#include "Source/UI/TitleEventMenu/TitleEventMenu.h"


namespace app
{
    namespace
    {
        /** タイトルシーンのスプライトパス */
        constexpr const char* TITLE_SCENE_FILE_PATH = "Assets/spriteData/UI/Title/TitleBG.dds";
        /** タイトルシーンの幅 */
        constexpr float TITLE_SCENE_WIDTH = 1920.0f;
        /** タイトルシーンの高さ */
        constexpr float TITLE_SCENE_HEIGHT = 1080.0f;
        /** サウンドオプションのレイアウトのパス */
        constexpr const char* SOUND_OPTION_LAYOUT_PATH = "Assets/parameter/UI/Sound/SoundOption.json";
        /** タイトルイベントのレイアウトパス */
        constexpr const char* TITLE_EVENT_LAYOUT_PATH = "Assets/parameter/UI/TitleEventItem/TitleEventItem.json";
        /** 「Press A Button」のレイアウトのパス */
        constexpr const char* PRESS_BUTTON_LAYOUT_PATH = "Assets/parameter/UI/TitlePressButton/TitlePressButton.json";
    }


    TitleScene::TitleScene()
		: m_titleSceneSpriteRender(nullptr)
        , m_titleEventMenu(nullptr)
        , m_titleSelectIndex(static_cast<int>(ui::TitleEventMenu::TitleTextType::Start))
        , m_isRequestInGame(false)
        , m_isStartFlash(false)
        , m_titleState(TitleState::PressButton)
        , m_pressButtonMenu(nullptr)
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
        m_soundOption.Initialize(SOUND_OPTION_LAYOUT_PATH);
        
        // タイトルイベントのUIを作る(最初は閉じている)
        m_titleEventLayout.Initialize<ui::TitleEventMenu>(TITLE_EVENT_LAYOUT_PATH);
        m_titleEventMenu = m_titleEventLayout.GetMenu<ui::TitleEventMenu>();
        if (m_titleEventMenu)
        {
            m_titleEventMenu->SetSelect(static_cast<ui::TitleEventMenu::TitleTextType>(m_titleSelectIndex));
            // 最初は「Press A Button」だけを出して、Start / Sound / End は隠す
            m_titleEventMenu->SetIsItemOpen(false);
        }

        // 「Press A Button」の UI を作る(最初は出している)
        m_pressButtonLayout.Initialize<ui::TitlePressButtonMenu>(PRESS_BUTTON_LAYOUT_PATH);
        m_pressButtonMenu = m_pressButtonLayout.GetMenu<ui::TitlePressButtonMenu>();
        if (m_pressButtonMenu) m_pressButtonMenu->SetIsOpen(true);

        // タイトルのBGMを鳴らす
        sound::SoundManager::GetInstance()->Play(sound::EnSoundType::BGM, sound::bgm::TITLE);
        return true;
    }
    
    
    void TitleScene::Update()
    {
        // タイトルのフラッシュの調整
        m_titleFlashDebugWindow.Draw(m_titleEventMenu);

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

        // Start を決めた後は、フラッシュが終わるのを待つだけ(入力は受けない)
        if (m_isStartFlash)
        {
            if (m_titleEventMenu) m_titleEventMenu->Update();
            if (m_titleEventMenu == nullptr || m_titleEventMenu->IsScreenFlashFinished())
            {
                m_isRequestInGame = true;
            }
            return;
        }

        // 「Press A Button」の画面(ロゴのフラッシュは、この画面でも動かす)
        if (m_titleState == TitleState::PressButton)
        {
            UpdatePressButton();
            if (m_titleEventMenu) m_titleEventMenu->Update();
            return;
        }

        // サウンドオプションの更新(開いているときだけ、中で入力を受け付ける)
        m_soundOption.Update();

        // サウンドオプションを開いている間は、タイトルのメニューは動かさない
        if (m_soundOption.IsOpen()) return;

        // タイトルイベントの入力(上下で選ぶ・A で決める)
        UpdateTitleSelect();

        // タイトルイベントメニューの更新
        if (m_titleEventMenu) m_titleEventMenu->Update();
    }


    void TitleScene::UpdatePressButton()
    {
        m_pressButtonLayout.Update();

        if (!g_pad[0]->IsTrigger(enButtonA)) return;

        // Start / Sound / End の画面に移る
        // (このフレームは呼んだ所で return するので、同じ A で Start が決まることはない)
        sound::SoundManager::GetInstance()->Play(sound::EnSoundType::SE, sound::se::DECIDE);
        m_titleState = TitleState::Select;
        if (m_pressButtonMenu) m_pressButtonMenu->SetIsOpen(false);
        if (m_titleEventMenu) m_titleEventMenu->SetIsItemOpen(true);
    }


    void TitleScene::UpdateTitleSelect()
    {
        using TitleTextType = ui::TitleEventMenu::TitleTextType;
        constexpr int itemCount = static_cast<int>(TitleTextType::Max);

        // 上と下の入力で選ぶ項目を変える(端まで行ったら反対側に戻る)
        const int prevIndex = m_titleSelectIndex;
        if (g_pad[0]->IsTrigger(enButtonDown)) m_titleSelectIndex = (m_titleSelectIndex + 1) % itemCount;
        if (g_pad[0]->IsTrigger(enButtonUp))   m_titleSelectIndex = (m_titleSelectIndex + itemCount - 1) % itemCount;

        // 変わった時だけ、View に選んでいる項目を伝える
        const TitleTextType selectType = static_cast<TitleTextType>(m_titleSelectIndex);
        if (m_titleSelectIndex != prevIndex && m_titleEventMenu)
        {
            m_titleEventMenu->SetSelect(selectType);
        }

        // A で決める
        if (!g_pad[0]->IsTrigger(enButtonA)) return;
        switch (selectType)
        {
            case TitleTextType::Start:
                // すぐには移らずに、カメラのフラッシュのように画面を真っ白にしてから移る
                m_isStartFlash = true;
                if (m_titleEventMenu) m_titleEventMenu->StartScreenFlash();
                sound::SoundManager::GetInstance()->Play(sound::EnSoundType::SE, sound::se::SHUTTER);
                break;
            case TitleTextType::Sound: m_soundOption.Open();     break;
            case TitleTextType::End:   PostQuitMessage(0);       break;
            default: break;
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
        // 「Press A Button」(消している時は描画されない)
        m_pressButtonLayout.Render(rc);
        // 背景の上に重ねる(閉じているときは描画されない)
        m_soundOption.Render(rc);
    }


    bool TitleScene::RequestScene(uint32_t& id, float& waitTime)
    {
        // サウンドオプションを開いている間は、Aボタンをメニューが使うので遷移しない
        if (m_soundOption.IsOpen())
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