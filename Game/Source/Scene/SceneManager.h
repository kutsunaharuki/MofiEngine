/**
 * @file SceneManager.h
 * @brief シーンを一元管理するクラス
 */
#pragma once
#include "Source/Scene/IScene.h"


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

	private:
		/** シーン遷移の状態 */
		enum class TransitionState
		{
			Idle,          // 通常のゲームプレイ
			FadOut,		   // 画面が暗くなる
			Loading,       // シーンのロード中
			FadeIn		   // ロー後画面が消えて明るくなる
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
			m_sceneMap.emplace(T::ID(), []()
				{
					return new T();
				});
		}

		/**
		 * @brief シーンを生成する
		 * @param id シーンID
		 */
		void CreateScene(const uint32_t id);


	private:
		/** シングルトンインスタンス */
		static SceneManager* m_instance;
		/** シーンマップ */
		std::map<uint32_t, std::function<IScene* ()>> m_sceneMap;
		/** 現在のシーン */
		IScene* m_currentScene;
		/** 次のシーンID */
		uint32_t m_nextSceneId = INVALID_SCENE_ID;
		/** シーン遷移の状態 */
		TransitionState m_transitionState = TransitionState::Idle;
		/** フェードの持続時間 */
		float m_fadeDuration = 0.0f;
	};
}