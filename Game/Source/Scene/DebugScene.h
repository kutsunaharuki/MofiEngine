/**
 * @file DebugScene.h
 * @brief 開発用のシーン
 */
#pragma once
#include "IScene.h"
#include "Source/Camera/FinderController.h"
#include <memory>


namespace app
{
	/** 前方宣言 */
	namespace camera
	{
		class GameCamera;
	}

	namespace actor
	{
		class Boy;
	}


	/**
	 * @brief 開発用のシーン
	 * @details 地面・主人公・カメラ・撮影だけを置いて、動きや値をすぐに確かめるためのシーン。
	 *          ImGui の「Parameters」で、Assets/parameter の JSON の値をその場で変えて、保存できる。
	 *          タイトルで Select(スペースキー)を押すと入れて、もう一度 Select でタイトルに戻る(K2_DEBUG の構成だけ)
	 */
	class DebugScene : public IScene
	{
		appScene(DebugScene);


	public:
		DebugScene();
		~DebugScene() override;

		bool Start() override;
		void Update() override;
		void Render(RenderContext& rc) override;
		bool RequestScene(uint32_t& id, float& waitTime) override;


	private:
		/**
		 * @brief 今の状態を表示する ImGui の画面を出す
		 * @param isShot このフレームで写真を撮ったか
		 */
		void DrawDebugWindow(const bool isShot);


	private:
		/** 写真撮影クラス */
		PhotoCapture m_photoCapture;
		/** ファインダーの操作 */
		camera::FinderController m_finderController;
		/** 主人公 */
		std::unique_ptr<actor::Boy> m_boy;
		/** ゲームカメラ */
		std::unique_ptr<camera::GameCamera> m_gameCamera;
		/** 地面のモデルレンダー */
		ModelRender m_groundModelRender;
		/** 地面の当たり判定 */
		PhysicsStaticObject m_groundPhysicsObject;
		/** 撮った写真の枚数 */
		int m_shotCount;
	};
}
