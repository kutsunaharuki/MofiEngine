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
    /** シングルトンインスタンス */
    SceneManager* SceneManager::m_instance = nullptr;



    SceneManager::SceneManager()
        : m_currentScene(nullptr)
    {
        // ここでシーンを追加
        AddSceneMap<app::InGameScene>();
        AddSceneMap<app::ResultScene>();
        AddSceneMap<app::TitleScene>();

        // 最初のシーンを作成
        CreateScene(TitleScene::ID());
    }

    
    SceneManager::~SceneManager()
    {
        // m_currentScene は unique_ptrなので自動的に解放される
    }


    void SceneManager::Update()
    {
        switch (m_transitionState)
        {
        // 通常のゲームプレイ
        case TransitionState::Idle:
        {
            // シーンが無ければ何もしない
            if (!m_currentScene) return;

            // 通常のゲームプレイのシーンを更新
            m_currentScene->Update();

            // シーン遷移の要求があれば、フェードアウトを開始
            if (m_currentScene->RequestScene(m_nextSceneId, m_fadeDuration))
            {
                m_fadeElapsedTimer = 0.0f;
                m_transitionState = TransitionState::FadingOut;
            }
            break;
        }

        // フェードアウト
        case TransitionState::FadingOut:
        {
            // 画面が暗くなっている間はこのシーンの更新を止める
            m_fadeAlpha = 0.0f;
            
            if (UpdateFadeTimer())
            {
                // 真っ暗になったので古いシーンを破棄
                m_currentScene.reset();
                // ロード画面に遷移
                m_transitionState = TransitionState::Loading;
            }
            break;
        }

        // シーンのロード中
        case TransitionState::Loading:
        {
            // 画面が真っ暗な状態で次のシーンを作成する
            if (!m_currentScene && m_nextSceneId != INVALID_SCENE_ID)
            {
                // 次のシーンを作成
                CreateScene(m_nextSceneId);
                // 作成した後は次のシーンIDを無効値にする
                m_nextSceneId = INVALID_SCENE_ID;
            }
            
            if (m_currentScene)
            {
                // 今のシーンを更新
                m_currentScene->Update();

                // シーンのロードが完了したら
                if (m_currentScene->IsLoaded())
                {
                    // フェードインを開始
                    m_fadeElapsedTimer = 0.0f;
                    m_transitionState = TransitionState::FadingIn;
                }
            }
            break;
        }
        
        // フェードイン
        case TransitionState::FadingIn:
        {
            if (m_currentScene)
            {
                // フェードイン中はこのシーンを更新
                m_currentScene->Update();
            }

            if (UpdateFadeTimer())
            {
                // フェードインが完了したので通常のゲームプレイに戻る
                m_fadeDuration = 0.0f;
                m_transitionState = TransitionState::Idle;
            }
            break;
        }

        }
    }


    void SceneManager::Render(RenderContext & rc)
    {
        // シーンがなければ何もしない
        if (!m_currentScene) return;

        // ロード中は、シーンを描画しない(この時は真っ暗)
        if (m_transitionState == TransitionState::Loading) return;
        
        m_currentScene->Render(rc);
    }
    
    
    void SceneManager::CreateScene(const uint32_t id)
    {
        // シーンIDを探す
        auto it = m_sceneMap.find(id);

        // m_sceneMapにシーンが追加されていなければ警告
        if (it == m_sceneMap.end())
        {
            K2_ASSERT(false, "新規シーンが追加されていません。");
            return;
        }

        // シーンを生成して、Start()を呼ぶ
        m_currentScene = it->second();
        m_currentScene->Start();
    }


    bool SceneManager::UpdateFadeTimer()
    {
        m_fadeElapsedTimer += g_gameTime->GetFrameDeltaTime();

        // 持続時間が0以下なら、すぐに完了とみなす
        float rate = 1.0f;
        if (m_fadeDuration > 0.0f)
        {
            rate = m_fadeElapsedTimer / m_fadeDuration;
            if (rate > 1.0f)
            {
                rate = 1.0f;
            }
        }

        // フェードアウト中は 0->1、フェードイン中は 1->0になる
        m_fadeAlpha = (m_transitionState == TransitionState::FadingOut) ? rate : 1.0f - rate;
        
        return rate >= 1.0f;
    }
}