/**
 * @file SceneManager.h
 * @brief シーンを一元管理するクラス
 */
#pragma once
#include "Source/Scene/IScene.h"
#include <map>
#include <functional>
#include <memory>


namespace app
{
	/**
	 * @brief シーンを一元管理するクラス
	 */
	class SceneManager
	{
	public:
		/**
		 * @brief 更新処理
		 */
		void Update();
		/**
		 * @brief 描画処理
		 */
		void Render(RenderContext& rc);


	public:
		/**
		 * @brief SceneManagerのインスタンスを作成する
		 */
		static void CreateInstance()
		{
			if (m_instance == nullptr)
			{
				m_instance = new SceneManager;
			}
		}

		/**
		 * @brief SceneManagerのインスタンスを取得する 
		 */
		static SceneManager* GetInstance()
		{
			return m_instance;
		}

		/**
		 * @brief SceneManagerのインスタンスを破棄する
		 */
		static void DestoryInstance()
		{
			if (m_instance != nullptr)
			{
				delete m_instance;
				m_instance = nullptr;
			}
		}


	public:
		/**
		 * @brief 画面の暗さを取得
		 * @return 0.0f ~ 1.0fの範囲で画面の暗さを返す
		 * @details フェード用のスプライトを作った時にこのα値にそのまま使う
		 */
		float GetFadeAlpha() const { return m_fadeAlpha; }
		/**
		 * @brief シーン遷移中かどうか
		 * @return 遷移中なら true
		 */
		bool IsTransitioning() const { return m_transitionState != TransitionState::Idle; }


	private:
		/** シーン遷移の状態 */
		enum class TransitionState
		{
			Idle,          // 通常のゲームプレイ
			FadingOut,		   // 画面が暗くなる
			Loading,       // シーンのロード中
			FadingIn		   // ロー後画面が消えて明るくなる
		};


	private:
		SceneManager();
		~SceneManager();


	private:
		/**
		 * @brief SceneMapにシーンを追加する
		 * @tparam T 追加するシーンの型
		 * @return T型のオブジェクトをnewで生成しポインタを返す
		 */
		template<typename T>
		void AddSceneMap()
		{
			// 前はreturn new T();だったが、それだとメモリリークになる
			m_sceneMap.emplace(T::ID(), []()
				{
					return std::make_unique<T>();
				});
		}

		/**
		 * @brief シーンを生成する
		 * @param id シーンID
		 */
		void CreateScene(const uint32_t id);

		/**
		 * @brief フェードの進み具合を更新
		 * @return フェードが完了したら true
		 */
		bool UpdateFadeTimer();


	private:
		/** シングルトンインスタンス */
		static SceneManager* m_instance;
		/** シーンマップ */
		std::map<uint32_t, std::function<std::unique_ptr<IScene>()>> m_sceneMap;
		/** 現在のシーン */
		std::unique_ptr<IScene> m_currentScene;
		/** 次のシーンID */
		uint32_t m_nextSceneId = INVALID_SCENE_ID;
		/** シーン遷移の状態 */
		TransitionState m_transitionState = TransitionState::Idle;
		/** フェードの持続時間 */
		float m_fadeDuration = 0.0f;
		/** フェードの経過時間 */
		float m_fadeElapsedTimer = 0.0f;
		/** 画面の暗さ(0 ~ 1) */
		float m_fadeAlpha = 0.0f;
	};
}