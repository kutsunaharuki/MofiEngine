/**
 * @file InGameScene.h
 * @brief インゲームシーン
 */
#pragma once
#include "Source/Core/TransformNode.h"
#include "IScene.h"
#include "Source/UI/UILayout.h"
#include "Source/Camera/FinderController.h"
#include <memory>
#include <string>


namespace app
{
	/** 前方宣言 */
	class Enemy;

	namespace ai
	{
		class NavGrid;
		class Pathfinder;
	}

	namespace ui
	{
		class UILayout;
		class SoundOptionMenu;
	}

	namespace camera
	{
		class GameCamera;
		class FinderController;
	}

	namespace core
	{
		class TransformNode;
	}

	namespace actor
	{
		class Boy;
		class EnemyBase;
	}

	namespace stage
	{
		class Stage;
		class StageEditor;
	}



	/**
	 * @brief インゲームシーン
	 */
	class InGameScene : public IScene
	{
		appScene(InGameScene);


	public:
		InGameScene();
		~InGameScene() override;

		bool Start() override;
		void Update() override;
		void Render(RenderContext& rc) override;

		/**
		 * @brief シーンの移行を要求する
		 * @param id シーンID
		 * @param waitTime シーン遷移するまでの時間
		 * @return true:移行する, false:移行しない
		 */
		bool RequestScene(uint32_t& id, float& waitTime) override;


	private:
		/**
		 * @brief 経路探索のデバッグ表示
		 * @details NavGridのミニマップ(赤:通れない)と敵の道順を描く。
		 *          ボタンで敵を主人公の位置へ向かわせたり、地図を作り直したりできる。
		 */
		void DrawNavigationDebug();


	private:
		/** 写真撮影クラス */
		nsK2EngineLow::PhotoCapture m_photoCapture;
		/** 敵 */
		std::unique_ptr<app::actor::EnemyBase> m_enemy;
		/** 主人公 */
		std::unique_ptr<app::actor::Boy> m_boy;
		/** ステージ(置物) */
		std::unique_ptr<app::stage::Stage> m_stage;
		/** ステージエディタ(Debug の時だけ作る。Release では nullptr) */
		std::unique_ptr<app::stage::StageEditor> m_stageEditor;

		/** 敵の一覧 */
		std::vector<std::unique_ptr<actor::EnemyBase>> m_enemyList;
		/** 経路探索のデバッグ表示に出す、最後の操作の結果 */
		std::string m_navDebugMessage;

		/** 経路探索の地図 */
		std::unique_ptr<app::ai::NavGrid> m_navGrid;
		/** 道順を求める */
		std::unique_ptr<app::ai::Pathfinder> m_pathfinder;

		/** ゲームカメラ */
		std::unique_ptr<app::camera::GameCamera> m_gameCamera;
		/** スプライトレンダー */
		SpriteRender* m_spriteRender;
		/** 地面の当たり判定(主人公が地面に立つために必要) */
		PhysicsStaticObject m_groundPhysicsObject;
		/** 主人公の初期位置 */
		Vector3 m_position;
		/** 地面の位置 */
		Vector3 m_gPosition;
		/** 地面の大きさ */
		Vector3 m_gScale;
		/** 地面の回転 */
		Quaternion m_gRotation;
		/** ピントの判定を記録 */
		bool m_isLastShotInFocus;
		
		/** ファインダーコントローラー */
		camera::FinderController m_finderController;

		// UI関連
		/** SoundOptionのUIレイアウト */
		ui::UILayout m_soundOptionLayout;
		/** サウンドオプションメニュー */
		ui::SoundOptionMenu* m_soundOptionMenu;

	};
}