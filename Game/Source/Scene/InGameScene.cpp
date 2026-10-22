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
#include "Enemy.h"
#include "Source/Actor/Character/Boy/Boy.h"
#include "Source/UI/SoundOptionMenu/SoundOptionMenu.h"
#include "Source/Sound/SoundManager.h"


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
	}


	InGameScene::InGameScene()
		: m_spriteRender(nullptr)
		, m_position(INITIALIZE_POSITION)
		, m_gPosition(Vector3::Zero)
		, m_gRotation(Quaternion::Identity)
		, m_gScale(Vector3::One)
		, m_gameCamera(nullptr)
		, m_groundModelRender(nullptr)
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
		if (m_groundModelRender != nullptr)
		{
			delete m_groundModelRender;
			m_groundModelRender = nullptr;
		}

		// ファインダーコントローラーから敵の登録を解除
		m_finderController.UnregisterSubject(m_enemy.get());
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

		// 地面のモデルレンダーを初期化
		m_groundModelRender = new ModelRender;
		m_groundModelRender->Init("Assets/modelData/ground.tkm", nullptr, enModelUpAxisZ, 0, false, true);
		m_groundModelRender->SetTRS(m_gPosition, m_gRotation, m_gScale);
		m_groundModelRender->Update();

		// 地面の形から当たり判定を作る(ワールド行列が決まってから作る)
		Model& groundModel = m_groundModelRender->GetModel();
		m_groundPhysicsObject.CreateFromModel(groundModel, groundModel.GetWorldMatrix());

		// 主人公を作る(位置を決めてから Start を呼ぶ)
		m_boy = std::make_unique<app::actor::Boy>();
		m_boy->Start();

		// 写真撮影クラスの初期化
		m_photoCapture.Init();
		// RenderingEngineにPhotoCaptureを登録
		RenderingEngine::GetInstance().SetPhotoCapture(&m_photoCapture);
		// ファインダーの操作(覗く・ピント・ズーム・撮影)を FinderController に任せる
		m_finderController.Initialize(&m_photoCapture);

		// 敵を作成
		m_enemy = std::make_unique<app::Enemy>();
		m_enemy->Init();
		// ファインダーコントローラーに敵を登録する
		m_finderController.RegisterSubject(m_enemy.get());
		m_enemy->Update();

		// 後々に響きそうなのでゲームカメラを作成しておく。
		m_gameCamera = new app::camera::GameCamera;
		m_gameCamera->Start();
		// カメラは主人公を追いかけて、主人公はカメラの向きを基準に動く
		m_gameCamera->SetTarget(&m_boy->GetTransform());
		m_boy->SetGameCamera(m_gameCamera);
		m_gameCamera->Update();
		return true;
	}
	
	
	void InGameScene::Update()
	{
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
		// ※ 撮影できたフレーム true が返る(クールタイム中は撮れない)
		const bool isShot = m_finderController.Update();

		// 主人公の更新(ファインダーを覗いている間は、撮影ステートになる)
		// ※ カメラが主人公の新しい位置を追いかけるように、カメラより先に更新する
		m_boy->SetIsFinderActive(m_photoCapture.IsFinderActive());
		m_boy->Update();

		// ファインダーを覗いている間だけ、1人称にする
		m_gameCamera->SetFirstPeerson(m_photoCapture.IsFinderActive());
		m_gameCamera->Update();

		// ファインダー視点のカメラをゲームカメラの位置と向きに合わせる
		Camera& finderCamera = m_photoCapture.GetFinderCamera();
		finderCamera.SetPosition(g_camera3D->GetPosition());
		finderCamera.SetTarget(g_camera3D->GetTarget());
		finderCamera.Update();

		// 敵の位置更新
		Quaternion enemyRot;
		enemyRot.SetRotationDegY(180.0f);
		m_enemy->SetRotation(enemyRot);
		m_enemy->Update();


		/** ファインダーのオートフォーカス */

		// オートフォーカスとファインダーの UI
		// ※ ファインダーのカメラと敵の位置が決まった後に呼ぶ(顔の画面の位置を計算するため)
		m_finderController.LateUpdate();

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

		// モデル描画(地面)
		m_groundModelRender->Draw(rc);

		// ファインダーの UI(構えていない時は、何も描画されない)
		m_finderController.Render(rc);

		// サウンドオプションは毎フレーム描画を呼ぶ(閉じているときは、メニューの中で何も描画されない)
		// ※ IsTrigger の中で呼ぶと、ボタンを押した1フレームしか描画されない
		// ※ ファインダーの UI より後に描画して、手前に出す
		m_soundOptionLayout.Render(rc);

		// 敵の描画
		m_enemy->Render(rc);
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
}