/**
 * @file InGameScene.cpp
 * @brief インゲームシーン
 */
#include "stdafx.h"
#include "SceneManager.h"
#include "InGameScene.h"
#include "ResultScene.h"
#include "Source/Camera/GameCamera.h"
#include "Source/Actor/Character/Enemy/EnemyBase.h"
#include "Source/Actor/Character/Enemy/EnemyStateMachine.h"
#include "Source/Actor/Character/Boy/Boy.h"
#include "Source/UI/SoundOptionMenu/SoundOptionMenu.h"
#include "Source/UI/MissionMenu/MissionMenu.h"
#include "Source/Sound/SoundManager.h"
#include "Source/Stage/Stage.h"
#include "Source/Actor/StageObject/StageObject.h"
#include "Source/Stage/StageEditor.h"
#include "Source/Stage/StageLayout.h"
#include "Source/Util/ParameterManager.h"
#include "Source/AI/Navigation/NavGrid.h"
#include "Source/AI/Navigation/Pathfinder.h"


namespace app
{
	namespace
	{
		/** 移動速度 */
		constexpr float MOVE_SPEED = 1.0f;
		/** 回転速度 */
		constexpr float ROTATE_SPEED = 2.0f;
		/** 拡縮速度 */
		constexpr float SCALE_SPEED = 0.01f;
		/** 初期位置 */
		const Vector3 INITIALIZE_POSITION = { 0.0f,0.0f,0.0f };
		/** 初期スケール */
		const Vector3 INITIALIZE_SCALE = { 2.0f,2.0f,2.0f };
		/** 子の座標 */
		const Vector3 CHILD_POSITION = { 10.0f,0.0f,0.0f };
		/** サウンドオプションのレイアウトのパス */
		constexpr const char* SOUND_OPTION_LAYOUT_PATH = "Assets/parameter/UI/Sound/SoundOption.json";
		/** ミッションのレイアウトのパス */
		constexpr const char* MISSION_LAYOUT_PATH = "Assets/parameter/UI/Mission/Mission.json";
		/** ミッションの一覧のパス */
		constexpr const char* MISSION_LIST_PATH = "Assets/parameter/Mission/MissionList.json";
		/** ステージの配置(エディタで保存する JSON) */
		constexpr const char* STAGE_FILE_PATH = "Assets/stage/Museum.json";
		/** ステージの最初の配置(3ds Max で作った tkl。JSON が無い時に使う) */
		constexpr const char* STAGE_TKL_PATH = "Assets/modelData/Stage/Museum.tkl";
		/** ステージのモデルのフォルダ */
		constexpr const char* STAGE_MODEL_DIRECTORY = "Assets/modelData/Stage/";
		/** ステージの構造(階・区画・階段) */
		constexpr const char* STAGE_LAYOUT_PATH = "Assets/stage/MuseumLayout.json";
		/** 明かりのパラメーターパス */
		constexpr const char* LIGHT_PARAMETER_PATH = "Assets/parameter/Light/Light.json";
	}


