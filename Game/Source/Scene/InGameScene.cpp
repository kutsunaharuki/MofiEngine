/**
 * @file InGameScene.cpp
 * @brief インゲームシーン
 */
#include "stdafx.h"
#include "SceneManager.h"
#include "InGameScene.h"
#include "ResultScene.h"
#include "TitleScene.h"
#include "Source/Camera/GameCamera.h"
#include "Source/Actor/Character/Enemy/EnemyBase.h"
#include "Source/Actor/Character/Enemy/EnemyStateMachine.h"
#include "Source/Actor/Character/Boy/Boy.h"
#include "Source/Actor/Character/Visitor/VisitorManager.h"
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
#include "Source/Mission/ShotMeasure.h"
#include "Source/UI/SampleMenu/SampleMenu.h"
#include "Source/Mission/PhotoSampleRenderer.h"
#include "Source/Manager/GameResult.h"
#include "Source/UI/JudgeMenu/JudgeMenu.h"
#include "Source/UI/TimerMenu/TimerMenu.h"
#include "Source/UI/PauseMenu/PauseMenu.h"
#include "Source/UI/FoundMenu/FoundMenu.h"
#include "Source/Util/ScreenPosition.h"
#include "Source/Mission/MissionText.h"
#include "imgui.h"


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
		/** 見本の絵のレイアウトのパス */
		constexpr const char* SAMPLE_LAYOUT_PATH = "Assets/parameter/UI/Sample/Sample.json";
		/** 照合ジャッジのレイアウトのパス */
		constexpr const char* JUDGE_LAYOUT_PATH = "Assets/parameter/UI/Judge/Judge.json";
		/** 制限時間のレイアウトのパス */
		constexpr const char* TIMER_LAYOUT_PATH = "Assets/parameter/UI/Timer/Timer.json";
		/** ポーズ画面のレイアウトのパス */
		constexpr const char* PAUSE_LAYOUT_PATH = "Assets/parameter/UI/Pause/Pause.json";
		/** 悪党に見つかった時のレイアウトのパス */
		constexpr const char* FOUND_LAYOUT_PATH = "Assets/parameter/UI/Found/Found.json";
		/** 「！」を出す高さ(悪党の頭から上へ) */
		constexpr float FOUND_MARK_HEIGHT = 35.0f;
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
		/** 一般客を何組出すか */
		constexpr const char* VISITOR_SPAWN_PATH = "Assets/parameter/Actor/Character/Visitor/VisitorSpawn.json";
	}


	InGameScene::InGameScene()
		: m_spriteRender(nullptr)
		, m_position(INITIALIZE_POSITION)
		, m_gPosition(Vector3::Zero)
		, m_gRotation(Quaternion::Identity)
		, m_gScale(Vector3::One)
		, m_gameCamera(nullptr)
		, m_isLastShotInFocus(false)
		, m_isFinishRequested(false)
		, m_missionMenu(nullptr)
		, m_sampleMenu(nullptr)
		, m_judgeMenu(nullptr)
		, m_isJudgeOpen(false)
		, m_timerMenu(nullptr)
		, m_remainTime(0.0f)
		, m_isTimeUp(false)
		, m_pauseMenu(nullptr)
		, m_pauseSelectIndex(0)
		, m_isPauseOpen(false)
		, m_isTitleRequested(false)
		, m_loadStep(LoadStep::UI)
		, m_foundMenu(nullptr)
		, m_foundEnemy(nullptr)
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
		// 読み込み時間を区間ごとに計る(結果は Load Time ウィンドウと出力ウィンドウに出る)
		m_loadTimeWindow.Begin("InGameScene::Start");

		// ここでは作らない。Update で 1 フレームに 1 つずつ作っていき、全部作り終わったら IsLoaded が true になる
		// ※ 1 回で全部作ると、その間ロード画面も止まってしまうため
		m_loadStep = LoadStep::UI;
		return true;
	}


	void InGameScene::UpdateLoading()
	{
		switch (m_loadStep)
		{
		case LoadStep::UI:
		{
			// UI とミッション
			// サウンドオプションのUIを作る(最初は閉じている)
			m_soundOption.Initialize(SOUND_OPTION_LAYOUT_PATH);

			// ミッションを読み込んでから、ミッションの UI に渡す(渡した時に行が作られる)
			m_missionManager.Initialize(MISSION_LIST_PATH);
			// 新しいプレイを始める(前のプレイの結果と写真を空にする。リトライでもここを通る)
			GameResult::GetInstance()->Reset();
			m_missionLayout.Initialize<ui::MissionMenu>(MISSION_LAYOUT_PATH);
			m_missionMenu = m_missionLayout.GetMenu<ui::MissionMenu>();
			// ミッション(Model)から行を作って、ミッションの UI(View)に渡す
			UpdateMissionMenu();
			// 見本の絵の UI(絵は PhotoSampleRenderer が描き終わってから渡す)
			m_sampleLayout.Initialize<ui::SampleMenu>(SAMPLE_LAYOUT_PATH);
			m_sampleMenu = m_sampleLayout.GetMenu<ui::SampleMenu>();
			// 照合ジャッジの UI(撮った直後に開く)
			m_judgeLayout.Initialize<ui::JudgeMenu>(JUDGE_LAYOUT_PATH);
			m_judgeMenu = m_judgeLayout.GetMenu<ui::JudgeMenu>();
			// 制限時間の UI(のこり時間はミッションの JSON から決める)
			m_timerLayout.Initialize<ui::TimerMenu>(TIMER_LAYOUT_PATH);
			m_timerMenu = m_timerLayout.GetMenu<ui::TimerMenu>();
			m_remainTime = m_missionManager.GetTimeLimit();
			m_isTimeUp = false;
			if (m_timerMenu) m_timerMenu->SetRemainTime(m_remainTime);
			// ポーズ画面の UI(START で開く)
			m_pauseLayout.Initialize<ui::PauseMenu>(PAUSE_LAYOUT_PATH);
			m_pauseMenu = m_pauseLayout.GetMenu<ui::PauseMenu>();
			// 悪党に見つかった時の UI(見つかってから出す)
			m_foundLayout.Initialize<ui::FoundMenu>(FOUND_LAYOUT_PATH);
			m_foundMenu = m_foundLayout.GetMenu<ui::FoundMenu>();

			m_loadTimeWindow.Mark("UI / Mission");
			break;
		}
		case LoadStep::Stage:
		{
			// ステージの置物
			// ステージの置物を作る。エディタで保存した JSON が無ければ、3dsMax の tkl から作る
			m_stage = std::make_unique<stage::Stage>();
			if (!m_stage->Load(STAGE_FILE_PATH))
			{
				m_stage->ImportTkl(STAGE_TKL_PATH, STAGE_MODEL_DIRECTORY);
			}
			m_loadTimeWindow.Mark("Stage Load");
			break;
		}
		case LoadStep::NavGrid:
		{
			// 経路探索の地図
			// 経路探索の地図を作る
			m_navGrid = std::make_unique<ai::NavGrid>();
			m_navGrid->Build(*m_stage);

			// 経路探索の地図ができてから作る
			m_pathfinder = std::make_unique<ai::Pathfinder>(*m_navGrid);
			m_loadTimeWindow.Mark("NavGrid Build");
			break;
		}
		case LoadStep::StageLayout:
		{
			// 敵を作る・ステージの構造・盗みに行く美術品
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
			break;
		}
		case LoadStep::Enemy:
		{
			// 敵の Start
			// 敵の一覧の敵に経路探索の地図を設定して
			for (auto& enemy : m_enemyList)
			{
				enemy->SetNavigation(m_pathfinder.get(), m_navGrid.get());
				// 敵の入力に登録する(美術品の一覧が渡される)
				m_enemyController.RegisterEnemy(enemy.get());
				enemy->Start();
			}
			m_loadTimeWindow.Mark("Enemy Start");
			break;
		}
		case LoadStep::Visitor:
		{
			// 一般客
			// 一般客(1 人の客とグループ)を、JSON の数だけ作って並べる
			// ※ 見に行く美術品は 1 階の分だけ(経路探索の地図が 1 階の分しか無いため)
			m_visitorManager = std::make_unique<actor::VisitorManager>();
			m_visitorManager->Initialize(VISITOR_SPAWN_PATH, m_pathfinder.get(), m_navGrid.get(), m_enemyController.MakeFloorTargetList(0));
			m_loadTimeWindow.Mark("Visitor Start");
			break;
		}
		case LoadStep::Boy:
		{
			// ステージエディタ(Debug の時だけ)と主人公
#ifdef APP_ENABLE_PARAMETER_HOTRELOAD
			// ステージエディタは Debug の時だけ
			m_stageEditor = std::make_unique<stage::StageEditor>(m_stage.get(), STAGE_FILE_PATH, STAGE_TKL_PATH, STAGE_MODEL_DIRECTORY);
			m_loadTimeWindow.Mark("StageEditor");
#endif

			// 主人公を作る
			m_boy = std::make_unique<app::actor::Boy>();
			m_boy->Start();
			m_loadTimeWindow.Mark("Boy Start");
			break;
		}
		case LoadStep::Finder:
		{
			// 写真撮影・ファインダー・明かり
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
			// ピントのレイが主人公に当たらないように設定
			m_finderController.SetPhotographerCollisionObject(m_boy->GetCollisionObject());
			// 明かり(影は階ごとに作るので、ステージの構造を渡す)
			m_lightController.Initialize(m_stageLayout.get());

			// ファインダーコントローラーに敵を登録する
			for (auto& enemy : m_enemyList)
			{
				m_finderController.RegisterSubject(enemy.get());
			}
			m_loadTimeWindow.Mark("PhotoCapture / Finder");
			break;
		}
		case LoadStep::PhotoSample:
		{
			// ミッションの見本の絵
			// ミッションの見本の絵(見本のあるミッションだけ人形を作る。描くのは人形のポーズが止まってから)
			// ※ ミッションの一覧と同じ順番にする(見本の無いミッションは nullptr)
			for (const mission::MissionData& mission : m_missionManager.GetMissionList())
			{
				if (!mission.hasSample)
				{
					m_photoSampleList.push_back(nullptr);
					continue;
				}
				auto photoSample = std::make_unique<mission::PhotoSampleRenderer>();
				photoSample->Initialize(mission);
				m_photoSampleList.push_back(std::move(photoSample));
			}
			m_loadTimeWindow.Mark("Photo Sample");
			break;
		}
		case LoadStep::Camera:
		{
			// 最初のフレームとカメラ
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
			break;
		}
		default:
			break;
		}

		// 次の段階へ進む
		m_loadStep = static_cast<LoadStep>(static_cast<int>(m_loadStep) + 1);

		// 全部作り終わった
		if (m_loadStep == LoadStep::Done)
		{
			m_loadTimeWindow.End();
			// タイトルのBGMを鳴らす(読み込みが全部終わってから)
			sound::SoundManager::GetInstance()->Play(sound::EnSoundType::BGM, sound::bgm::TITLE);
		}
	}


	void InGameScene::Update()
	{
		// 読み込みが終わるまでは、作るだけ(ゲームはまだ動かさない)
		if (!IsLoaded())
		{
			UpdateLoading();
			return;
		}

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
		// ゲームループ確認用(Debug の時だけ。撮らずにミッションを進める)
		DrawGameLoopDebug();

		// サウンドオプションの更新(開いているときだけ、中で入力を受け付ける)
		// ※ 更新の前に開いていたかを残す(B で閉じたフレームに、同じ B でポーズまで閉じないようにする)
		const bool wasSoundOptionOpen = m_soundOption.IsOpen();
		m_soundOption.Update();
		// ミッションの UI の更新(入力は使わないので、サウンドオプションを開いていても呼ぶ)
		m_missionLayout.Update();
		// 制限時間の UI の更新(点滅などの動き。時間を減らすのは UpdateTimeLimit)
		m_timerLayout.Update();
		// 見本の絵を描き進めて、UI に渡す
		UpdatePhotoSample();
		m_sampleLayout.Update();

		// 照合ジャッジを開いている間は、ゲームの時間を止める(悪党・一般客・主人公・カメラを動かさない)
		// ※ スタートボタンより先に見て、ジャッジの間にサウンドオプションが開かないようにする
		if (m_isJudgeOpen)
		{
			UpdateJudge();
			return;
		}

		// 見つかった後は、ゲームオーバーの演出だけ進める(主人公は動かさない。ポーズも開かない)
		if (m_foundEnemy != nullptr)
		{
			UpdateFound();
			return;
		}

		// ポーズを開いている間は、ゲームの時間を止める(サウンドオプションも、ポーズから開く)
		if (m_isPauseOpen)
		{
			// サウンドオプションを開いている間(と、閉じたフレーム)は、ポーズの入力を受けない
			if (!wasSoundOptionOpen && !m_soundOption.IsOpen())
			{
				UpdatePause();
			}
			m_pauseLayout.Update();
			return;
		}

		// スタートボタンで、ポーズを開く
		if (g_pad[0]->IsTrigger(enButtonStart))
		{
			OpenPause();
			return;
		}

		// サウンドオプションを開いている間は、ゲームを止める
		// ※ 止めないと、メニューの A(試し聞き)で主人公がジャンプしたり、B(閉じる)で撮影したりしてしまう
		if (m_soundOption.IsOpen())
		{
			return;
		}

		// のこり時間を減らす(ジャッジ・サウンドオプションの間は、ここまで来ないので止まる)
		UpdateTimeLimit();

		// ファインダーの操作(覗く・十字の上下でピント・RB1/LB1)でズーム・RT で撮影
		const bool isShot = m_finderController.Update();

		// 主人公の更新(ファインダーを覗いている間は、撮影ステートになる)
		m_boy->SetIsFinderActive(m_photoCapture.IsFinderActive());
		// ステージエディタのパースの画面で、右クリック + WASD でカメラを飛ばしている間は、主人公を動かさない
		// ※ WASD は主人公の移動にも使っているので、止めないと一緒に歩いてしまう
		const bool isEditorCameraFlying = m_stageEditor != nullptr && m_stageEditor->IsCameraFlying();
		if (!isEditorCameraFlying)
		{
			K2_PROFILE_CPU("Boy Update");
			m_boy->Update();
		}

		// ファインダーを覗いている間だけ、1人称にする
		m_gameCamera->SetFirstPeerson(m_photoCapture.IsFinderActive());
		m_gameCamera->SetZoomRatio(m_photoCapture.GetFinderZoomRatio());
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

		// 一般客の更新(全部の組)
		m_visitorManager->Update();

		// 主人公を見つけた悪党がいれば、「！」を出す(次のフレームから、見つかった後の更新になる)
		CheckFound();


		/** ファインダーのオートフォーカス */

		// オートフォーカスとファインダーの UI
		// ※ ファインダーのカメラと敵の位置が決まった後に呼ぶ(顔の画面の位置を計算するため)
		{
			K2_PROFILE_CPU("Finder LateUpdate");
			m_finderController.LateUpdate();
		}

		// 明かりの色と向き
		m_lightController.Update();

		// オートフォーカスが選んだ被写体(いなければ nullptr)
		const camera::IPhotoSubject* focusSubject = m_finderController.GetFocusSubject();

		// 照合のデバッグ表示(Debug の時だけ。今の写り方を出して、見本の数値を決める道具)
		// ※ 悪党とファインダーのカメラを更新した後に呼ぶ(撮った時の判定と同じ値になるように)
		m_photoMatchDebugWindow.Draw(FindEnemy(focusSubject), m_photoCapture.GetFinderCamera(), m_boy->GetCollisionObject(), m_missionManager);

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

		// ファインダーのチェック欄を、今の写り方で入れる(覗いている時だけ)
		UpdateFinderCheck(focusSubject, isInFocus);

		// 撮影できたフレームだけ、この瞬間のピントの判定を記録
		if (isShot)
		{
			m_isLastShotInFocus = isInFocus;
			JudgeMissionShot(focusSubject, isInFocus);
			// 撮った直後に、照合の結果を出す(次のフレームから時間が止まる)
			OpenJudge();
		}
	}
	
	
	void InGameScene::Render(RenderContext& rc)
	{
		// 読み込みが終わるまでは、まだ作っていない物があるので描画しない
		if (!IsLoaded()) return;

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

		// 制限時間(覗いている間も、のこり時間が分かるように出す)
		m_timerLayout.Render(rc);

		// 見本の絵(覗いている時と、いない時で、出す場所が変わる)
		// ※ ファインダーの UI より後に描画して、手前に出す
		m_sampleLayout.Render(rc);

		// サウンドオプションは毎フレーム描画を呼ぶ(閉じているときは、メニューの中で何も描画されない)
		// ※ IsTrigger の中で呼ぶと、ボタンを押した1フレームしか描画されない
		// ※ ファインダーの UI より後に描画して、手前に出す
		// ポーズ画面(開いている時だけ。サウンドオプションはこの上に出す)
		if (m_isPauseOpen)
		{
			m_pauseLayout.Render(rc);
		}
		m_soundOption.Render(rc);

		// 悪党に見つかった時(「！」と、暗くする板・文字)
		if (m_foundEnemy != nullptr)
		{
			m_foundLayout.Render(rc);
		}

		// 照合ジャッジ(開いている時だけ。一番手前に出す)
		if (m_isJudgeOpen)
		{
			m_judgeLayout.Render(rc);
		}

		// 敵の描画
		for (auto& enemy : m_enemyList)
		{
			enemy->Render(rc);
		}

		// 一般客の描画(全部の組)
		m_visitorManager->Render(rc);
	}
	
	
	bool InGameScene::RequestScene(uint32_t& id, float& waitTime)
	{
		// 読み込みが終わるまでは移らない
		if (!IsLoaded())
		{
			return false;
		}
		// サウンドオプションを開いている間は、Aボタンをメニューが使うので遷移しない
		if (m_soundOption.IsOpen())
		{
			return false;
		}
		// 照合ジャッジを開いている間は移らない(最後の 1 枚で全部達成しても、照合〇 を見せてから移る)
		if (m_isJudgeOpen)
		{
			return false;
		}
		// ポーズの「タイトルへ」：リザルトを通らずにタイトルへ
		if (m_isTitleRequested)
		{
			id = TitleScene::ID();
			waitTime = 1.0f;
			return true;
		}
		// ポーズを開いている間は、ほかの理由では移らない
		if (m_isPauseOpen)
		{
			return false;
		}
		// 見つかった時：演出が終わるまで待って、ゲームオーバーとしてリザルトへ
		if (m_foundEnemy != nullptr)
		{
			if (m_foundMenu != nullptr && !m_foundMenu->IsFinished()) return false;

			m_missionManager.Finish();
			GameResult::GetInstance()->SetResult(m_missionManager, FinishReason::GameOver);
			id = app::ResultScene::ID();
			waitTime = 1.0f;
			return true;
		}


		// TODO : ゲームオーバーの条件を作る
		// ゴール：全てのミッションを達成したら、すぐにリザルトへ
		// 時間切れ：のこり時間が 0 になったら、リザルトへ
		const bool isAllAchieved = m_missionManager.IsAllAchieved();
		if (isAllAchieved || m_isTimeUp || m_isFinishRequested)
		{
			// どう終わったか(全部達成が先。最後の 1 枚と時間切れが同じフレームなら、達成にする)
			FinishReason reason = FinishReason::Other;
			if (isAllAchieved)   reason = FinishReason::Clear;
			else if (m_isTimeUp) reason = FinishReason::TimeUp;

			// 達成していないミッションを失敗にして、結果をリザルトに渡す(インゲームはこの後に消える)
			m_missionManager.Finish();
			GameResult::GetInstance()->SetResult(m_missionManager, reason);

			id = app::ResultScene::ID();
			waitTime = 1.0f;
			return true;
		}
		return false;
	}


	actor::EnemyBase* InGameScene::FindEnemy(const camera::IPhotoSubject* subject)
	{
		if (subject == nullptr) return nullptr;

		// 被写体は const なので、敵の一覧から同じ物を探して、const でない悪党を返す
		for (auto& enemy : m_enemyList)
		{
			if (enemy.get() == subject) return enemy.get();
		}
		return nullptr;
	}


	void InGameScene::JudgeMissionShot(const camera::IPhotoSubject* focusSubject, const bool isInFocus)
	{
		mission::ShotInfo shot = MakeShotInfo(focusSubject, isInFocus);

		// 撮った写真をアルバムにも残す(写真用のRTは次に撮ると上書きされ、インゲームと一緒に消えるため)
		// ※ 撮影は、このフレームの描画(RenderOffscreen)で行われるので、それより前に描き先を渡す
		RenderTarget* saveTarget = GameResult::GetInstance()->GetAlbum().Reserve(shot.photoIndex);
		m_photoCapture.SetSaveTarget(saveTarget);

		// 枚数が増えた時だけ、UI の文と色を変える
		if (m_missionManager.OnShot(shot))
		{
			UpdateMissionMenu();
		}
	}


	void InGameScene::UpdateFinderCheck(const camera::IPhotoSubject* focusSubject, const bool isInFocus)
	{
		// 覗いていない時は、測らない(重いレイを毎フレーム飛ばさない)
		if (m_boy == nullptr || !m_boy->IsFinderActive()) return;

		const mission::ShotInfo shot = MakeShotInfo(focusSubject, isInFocus);
		const mission::MatchResult result = m_missionManager.PreviewShot(shot);
		auto isOk = [&result](const mission::MatchItem item)
			{
				return result.isSuccess[static_cast<size_t>(item)];
			};

		// 顔が見える = こちらを向いている・隠れていない・枠の中
		const bool isFaceVisible = isOk(mission::MatchItem::FaceAngle)
			&& isOk(mission::MatchItem::FaceBlocked)
			&& isOk(mission::MatchItem::FaceInFrame);
		// 盗んでいる = 狙いの悪党が盗んでいる
		const bool isStealing = isOk(mission::MatchItem::Target) && isOk(mission::MatchItem::Stealing);
		m_finderController.SetMatchCheck(isFaceVisible, isStealing, isOk(mission::MatchItem::Focus));
	}


	mission::ShotInfo InGameScene::MakeShotInfo(const camera::IPhotoSubject* focusSubject, const bool isInFocus)
	{
		mission::ShotInfo shot;
		shot.isInFocus = isInFocus;

		// 被写体が敵なら、盗んでいるかと、何回目の盗みかを入れる
		// ※ 被写体は const なので、敵の一覧から同じ敵を探して、ステートマシンを見る
		for (auto& enemy : m_enemyList)
		{
			if (enemy.get() != focusSubject) continue;

			shot.subject = enemy.get();

			// 悪党の写り方を測る(撮影者の体はレイで無視)
			shot.measure = mission::MeasureShot(*enemy, m_photoCapture.GetFinderCamera(), m_boy->GetCollisionObject());

			const actor::EnemyStateMachine* stateMachine = enemy->GetEnemyStateMachine();
			if (stateMachine)
			{
				shot.isStealing = stateMachine->IsStealing();
				shot.stealCount = stateMachine->GetStealCount();
			}
			break;
		}
		return shot;
	}


	void InGameScene::DrawGameLoopDebug()
	{
#ifdef APP_ENABLE_PARAMETER_HOTRELOAD
		ImGui::Begin("Game Loop");

		bool isChanged = false;
		const std::vector<mission::MissionData>& missionList = m_missionManager.GetMissionList();
		for (int i = 0; i < static_cast<int>(missionList.size()); ++i)
		{
			const mission::MissionData& mission = missionList[i];
			const char* stateName = "Progress";
			if (mission.state == mission::MissionState::Achievement) stateName = "Achievement";
			else if (mission.state == mission::MissionState::Failed) stateName = "Failed";

			// ボタンの名前が同じだと ImGui が区別できないので、番号を ID にする
			ImGui::PushID(i);
			ImGui::Text("Mission %d : %d / %d  %s", i, mission.currentCount, mission.requiredCount, stateName);
			ImGui::SameLine();
			if (ImGui::Button("+1"))
			{
				isChanged |= m_missionManager.DebugAddCount(i);
			}
			ImGui::PopID();
		}

		// 全部達成にする(次の RequestScene でリザルトへ移る)
		if (ImGui::Button("Achieve all -> Result"))
		{
			m_missionManager.DebugAchieveAll();
			isChanged = true;
		}
		// 今のまま終わらせる(達成していないミッションは失敗。失敗のリザルトを確かめる用)
		if (ImGui::Button("Finish -> Result"))
		{
			m_isFinishRequested = true;
		}

		ImGui::End();

		// 枚数が変わったら、ミッションの UI の文と色を変える(撮って数えた時と同じ)
		if (isChanged)
		{
			UpdateMissionMenu();
		}
#endif
	}


	void InGameScene::UpdateMissionMenu()
	{
		if (m_missionMenu == nullptr) return;

		std::vector<ui::MissionRowData> rowList;
		for (const mission::MissionData& mission : m_missionManager.GetMissionList())
		{
			ui::MissionRowData row;
			// 文の後ろに、今の枚数を付ける(例: (1/3))
			row.text = mission.GetDisplayText()
				+ L" (" + std::to_wstring(mission.currentCount) + L"/" + std::to_wstring(mission.requiredCount) + L")";

			switch (mission.state)
			{
			case mission::MissionState::Achievement: row.tone = ui::MissionRowTone::Achievement; break;
			case mission::MissionState::Failed:      row.tone = ui::MissionRowTone::Failed;      break;
			default:                                 row.tone = ui::MissionRowTone::Progress;    break;
			}
			rowList.push_back(row);
		}
		m_missionMenu->SetMissionRows(rowList);
	}


	void InGameScene::UpdatePhotoSample()
	{
		// 描き終わっていない見本だけ進める(描き終わった物は中で何もしない)
		for (auto& photoSample : m_photoSampleList)
		{
			if (photoSample) photoSample->Update();
		}

		// 出す絵: 進行中で、見本を描き終わった最初のミッション
		Texture* texture = nullptr;
		const std::vector<mission::MissionData>& missionList = m_missionManager.GetMissionList();
		for (int i = 0; i < static_cast<int>(missionList.size()) && i < static_cast<int>(m_photoSampleList.size()); ++i)
		{
			if (missionList[i].state != mission::MissionState::Progress) continue;
			mission::PhotoSampleRenderer* photoSample = m_photoSampleList[i].get();
			if (photoSample == nullptr || !photoSample->IsRendered()) continue;

			texture = &photoSample->GetTexture();
			break;
		}

		// 毎フレーム渡す(同じ絵なら中で作り直さない。ホットリロードで UI が作り直されても出る)
		if (m_sampleMenu)
		{
			m_sampleMenu->SetTexture(texture);
			m_sampleMenu->SetIsFinderActive(m_photoCapture.IsFinderActive());
		}
	}


	void InGameScene::OpenJudge()
	{
		if (m_judgeMenu == nullptr) return;

		const std::vector<mission::ShotRecord>& recordList = m_missionManager.GetShotRecordList();
		if (recordList.empty()) return;
		const mission::ShotRecord& record = recordList.back();

		ui::JudgeViewData viewData;
		// 撮った写真(アルバムに残した物)
		viewData.photo = GameResult::GetInstance()->GetAlbum().GetTexture(record.photoIndex);

		// 比べたミッション(比べていなければ無し)
		const std::vector<mission::MissionData>& missionList = m_missionManager.GetMissionList();
		const bool hasMission = record.missionIndex >= 0 && record.missionIndex < static_cast<int>(missionList.size());

		// チェックリスト: 照合した時だけ、項目ごとに 〇 / × を並べる
		const bool isMatched = record.status == mission::ShotStatus::Counted || record.status == mission::ShotStatus::Failed;
		if (isMatched)
		{
			// 見本が無いミッションは、角度・大きさ・位置を見ていないので出さない
			const bool hasSample = hasMission && missionList[record.missionIndex].hasSample;
			for (int i = 0; i < static_cast<int>(mission::MatchItem::Max); ++i)
			{
				const mission::MatchItem item = static_cast<mission::MatchItem>(i);
				const bool isSampleItem = item == mission::MatchItem::Yaw || item == mission::MatchItem::Pitch
					|| item == mission::MatchItem::Size || item == mission::MatchItem::Position;
				if (isSampleItem && !hasSample) continue;

				ui::JudgeCheckData check;
				check.name = mission::GetMatchItemCheckName(item);
				check.isOk = record.result.isSuccess[i];
				viewData.checkList.push_back(check);
			}
		}

		const std::wstring enemyName = mission::GetEnemyName(record.subjectType);
		switch (record.status)
		{
		case mission::ShotStatus::NoSubject:
			viewData.tone = ui::JudgeTone::Miss;
			viewData.matchRate = 0.0f;
			viewData.detail = L"悪党が写っていない";
			break;

		case mission::ShotStatus::AlreadyCounted:
			viewData.tone = ui::JudgeTone::Neutral;
			viewData.detail = enemyName + L"\n同じ盗みは、もう撮った";
			break;

		case mission::ShotStatus::NoMission:
			viewData.tone = ui::JudgeTone::Neutral;
			viewData.detail = enemyName + L"\nミッションは、もう終わっている";
			break;

		case mission::ShotStatus::Counted:
		{
			viewData.tone = ui::JudgeTone::Match;
			viewData.matchRate = 1.0f;
			viewData.detail = enemyName + L"  ミッションに数えた";
			// 何枚目か(例: 2 / 3)
			if (hasMission)
			{
				const mission::MissionData& missionData = missionList[record.missionIndex];
				viewData.detail += L"  " + std::to_wstring(missionData.currentCount) + L" / " + std::to_wstring(missionData.requiredCount);
			}
			break;
		}

		default:
		{
			// 照合して NG(どこがダメだったかは、チェックリストの × で分かる)
			viewData.tone = ui::JudgeTone::Miss;
			viewData.matchRate = record.result.GetSuccessRate();
			viewData.detail = enemyName + L"  チェックが無い項目を合わせて、もう一度";
			break;
		}
		}

		m_judgeMenu->Show(viewData);
		m_isJudgeOpen = true;
	}


	void InGameScene::UpdateJudge()
	{
		if (m_judgeMenu == nullptr)
		{
			m_isJudgeOpen = false;
			return;
		}

		// 出てくる動き
		m_judgeLayout.Update();

		// ハンコを押し終わったら、A か RT で閉じる(RT は撮影と同じボタンなので、続けて撮りやすい)
		// ※ 前に撮影だった B でも閉じられるように残しておく
		if (!m_judgeMenu->CanClose()) return;
		if (g_pad[0]->IsTrigger(enButtonA) || g_pad[0]->IsTrigger(enButtonRB2) || g_pad[0]->IsTrigger(enButtonB))
		{
			m_isJudgeOpen = false;
		}
	}


	void InGameScene::UpdateTimeLimit()
	{
		// 時間切れの後と、全部達成した後は減らさない(リザルトに移るのを待つ)
		if (m_isTimeUp || m_missionManager.IsAllAchieved()) return;

		m_remainTime -= g_gameTime->GetFrameDeltaTime();
		if (m_remainTime <= 0.0f)
		{
			m_remainTime = 0.0f;
			m_isTimeUp = true;
		}

		// のこり時間を UI に渡す(数字は 1 秒変わった時だけ書きかわる)
		if (m_timerMenu) m_timerMenu->SetRemainTime(m_remainTime);
	}


	void InGameScene::CheckFound()
	{
		if (m_foundEnemy != nullptr) return;

		for (auto& enemy : m_enemyList)
		{
			if (!enemy->IsFound()) continue;

			// 最初に見つけた悪党だけを見る(ほかの悪党が後で見つけても、演出は 1 回)
			m_foundEnemy = enemy.get();
			if (m_foundMenu) m_foundMenu->ShowMark();
			return;
		}
	}


	void InGameScene::UpdateFound()
	{
		// 悪党と一般客は動かし続ける(悪党が主人公の方へ走ってくる)
		// ※ 主人公は動かさない(入力でにげられないように)。時間も減らさない
		// ※ カメラも動かさない(更新を呼ぶと、スティックで視点が回ってしまう)。見つかった瞬間の視点のまま止める
		for (auto& enemy : m_enemyList)
		{
			enemy->Update();
		}
		m_visitorManager->Update();
		m_lightController.Update();

		if (m_foundMenu)
		{
			// 「！」を悪党の頭の上に出す(頭のボーンが無ければ、ピントの位置を使う)
			Vector3 headPosition;
			Vector3 headForward;
			if (!m_foundEnemy->GetHead(headPosition, headForward))
			{
				headPosition = m_foundEnemy->GetFocusPosition();
			}
			headPosition.y += FOUND_MARK_HEIGHT;
			Vector2 screenPosition;
			const bool isVisible = util::CalcScreenPosition(*g_camera3D, headPosition, screenPosition);
			m_foundMenu->SetMarkPosition(screenPosition, isVisible);

			// 主人公のところまで走り終わったら、画面をうっすら暗くして「見つかってしまった...」を出す
			if (!m_foundMenu->IsGameOverStarted() && m_foundEnemy->IsCaught())
			{
				m_foundMenu->StartGameOver();
			}
		}
		m_foundLayout.Update();
	}


	void InGameScene::OpenPause()
	{
		m_isPauseOpen = true;
		// 開くたびに「つづける」から選ぶ(すぐに戻れるように)
		m_pauseSelectIndex = static_cast<int>(ui::PauseMenu::SelectType::Resume);
		if (m_pauseMenu) m_pauseMenu->SetSelect(ui::PauseMenu::SelectType::Resume);
	}


	void InGameScene::UpdatePause()
	{
		using SelectType = ui::PauseMenu::SelectType;
		constexpr int itemCount = static_cast<int>(SelectType::Max);

		// B か START で閉じる(ゲームに戻る)
		if (g_pad[0]->IsTrigger(enButtonB) || g_pad[0]->IsTrigger(enButtonStart))
		{
			m_isPauseOpen = false;
			return;
		}

		// 上と下の入力で選ぶ項目を変える(端まで行ったら反対側に戻る)
		const int prevIndex = m_pauseSelectIndex;
		if (g_pad[0]->IsTrigger(enButtonDown)) m_pauseSelectIndex = (m_pauseSelectIndex + 1) % itemCount;
		if (g_pad[0]->IsTrigger(enButtonUp))   m_pauseSelectIndex = (m_pauseSelectIndex + itemCount - 1) % itemCount;

		// 変わった時だけ、View に選んでいる項目を伝える
		const SelectType selectType = static_cast<SelectType>(m_pauseSelectIndex);
		if (m_pauseSelectIndex != prevIndex && m_pauseMenu)
		{
			m_pauseMenu->SetSelect(selectType);
		}

		// A で決める
		if (!g_pad[0]->IsTrigger(enButtonA)) return;
		// 決めたボタンを押した形にする(見た目は View が変える)
		if (m_pauseMenu) m_pauseMenu->PressSelect();
		switch (selectType)
		{
		case SelectType::Resume: m_isPauseOpen = false;     break;
		case SelectType::Sound:  m_soundOption.Open();      break;
		case SelectType::Title:  m_isTitleRequested = true; break;
		default: break;
		}
	}
}
