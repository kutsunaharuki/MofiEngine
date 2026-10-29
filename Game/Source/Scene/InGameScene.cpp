/**
 * @file InGameScene.cpp
 * @brief インゲームシーン
 */
#include "stdafx.h"
#include "SceneManager.h"
#include "InGameScene.h"
#include "ResultScene.h"
#include "Source/Camera/GameCamera.h"
#include "imgui.h"
#include "Source/Actor/Character/Enemy/EnemyBase.h"
#include "Source/Actor/Character/Enemy/EnemyStateMachine.h"
#include "Source/Actor/Character/Boy/Boy.h"
#include "Source/UI/SoundOptionMenu/SoundOptionMenu.h"
#include "Source/Sound/SoundManager.h"
#include "Source/Stage/Stage.h"
#include "Source/Actor/StageObject/StageObject.h"
#include "Source/Stage/StageEditor.h"
#include "Source/Util/ParameterManager.h"
#include "Source/AI/Navigation/NavGrid.h"
#include "Source/AI/Navigation/Pathfinder.h"
#include "Source/AI/Navigation/PathFollower.h"


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
		/** ステージの配置(エディタで保存する JSON) */
		constexpr const char* STAGE_FILE_PATH = "Assets/stage/Museum.json";
		/** ステージの最初の配置(3ds Max で作った tkl。JSON が無い時に使う) */
		constexpr const char* STAGE_TKL_PATH = "Assets/modelData/Stage/Museum.tkl";
		/** ステージのモデルのフォルダ */
		constexpr const char* STAGE_MODEL_DIRECTORY = "Assets/modelData/Stage/";

		/** 経路探索のミニマップで、1マスのを何ピクセルで描くか */
		constexpr float NAV_DEBUG_PIXEL_PER_CELL = 6.0f;
		/** マスにカーソルを当てた時に塞いでいる置物を何個まで出すか */
		constexpr int NAV_DEBUG_BLOCKER_MAX = 10;
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
		// サウンドオプションのUIを作る(最初は閉じている)
		m_soundOptionLayout.Initialize<ui::SoundOptionMenu>(SOUND_OPTION_LAYOUT_PATH);
		m_soundOptionMenu = m_soundOptionLayout.GetMenu<ui::SoundOptionMenu>();

		// タイトルのBGMを鳴らす
		sound::SoundManager::GetInstance()->Play(sound::EnSoundType::BGM, sound::bgm::TITLE);

		// ステージの置物を作る。エディタで保存した JSON が無ければ、3ds Max の tkl から作る
		m_stage = std::make_unique<stage::Stage>();
		if (!m_stage->Load(STAGE_FILE_PATH))
		{
			m_stage->ImportTkl(STAGE_TKL_PATH, STAGE_MODEL_DIRECTORY);
		}

		// 経路探索の地図を作る
		m_navGrid = std::make_unique<ai::NavGrid>();
		m_navGrid->Build(*m_stage);

		// 経路探索の地図ができてから作る
		m_pathfinder = std::make_unique<ai::Pathfinder>(*m_navGrid);

		// 敵の一覧に大男を追加する
		m_enemyList.push_back(std::make_unique<actor::EnemyBase>(actor::EnemyBase::EnemyType::BigMan));
		// 敵の一覧にねずみ男を追加する
		//m_enemyList.push_back(std::make_unique<actor::EnemyBase>(actor::EnemyBase::EnemyType::RatMan));

		// 敵の一覧の敵に経路探索の地図を設定して
		for (auto& enemy : m_enemyList)
		{
			enemy->SetNavigation(m_pathfinder.get(), m_navGrid.get());
			enemy->Start();
		}


#ifdef APP_ENABLE_PARAMETER_HOTRELOAD
		// ステージエディタは Debug の時だけ
		m_stageEditor = std::make_unique<stage::StageEditor>(m_stage.get(), STAGE_FILE_PATH, STAGE_TKL_PATH, STAGE_MODEL_DIRECTORY);
