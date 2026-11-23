/**
 * @file InGameScene.h
 * @brief インゲームシーン
 */
#pragma once
#include "Source/Core/TransformNode.h"
#include "IScene.h"
#include "Source/UI/UILayout.h"
#include "Source/UI/SoundOptionMenu/SoundOptionMenu.h"
#include "Source/Camera/FinderController.h"
#include "Source/Actor/Character/Enemy/EnemyController.h"
#include "Source/Mission/MissionManager.h"
#include "Source/DebugTool/NavigationDebugWindow.h"
#include "Source/DebugTool/LoadTimeWindow.h"
#include "Source/DebugTool/PhotoMatchDebugWindow.h"
#include "Source/Stage/LightController.h"
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
		/** ミッションの表示 */
		class MissionMenu;
		/** ミッションの見本の絵の表示 */
		class SampleMenu;
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
		/** 一般客の管理場所 */
		class VisitorManager;
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
		/** ミッションの見本の絵を描く */
		class PhotoSampleRenderer;
	}

	namespace ui
	{
		/** 照合ジャッジの表示 */
		class JudgeMenu;
		/** 制限時間の表示 */
		class TimerMenu;
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
		/**
		 * @brief ゲームループ確認用のデバッグ窓(Debug の時だけ)
		 * @details 撮らずにミッションを進めて、インゲーム → リザルト → タイトル を流して確かめる
		 */
		void DrawGameLoopDebug();
		/**
		 * @brief ミッション(Model)から行の値を作って、ミッションの UI(View)に渡す
		 * @details MVP の Presenter の役目。枚数や状態が変わった時に呼ぶ
		 */
		void UpdateMissionMenu();
		/**
		 * @brief 被写体と同じ悪党を、敵の一覧から探す
		 * @param subject 被写体(いなければ nullptr)
		 * @return 悪党(被写体が悪党でなければ nullptr)
		 * @details 被写体は const なので、敵の一覧から同じ物を探して、const でない悪党を返す(写り方を測るのに使う)
		 */
		actor::EnemyBase* FindEnemy(const camera::IPhotoSubject* subject);
		/**
		 * @brief 見本の絵を描き進めて、UI(ミッションの欄・ファインダー)に渡す
		 * @details 出すのは「進行中で、見本を描き終わった最初のミッション」の絵。無ければ見本の枠を消す
		 */
		void UpdatePhotoSample();
		/**
		 * @brief 最後に撮った写真の照合結果から、ジャッジの値を作って、ジャッジの UI を開く(時間が止まる)
		 * @details MVP の Presenter の役目。撮った直後に呼ぶ
		 */
		void OpenJudge();
		/**
		 * @brief ジャッジの UI を開いている間の更新(閉じる入力を受ける)
		 */
		void UpdateJudge();
		/**
		 * @brief のこり時間を減らして、UI に渡す(0 になったら、次の RequestScene でリザルトへ)
		 * @details MVP の Presenter の役目。ゲームの時間が動いている時だけ呼ぶ(ジャッジ・サウンドオプションの間は止まる)
		 */
		void UpdateTimeLimit();


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
		/** 一般客の管理場所(1 人の客とグループを、何組も持つ) */
		std::unique_ptr<actor::VisitorManager> m_visitorManager;
		/** 経路探索のデバッグ表示 */
		debug::NavigationDebugWindow m_navigationDebugWindow;
		/** 読み込み時間の計測と表示 */
		debug::LoadTimeWindow m_loadTimeWindow;
		/** 照合のデバッグ表示(今の写り方を出して、見本の数値を決める道具。Debug の時だけ) */
		debug::PhotoMatchDebugWindow m_photoMatchDebugWindow;

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
		/** ミッションを終わらせてリザルトへ移るか(デバッグの「Finish -> Result」で立てる) */
		bool m_isFinishRequested;
		
		/** ファインダーコントローラー */
		camera::FinderController m_finderController;
		
		/** ステージの明かり(ライトの設定と影の範囲) */
		stage::LightController m_lightController;

		// UI関連
		/** サウンドオプション(入力と音量の受け渡し。メニューの UI も持っている) */
		ui::SoundOptionController m_soundOption;
		/** ミッションのUIレイアウト */
		ui::UILayout m_missionLayout;
		/** ミッションメニュー */
		ui::MissionMenu* m_missionMenu;
		/** 見本の絵のUIレイアウト */
		ui::UILayout m_sampleLayout;
		/** 見本の絵のメニュー */
		ui::SampleMenu* m_sampleMenu;
		/** 照合ジャッジのUIレイアウト */
		ui::UILayout m_judgeLayout;
		/** 照合ジャッジのメニュー */
		ui::JudgeMenu* m_judgeMenu;
		/** 照合ジャッジを開いているか(開いている間は、ゲームの時間を止める) */
		bool m_isJudgeOpen;
		/** 制限時間のUIレイアウト */
		ui::UILayout m_timerLayout;
		/** 制限時間のメニュー */
		ui::TimerMenu* m_timerMenu;
		/** のこり時間(秒) */
		float m_remainTime;
		/** 時間切れになったか */
		bool m_isTimeUp;

		/** 敵の入力(ステージから美術品を探して、盗みに行く一覧を敵に渡す) */
		actor::EnemyController m_enemyController;

		/** ミッションの管理(シーンと同じ寿命。リトライで作り直される) */
		mission::MissionManager m_missionManager;
		/** ミッションごとの見本の絵(ミッションの一覧と同じ順番。見本が無いミッションは nullptr) */
		std::vector<std::unique_ptr<mission::PhotoSampleRenderer>> m_photoSampleList;
	};
}