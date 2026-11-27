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
#include "Source/Actor/Character/Boy/PhotoChance.h"
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
	}

	namespace ui
	{
		/** 照合ジャッジの表示 */
		class JudgeMenu;
		/** 制限時間の表示 */
		class TimerMenu;
		/** ポーズ画面の表示 */
		class PauseMenu;
		/** 悪党に見つかった時の表示 */
		class FoundMenu;
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
		 * @brief ゲームに要る物を全部作り終わったか
		 * @return 作り終わったら true(SceneManager はそれまでロード画面を出す)
		 */
		bool IsLoaded() const override { return m_loadStep == LoadStep::Done; }

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
		 * @brief 今の写り方の情報を作る(撮った時と、ファインダーのチェック欄で同じ物を使う)
		 * @param focusSubject オートフォーカスが選んだ被写体(いなければ nullptr)
		 * @param isInFocus ピントが合っているか
		 * @return 写り方の情報(アルバムの番号はまだ入れない)
		 */
		mission::ShotInfo MakeShotInfo(const camera::IPhotoSubject* focusSubject, const bool isInFocus);
		/**
		 * @brief ファインダーのチェック欄(顔が見える・盗んでいる・ピント)を、今の写り方で入れる
		 * @param focusSubject オートフォーカスが選んだ被写体(いなければ nullptr)
		 * @param isInFocus ピントが合っているか
		 */
		void UpdateFinderCheck(const camera::IPhotoSubject* focusSubject, const bool isInFocus);
		/**
		 * @brief ゲームループ確認用のデバッグ窓(Debug の時だけ)
		 * @details 撮らずにミッションを進めて、インゲーム → リザルト → タイトル を流して確かめる
		 */
		void DrawGameLoopDebug();
		/**
		 * @brief 柱からのぞけるかを調べて、主人公に渡す
		 * @details ファインダーを覗いている時だけ調べる(覗いていなければ、のぞかない)
		 */
		void UpdatePhotoChance();
		/**
		 * @brief 近くの悪党に、シャッター音を聞かせる(音がした所へ見に来る)
		 * @param focusSubject 撮った時のピントの被写体
		 * @details 悪党の顔か背中(頭)が写っていて、間に物が無い時だけ音が出る。
		 *          音がとどく距離の中の悪党みんなが見に来て、撮られた悪党だけが主人公をマークする
		 */
		void NotifyShutterSound(const camera::IPhotoSubject* focusSubject);
		/**
		 * @brief 追いかけられている間の「！」を動かす(見失われたら消して、元に戻る)
		 */
		void UpdateFoundMark();
		/**
		 * @brief 柱からのぞいて撮った時の処理(その柱はしばらく使えない。柱の近くの悪党は気を付ける)
		 */
		void NotifyLeanShot();
		/**
		 * @brief 「！」を、見つけた悪党の頭の上に置く
		 */
		void ApplyFoundMarkPosition();
		/**
		 * @brief 見つけた悪党につかまったか
		 * @return つかまったら true(ゲームオーバーになる)
		 */
		bool IsCaught();
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
		/**
		 * @brief ポーズ画面を開く(時間が止まる)
		 */
		void OpenPause();
		/**
		 * @brief ポーズ画面の入力を受ける(上下で選ぶ・A で決める・B / START で閉じる)
		 * @details MVP の Presenter の役目
		 */
		void UpdatePause();
		/**
		 * @brief 読み込みを 1 段階だけ進める(1 フレームに 1 つずつ作る)
		 */
		void UpdateLoading();
		/**
		 * @brief 主人公を見つけた悪党を探す(見つけていたら「！」を出す)
		 * @details 悪党の更新の後に呼ぶ
		 */
		void CheckFound();
		/**
		 * @brief つかまった後の更新(画面を暗くして、ゲームオーバーにする)
		 * @details MVP の Presenter の役目。主人公は動かさない。ポーズも開かない
		 *          (見つかって追いかけられている間は、いつも通り動ける。UpdateFoundMark を見る)
		 */
		void UpdateFound();


	private:
		/**
		 * @brief 読み込みの段階(上から順に作る)
		 */
		enum class LoadStep : uint8_t
		{
			UI,				// UI とミッション
			Stage,			// ステージの置物
			NavGrid,		// 経路探索の地図
			StageLayout,	// 敵を作る・ステージの構造・盗みに行く美術品
			Enemy,			// 敵の Start
			Visitor,		// 一般客
			Boy,			// ステージエディタと主人公
			Finder,			// 写真撮影・ファインダー・明かり
			Camera,			// 最初のフレームとカメラ
			Done			// 全部作り終わった
		};


	private:
		/** 写真撮影クラス */
		nsK2EngineLow::PhotoCapture m_photoCapture;
		/** 敵 */
		std::unique_ptr<app::actor::EnemyBase> m_enemy;
		/** 主人公 */
		std::unique_ptr<app::actor::Boy> m_boy;
		/** フォトチャンス(柱からのぞいて撮れるかを調べる) */
		actor::PhotoChance m_photoChance;
		/** 悪党の位置の一覧(フォトチャンスに渡す。毎フレーム作り直さないように持っておく) */
		std::vector<Vector3> m_enemyPositionList;
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
		/** ポーズ画面のUIレイアウト */
		ui::UILayout m_pauseLayout;
		/** ポーズ画面のメニュー */
		ui::PauseMenu* m_pauseMenu;
		/** ポーズ画面で選んでいる項目の番号 */
		int m_pauseSelectIndex;
		/** ポーズ画面を開いているか(開いている間は、ゲームの時間を止める) */
		bool m_isPauseOpen;
		/** ポーズの「タイトルへ」を選んだか */
		bool m_isTitleRequested;
		/** 今の読み込みの段階 */
		LoadStep m_loadStep;
		/** 悪党に見つかった時のUIレイアウト */
		ui::UILayout m_foundLayout;
		/** 悪党に見つかった時のメニュー */
		ui::FoundMenu* m_foundMenu;
		/** 主人公を見つけた悪党(見つかっていなければ nullptr。見つかったら、ゲームオーバーの演出になる) */
		actor::EnemyBase* m_foundEnemy;

		/** 敵の入力(ステージから美術品を探して、盗みに行く一覧を敵に渡す) */
		actor::EnemyController m_enemyController;

		/** ミッションの管理(シーンと同じ寿命。リトライで作り直される) */
		mission::MissionManager m_missionManager;
	};
}