#endif

		// 主人公を作る
		m_boy = std::make_unique<app::actor::Boy>();
		m_boy->Start();

		// 写真撮影クラスの初期化
		m_photoCapture.Init();

		// RenderingEngineにPhotoCaptureを登録
		RenderingEngine::GetInstance().SetPhotoCapture(&m_photoCapture);
		// ファインダーの操作(覗く・ピント・ズーム・撮影)を FinderController に任せる
		m_finderController.Initialize(&m_photoCapture);

		// 大男を作成
		//m_enemy = std::make_unique<app::actor::EnemyBase>(app::actor::EnemyBase::EnemyType::BigMan);
		//m_enemy->Start();


		// ファインダーコントローラーに敵を登録する
		for (auto& enemy : m_enemyList)
		{
			m_finderController.RegisterSubject(enemy.get());
		}
		
		// 動作確認用
		const Vector3& boyPosition = m_boy->GetTransform().GetWorldPosition();
		for (auto& enemy : m_enemyList)
		{
			enemy->GetEnemyStateMachine()->MoveTo(boyPosition);
			enemy->Update();
		}

		// 後々に響きそうなのでゲームカメラを作成しておく。
		m_gameCamera = std::make_unique<app::camera::GameCamera>();
		m_gameCamera->Start();

		// カメラは主人公を追いかけて、主人公はカメラの向きを基準に動く
		m_gameCamera->SetTarget(&m_boy->GetTransform());
		m_boy->SetGameCamera(m_gameCamera.get());
		m_gameCamera->Update();
		return true;
	}
	
	
	void InGameScene::Update()
	{
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

		// デバック表示
		DrawNavigationDebug();

		// サウンドオプションの更新(開いているときだけ、中で入力を受け付ける)
		m_soundOptionLayout.Update();

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
		}

		// ファインダーを覗いている間だけ、1人称にする
		m_gameCamera->SetFirstPeerson(m_photoCapture.IsFinderActive());
		m_gameCamera->Update();

		// ファインダー視点のカメラをゲームカメラの位置と向きに合わせる
		Camera& finderCamera = m_photoCapture.GetFinderCamera();
		finderCamera.SetPosition(g_camera3D->GetPosition());
		finderCamera.SetTarget(g_camera3D->GetTarget());
		finderCamera.Update();

		// 確認用:右へ歩かせる
		//m_enemy->GetEnemyStateMachine()->SetMoveDirection(Vector3(1.0f, 0.0f, 0.0f));
		//m_enemy->GetEnemyStateMachine()->SetMoveAmount(1.0f);

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

		if (g_pad[0]->IsTrigger(enButtonA))
		{
			id = app::ResultScene::ID();
			waitTime = 1.0f;
			return true;
		}
		return false;
	}


	void InGameScene::DrawNavigationDebug()
	{
#ifdef APP_ENABLE_PARAMETER_HOTRELOAD

		if (m_navGrid == nullptr) return;

		ImGui::Begin("Navigation");

		/** 1. 操作ボタン */
		if (ImGui::Button("Move enemies to Boy"))
		{
			// 敵を主人公の今の位置へ向かわせる
			const Vector3& boyPosition = m_boy->GetTransform().GetWorldPosition();
			m_navDebugMessage.clear();
			for (int i = 0; i < static_cast<int>(m_enemyList.size()); ++i)
			{
				const bool isFound = m_enemyList[i]->GetEnemyStateMachine()->MoveTo(boyPosition);
				m_navDebugMessage += "Enemy" + std::to_string(i) + (isFound ? ": path found " : ": NO PATH");
			}
		}

		ImGui::SameLine();
		if (ImGui::Button("Stop"))
		{
			for (auto& enemy : m_enemyList)
			{
				enemy->GetEnemyStateMachine()->StopPathFollow();
			}

			m_navDebugMessage = "Stopped.";
		}
		ImGui::SameLine();

		// エディタで置物を動かした後に押す(地図が古いままだと、敵が棚に突っ込む)
		if (ImGui::Button("Rebuild NavGrid"))
		{
			m_navGrid->Build(*m_stage);
			m_navDebugMessage = "Rebuilt.";
		}

		if (!m_navDebugMessage.empty())
		{
			ImGui::TextUnformatted("NavGrid is not built");
		}

		if (!m_navGrid->IsBuilt())
		{
			ImGui::TextDisabled("NavGrid is not built.");
			ImGui::End();
			return;
		}

		const int width = m_navGrid->GetWidth();
		const int height = m_navGrid->GetHeight();
		ImGui::Text("Cells: %d x %d", width, height);

		// 敵ごとの状態(道順の点の数・今何番目か・着いたか)
		for (int i = 0; i < static_cast<int>(m_enemyList.size()); ++i)
		{
			const ai::PathFollower& follower = m_enemyList[i]->GetEnemyStateMachine()->GetPathFollower();
			const Vector3& position = m_enemyList[i]->GetTransform().GetWorldPosition();
			ImGui::Text("Enemy%d  pos(%.0f, %.0f, %.0f)  path %d  index %d  arrived %s  stuck %s",
				i, position.x, position.y, position.z,
				static_cast<int>(follower.GetPath().size()), follower.GetIndex(),
				follower.IsArrived() ? "yes" : "no",
				follower.IsStuck() ? "yes" : "no");
		}

		const Vector3& boyWorld = m_boy->GetTransform().GetWorldPosition();
		ImGui::Text("Boy  pos(%.0f, %.0f, %.0f)", boyWorld.x, boyWorld.y, boyWorld.z);

		// 地図を作った時の高さの帯(この範囲に入る置物が塞ぐ)。主人公の y(床の高さ)とずれていないか確認する
		const ai::NavGridSetting& debugSetting = m_navGrid->GetSetting();
		ImGui::Text("Block band  y[%.0f ~ %.0f]  (floorHeight %.0f)",
			debugSetting.floorHeight + debugSetting.stepHeight,
			debugSetting.floorHeight + debugSetting.agentHeight,
			debugSetting.floorHeight);


		/** 2. 地図(赤 = 通れない、灰 = 通れる) */
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		const ImVec2 origin = ImGui::GetCursorScreenPos();
		const float pixel = NAV_DEBUG_PIXEL_PER_CELL;

		// マス番号から、画面上のマスの左上の位置を求める
		// ※ 画面は下に行くほど Y が増えるので、奥(z)を上にするため反転する
		auto toScreen = [&](const int x, const int z)
			{
				return ImVec2(origin.x + x * pixel, origin.y + (height - 1 - z) * pixel);
			};

		for (int z = 0; z < height; ++z)
		{
			for (int x = 0; x < width; ++x)
			{
				const ImVec2 topLeft = toScreen(x, z);
				const ImU32 color = m_navGrid->IsBlocked(x, z)
					? IM_COL32(200, 60, 60, 255)
					: IM_COL32(60, 60, 60, 255);
				drawList->AddRectFilled(topLeft, ImVec2(topLeft.x + pixel, topLeft.y + pixel), color);
			}
		}

		// 座標から、画面上のマスの中心の位置を求める(地図の外なら false)
		auto worldToScreen = [&](const Vector3& position, ImVec2& outScreen)
			{
				int x = 0;
				int z = 0;
				if (!m_navGrid->WorldToCell(position, x, z)) return false;
				const ImVec2 topLeft = toScreen(x, z);
				outScreen = ImVec2(topLeft.x + pixel * 0.5f, topLeft.y + pixel * 0.5f);
				return true;
			};


		/** 3. 敵の道順(黄色の線)と、敵(水色)・主人公(緑)の位置 */
		for (auto& enemy : m_enemyList)
		{
			const ai::PathFollower& follower = enemy->GetEnemyStateMachine()->GetPathFollower();
			const std::vector<Vector3>& path = follower.GetPath();

			// 敵の今の位置から、今目指している点までと、その先の点同士を線でつなぐ
			ImVec2 previous;
			bool hasPrevious = worldToScreen(enemy->GetTransform().GetWorldPosition(), previous);
			for (int i = follower.GetIndex(); i < static_cast<int>(path.size()); ++i)
			{
				ImVec2 current;
				if (!worldToScreen(path[i], current)) continue;
				if (hasPrevious)
				{
					drawList->AddLine(previous, current, IM_COL32(255, 220, 0, 255), 2.0f);
				}
				previous = current;
				hasPrevious = true;
			}

			ImVec2 enemyScreen;
			if (worldToScreen(enemy->GetTransform().GetWorldPosition(), enemyScreen))
			{
				drawList->AddCircleFilled(enemyScreen, pixel, IM_COL32(80, 200, 255, 255));
			}
		}

		ImVec2 boyScreen;
		if (worldToScreen(m_boy->GetTransform().GetWorldPosition(), boyScreen))
		{
			drawList->AddCircleFilled(boyScreen, pixel, IM_COL32(80, 255, 120, 255));
		}

		// 描いた分だけ場所を空ける(ウィンドウの大きさとスクロールに使われる)
		const ImVec2 mapSize(width* pixel, height* pixel);
		ImGui::Dummy(mapSize);

		/** 4. マスにカーソルを当てると、そのマスを塞いでいる置物を出す */
		// ※ NavGrid::Build と同じ条件(高さの帯 + 敵の半径ぶん太らせた箱)で探す
		if (ImGui::IsItemHovered())
		{
			const ImVec2 mouse = ImGui::GetIO().MousePos;
			const int cellX = static_cast<int>(std::floor((mouse.x - origin.x) / pixel));
			const int cellZ = height - 1 - static_cast<int>(std::floor((mouse.y - origin.y) / pixel));

			if (m_navGrid->IsInside(cellX, cellZ))
			{
				const ai::NavGridSetting& setting = m_navGrid->GetSetting();
				const Vector3 center = m_navGrid->CellToWorld(cellX, cellZ);
				const float half = setting.cellSize * 0.5f;
				const float bandMin = setting.floorHeight + setting.stepHeight;
				const float bandMax = setting.floorHeight + setting.agentHeight;

				ImGui::BeginTooltip();
				ImGui::Text("Cell (%d, %d)  world (%.0f, %.0f)  %s",
					cellX, cellZ, center.x, center.z,
					m_navGrid->IsBlocked(cellX, cellZ) ? "BLOCKED" : "open");

				int blockerCount = 0;
				for (int i = 0; i < m_stage->GetObjectCount(); ++i)
				{
					const actor::StageObject* object = m_stage->GetStageObject(i);
					if (object == nullptr || !object->IsCollision()) continue;

					const Vector3& boxMin = object->GetWorldAABBMin();
					const Vector3& boxMax = object->GetWorldAABBMax();

					// 体の高さに重なっていなければ、塞いでいない
					if (boxMax.y <= bandMin || boxMin.y >= bandMax) continue;

					// 太らせた箱と、マスが重なっているか
					const bool isOverlapX = (center.x + half >= boxMin.x - setting.agentRadius) && (center.x - half <= boxMax.x + setting.agentRadius);
					const bool isOverlapZ = (center.z + half >= boxMin.z - setting.agentRadius) && (center.z - half <= boxMax.z + setting.agentRadius);
					if (!isOverlapX || !isOverlapZ) continue;

					// 名前、高さの範囲、箱の大きさ(大きすぎる物は、部屋ごと塞いでいる犯人の可能性が高い)
					if (blockerCount < NAV_DEBUG_BLOCKER_MAX)
					{
						ImGui::Text("%d: %s  y[%.0f ~ %.0f]  size %.0f x %.0f",
							i, object->GetName().c_str(), boxMin.y, boxMax.y,
							boxMax.x - boxMin.x, boxMax.z - boxMin.z);
					}
					++blockerCount;
				}
				if (blockerCount > NAV_DEBUG_BLOCKER_MAX)
				{
					ImGui::Text("... and %d more", blockerCount - NAV_DEBUG_BLOCKER_MAX);
				}
				ImGui::EndTooltip();
			}
		}
		ImGui::End();
#endif
	}
}