	InGameScene::InGameScene()
		: m_spriteRender(nullptr)
		, m_position(INITIALIZE_POSITION)
		, m_gPosition(Vector3::Zero)
		, m_gRotation(Quaternion::Identity)
		, m_gScale(Vector3::One)
		, m_gameCamera(nullptr)
		, m_isLastShotInFocus(false)
		, m_soundOptionMenu(nullptr)
		, m_missionMenu(nullptr)
	{}
	
	
	InGameScene::~InGameScene()
	{
		if (m_spriteRender != nullptr)
		{
			delete m_spriteRender;
			m_spriteRender = nullptr;
		}

		// ファインダーコントローラーから敵の登録を解除
		for(auto& enemy : m_enemyList)
		{
			m_finderController.UnregisterSubject(enemy.get());
		}

		// PhotoCaptureの登録を外す
		RenderingEngine::GetInstance().SetPhotoCapture(nullptr);
	}
	
	
	bool InGameScene::Start()
	{
		// 明かりの設定を読む
		m_lightParameter.Initialize(LIGHT_PARAMETER_PATH);

		// 読み込み時間を区間ごとに計る(結果は Load Time ウィンドウと出力ウィンドウに出る)
		m_loadTimeWindow.Begin("InGameScene::Start");

		// サウンドオプションのUIを作る(最初は閉じている)
		m_soundOptionLayout.Initialize<ui::SoundOptionMenu>(SOUND_OPTION_LAYOUT_PATH);
		m_soundOptionMenu = m_soundOptionLayout.GetMenu<ui::SoundOptionMenu>();

		// ミッションを読み込んでから、ミッションの UI に渡す(渡した時に行が作られる)
		m_missionManager.Initialize(MISSION_LIST_PATH);
		m_missionLayout.Initialize<ui::MissionMenu>(MISSION_LAYOUT_PATH);
		m_missionMenu = m_missionLayout.GetMenu<ui::MissionMenu>();
		if (m_missionMenu)
		{
			m_missionMenu->SetMissionManager(&m_missionManager);
		}

		// タイトルのBGMを鳴らす
		sound::SoundManager::GetInstance()->Play(sound::EnSoundType::BGM, sound::bgm::TITLE);
		m_loadTimeWindow.Mark("UI / Mission / BGM");

		// ステージの置物を作る。エディタで保存した JSON が無ければ、3ds Max の tkl から作る
		m_stage = std::make_unique<stage::Stage>();
		if (!m_stage->Load(STAGE_FILE_PATH))
		{
			m_stage->ImportTkl(STAGE_TKL_PATH, STAGE_MODEL_DIRECTORY);
		}
		m_loadTimeWindow.Mark("Stage Load");

		// 経路探索の地図を作る
		m_navGrid = std::make_unique<ai::NavGrid>();
		m_navGrid->Build(*m_stage);

		// 経路探索の地図ができてから作る
		m_pathfinder = std::make_unique<ai::Pathfinder>(*m_navGrid);
		m_loadTimeWindow.Mark("NavGrid Build");

		// 敵の一覧に大男を追加する
		m_enemyList.push_back(std::make_unique<actor::EnemyBase>(actor::EnemyBase::EnemyType::BigMan));
		// 敵の一覧にねずみ男を追加する
		m_enemyList.push_back(std::make_unique<actor::EnemyBase>(actor::EnemyBase::EnemyType::RatMan));

		// ステージの構造(階・区画・階段)を読み込む
		m_stageLayout = std::make_unique<stage::StageLayout>();
		m_stageLayout->Load(STAGE_LAYOUT_PATH);

		// ステージの置物から、盗みに行く美術品を探しておく(階を決めるので、構造を先に渡す)
		m_enemyController.SetLayout(m_stageLayout.get());
		m_enemyController.CollectArtTargets(*m_stage);
		m_enemyController.CollectHidePoints(*m_stage);
		m_loadTimeWindow.Mark("StageLayout / Targets");


		// 敵の一覧の敵に経路探索の地図を設定して
		for (auto& enemy : m_enemyList)
		{
			enemy->SetNavigation(m_pathfinder.get(), m_navGrid.get());
			// 敵の入力に登録する(美術品の一覧が渡される)
			m_enemyController.RegisterEnemy(enemy.get());
			enemy->Start();
		}
		m_loadTimeWindow.Mark("Enemy Start");


#ifdef APP_ENABLE_PARAMETER_HOTRELOAD
		// ステージエディタは Debug の時だけ
		m_stageEditor = std::make_unique<stage::StageEditor>(m_stage.get(), STAGE_FILE_PATH, STAGE_TKL_PATH, STAGE_MODEL_DIRECTORY);
		m_loadTimeWindow.Mark("StageEditor");
#endif

		// 主人公を作る
		m_boy = std::make_unique<app::actor::Boy>();
		m_boy->Start();
		m_loadTimeWindow.Mark("Boy Start");

		// 大男とねずみ男に見る相手を設定する
		for (auto& enemy : m_enemyList)
		{
			enemy->SetPlayer(m_boy.get());
		}

		// 写真撮影クラスの初期化
		m_photoCapture.Init();

		// RenderingEngineにPhotoCaptureを登録
		RenderingEngine::GetInstance().SetPhotoCapture(&m_photoCapture);
		// ファインダーの操作(覗く・ピント・ズーム・撮影)を FinderController に任せる
		m_finderController.Initialize(&m_photoCapture);

		// ファインダーコントローラーに敵を登録する
		for (auto& enemy : m_enemyList)
		{
			m_finderController.RegisterSubject(enemy.get());
		}
		m_loadTimeWindow.Mark("PhotoCapture / Finder");
		
		// 行き先は目的地モードが自動で決める
		for (auto& enemy : m_enemyList)
		{
			// 最初のフレームの位置と向きを決めておく(ファインダーが顔の位置を計算するため)
			enemy->Update();
		}

		// 主人公に追従するカメラを作成
		m_gameCamera = std::make_unique<app::camera::GameCamera>();
		m_gameCamera->Start();

		// カメラは主人公を追いかけて、主人公はカメラの向きを基準に動く
		m_gameCamera->SetTarget(&m_boy->GetTransform());
		m_boy->SetGameCamera(m_gameCamera.get());
		m_gameCamera->Update();

		// 経路探索のデバッグ表示に、見せるものを渡す(全部作り終わってから渡す)
		debug::NavigationDebugWindow::Target navigationDebugTarget;
		navigationDebugTarget.boy = m_boy.get();
		navigationDebugTarget.enemyList = &m_enemyList;
		navigationDebugTarget.navGrid = m_navGrid.get();
		navigationDebugTarget.stage = m_stage.get();
		navigationDebugTarget.stageLayout = m_stageLayout.get();
		navigationDebugTarget.enemyController = &m_enemyController;
		m_navigationDebugWindow.Initialize(navigationDebugTarget);
		m_loadTimeWindow.Mark("First Update / Camera");

		m_loadTimeWindow.End();
		return true;
	}
	
	
	void InGameScene::Update()
	{
		// 明かりの色と向き(JSON を書き換えると、ホットリロードで値がすぐ反映する)
		const stage::LightParameter& light = m_lightParameter.Get();
		SceneLight& sceneLight = SceneLight::GetInstance();
		sceneLight.SetColor(light.directionColor);
		sceneLight.SetAmbient(light.ambient);
		sceneLight.SetDirection(light.directionDir);


		// ステージの更新と、エディタ(Debug の時だけ)
		// ※ サウンドオプションを開いている間も編集できるように、一番先に呼ぶ
		{
			// 置物の数が多いので、どれくらいかかっているかを計る
			K2_PROFILE_CPU("Stage Update");
			m_stage->Update();
		}
		if (m_stageEditor != nullptr)
		{
			K2_PROFILE_CPU("Stage Editor");
			m_stageEditor->Update();
		}

		// 経路探索のデバッグ表示(Debug の時だけ)
		// 内部でデバッグ時にのみ ImGui を表示する用にしているため
		// InGame 内で Debug と区切らなくてもよい
		m_navigationDebugWindow.Draw();
		// 読み込み時間の表示(Debug / Preview の時だけ)
		m_loadTimeWindow.Draw();

		// サウンドオプションの更新(開いているときだけ、中で入力を受け付ける)
		m_soundOptionLayout.Update();
		// ミッションの UI の更新(入力は使わないので、サウンドオプションを開いていても呼ぶ)
		m_missionLayout.Update();

		// スタートボタンで、サウンドオプションを開く
		// ※ メニューの更新の後に開くので、開いたフレームの入力でメニューが動くことはない
		if (m_soundOptionMenu && !m_soundOptionMenu->IsOpen() && g_pad[0]->IsTrigger(enButtonStart))
		{
			m_soundOptionMenu->Open();
		}

		// サウンドオプションを開いている間は、ゲームを止める
		// ※ 止めないと、メニューの A(試し聞き)で主人公がジャンプしたり、B(閉じる)で撮影したりしてしまう
		if (m_soundOptionMenu && m_soundOptionMenu->IsOpen())
		{
			return;
		}

		// ファインダーの操作(覗く・十字の上下でピント・RB1/LB1)でズーム・Bで撮影
		const bool isShot = m_finderController.Update();

		// 主人公の更新(ファインダーを覗いている間は、撮影ステートになる)
		m_boy->SetIsFinderActive(m_photoCapture.IsFinderActive());
		{
			K2_PROFILE_CPU("Boy Update");
			m_boy->Update();
			// 影は主人公の周りだけ作る
			RenderingEngine::GetInstance().SetShadowCenter(m_boy->GetTransform().GetWorldPosition());
		}

		// ファインダーを覗いている間だけ、1人称にする
		m_gameCamera->SetFirstPeerson(m_photoCapture.IsFinderActive());
		m_gameCamera->Update();

		// ファインダー視点のカメラをゲームカメラの位置と向きに合わせる
		Camera& finderCamera = m_photoCapture.GetFinderCamera();
		finderCamera.SetPosition(g_camera3D->GetPosition());
		finderCamera.SetTarget(g_camera3D->GetTarget());
		finderCamera.Update();

		// 敵がいる階が変わっていたら、その階の美術品の一覧を渡し直す(敵の更新より先に呼ぶ)
		m_enemyController.Update();

		for(auto& enemy : m_enemyList)
		{
			K2_PROFILE_CPU("Enemy Update");
			enemy->Update();
		}


		/** ファインダーのオートフォーカス */

		// オートフォーカスとファインダーの UI
		// ※ ファインダーのカメラと敵の位置が決まった後に呼ぶ(顔の画面の位置を計算するため)
		{
			K2_PROFILE_CPU("Finder LateUpdate");
			m_finderController.LateUpdate();
		}

		// オートフォーカスが選んだ被写体(いなければ nullptr)
		const camera::IPhotoSubject* focusSubject = m_finderController.GetFocusSubject();

		// 今のピントが、選んだ被写体に合っているかどうか(被写体がいなければ、合っていない)
		// ※ ここで return すると、被写体がいないときに撮った写真が記録されず、前の結果が残ってしまう
		// ※ 真ん中の十字が、顔のまわりの丸の中に入っている(範囲内の)時だけ、合っている扱いにする
		bool isInFocus = false;
		if (focusSubject != nullptr && m_finderController.IsSubjectInRange())
		{
			// ファインダーのカメラから、被写体のピントを合わせる位置(体の中心)までの距離
			Vector3 toSubject = focusSubject->GetFocusPosition() - m_photoCapture.GetFinderCamera().GetPosition();
			isInFocus = m_photoCapture.IsInFocus(toSubject.Length());
		}

		// 撮影できたフレームだけ、この瞬間のピントの判定を記録
		if (isShot)
		{
			m_isLastShotInFocus = isInFocus;
			JudgeMissionShot(focusSubject, isInFocus);
		}
	}
	
	
	void InGameScene::Render(RenderContext& rc)
	{
		// ファインダーを覗いていない時だけ描画する
		if (!m_photoCapture.IsFinderActive())
		{
			m_boy->Render(rc);
		}

		// ステージの置物
		{
			K2_PROFILE_CPU("Stage Render (register)");
			m_stage->Render(rc);
		}

		// ファインダーの UI(構えていない時は、何も描画されない)
		m_finderController.Render(rc);

		// ミッションの UI(ファインダーを覗いている間は、写真の邪魔になるので出さない)
		if (!m_photoCapture.IsFinderActive())
		{
			m_missionLayout.Render(rc);
		}

		// サウンドオプションは毎フレーム描画を呼ぶ(閉じているときは、メニューの中で何も描画されない)
		// ※ IsTrigger の中で呼ぶと、ボタンを押した1フレームしか描画されない
		// ※ ファインダーの UI より後に描画して、手前に出す
		m_soundOptionLayout.Render(rc);

		// 敵の描画
		for (auto& enemy : m_enemyList)
		{
			enemy->Render(rc);
		}
	}
	
	
	bool InGameScene::RequestScene(uint32_t& id, float& waitTime)
	{
		// サウンドオプションを開いている間は、Aボタンをメニューが使うので遷移しない
		if (m_soundOptionMenu && m_soundOptionMenu->IsOpen())
		{
			return false;
		}

		// ゴール：全てのミッションを達成したら、すぐにリザルトへ
		if (m_missionManager.IsAllAchieved())
		{
			id = app::ResultScene::ID();
			waitTime = 1.0f;
			return true;
		}

		if (g_pad[0]->IsTrigger(enButtonA))
		{
			// インゲームを抜けるので、達成していないミッションは失敗にする
			m_missionManager.Finish();
			id = app::ResultScene::ID();
			waitTime = 1.0f;
			return true;
		}
		return false;
	}


	void InGameScene::JudgeMissionShot(const camera::IPhotoSubject* focusSubject, const bool isInFocus)
	{
		mission::ShotInfo shot;
		shot.isInFocus = isInFocus;

		// 被写体が敵なら、盗んでいるかと、何回目の盗みかを入れる
		// ※ 被写体は const なので、敵の一覧から同じ敵を探して、ステートマシンを見る
		for (auto& enemy : m_enemyList)
		{
			if (enemy.get() != focusSubject) continue;

			shot.subject = enemy.get();
			const actor::EnemyStateMachine* stateMachine = enemy->GetEnemyStateMachine();
			if (stateMachine)
			{
				shot.isStealing = stateMachine->IsStealing();
				shot.stealCount = stateMachine->GetStealCount();
			}
			break;
		}

		// 枚数が増えた時だけ、UI の文と色を変える
		if (m_missionManager.OnShot(shot) && m_missionMenu)
		{
			m_missionMenu->UpdateMissionTexts();
		}
	}
}