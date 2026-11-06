/**
 * @file InGameScene.h
 * @brief インゲームシーン
 */
#pragma once
#include "Source/Core/TransformNode.h"
#include "IScene.h"
#include "Source/UI/UILayout.h"
#include "Source/Camera/FinderController.h"
#include "Source/Actor/Character/Enemy/EnemyController.h"
#include "Source/Manager/MissionManager.h"
#include "Source/DebugTool/NavigationDebugWindow.h"
#include <memory>
#include <string>
#include <vector>


namespace app
{
	/** 前方宣言 */
	class Enemy;

	namespace ai
	{
		/** 経路探索の地図 */
		class NavGrid;
		/** 経路探索の道順 */
		class Pathfinder;
	}

	namespace ui
	{
		/** UI の大元 */
		class UILayout;
		/** サウンド設定 */
		class SoundOptionMenu;
		/** ミッションの表示 */
		class MissionMenu;
	}

	namespace camera
	{
		/** プレイヤーに追従するカメラ */
		class GameCamera;
		/** ファインダー入力 */
		class FinderController;
		/** 写真の被写体 */
		class IPhotoSubject;
	}

	namespace core
	{
		/** 親子付けのトランスフォームノード */
		class TransformNode;
	}

	namespace actor
	{
		/** プレイヤー(少年) */
		class Boy;
		/** 敵の基盤 */
		class EnemyBase;
	}

	namespace stage
	{
		/** ステージ */
		class Stage;
		/** ステージエディタ(ポリゴン数とリスト化) */
		class StageEditor;
		/** ステージレイアウト */
		class StageLayout;
	}

	namespace mission
	{
		/** ミッション管理 */
		class MissionManager;
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
		 * @brief 撮影した写真を、ミッションに数えてもらう
		 * @param focusSubject オートフォーカスが選んだ被写体(いなければ nullptr)
		 * @param isInFocus ピントが合っているか
		 */
		void JudgeMissionShot(const camera::IPhotoSubject* focusSubject, const bool isInFocus);


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
		/** ステージの構造(階・区画・階段) */
		std::unique_ptr<app::stage::StageLayout> m_stageLayout;

		/** 敵の一覧 */
		std::vector<std::unique_ptr<actor::EnemyBase>> m_enemyList;
		/** 経路探索のデバッグ表示 */
		debug::NavigationDebugWindow m_navigationDebugWindow;

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
		/** ミッションのUIレイアウト */
		ui::UILayout m_missionLayout;
		/** ミッションメニュー */
		ui::MissionMenu* m_missionMenu;

		/** 敵の入力(ステージから美術品を探して、盗みに行く一覧を敵に渡す) */
		actor::EnemyController m_enemyController;

		/** ミッションの管理(シーンと同じ寿命。リトライで作り直される) */
		mission::MissionManager m_missionManager;
	};
}