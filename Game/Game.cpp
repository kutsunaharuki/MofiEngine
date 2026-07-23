#include "stdafx.h"
#include "Game.h"
#include "GameCamera.h"


namespace {
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
}


Game::Game()
	: m_spriteRender(nullptr)
	, m_modelRender(nullptr)
	, m_position(INITIALIZE_POSITION)
	, m_rotation(Quaternion::Identity)
	, m_scale(Vector3::One)
	, m_gPosition(Vector3::Zero)
	, m_gRotation(Quaternion::Identity)
	, m_gScale(Vector3::One)
	, m_gameCamera(nullptr)
	, m_groundModelRender(nullptr)
{}


Game::~Game()
{
	if (m_spriteRender != nullptr)
	{
		delete m_spriteRender;
		m_spriteRender = nullptr;
	}
	if (m_modelRender != nullptr)
	{
		delete m_modelRender;
		m_modelRender = nullptr;
	}
	if (m_gameCamera != nullptr)
	{
		delete m_gameCamera;
		m_gameCamera = nullptr;
	}
}


bool Game::Start()
{
	// Step1-1完成(Step1-2にてSpriteRenderを自作)
	//m_spriteInitData.m_fxFilePath = "Assets/shader/sprite.fx";
	//m_spriteInitData.m_ddsFilePath[0] = "Assets/texture/texture.dds";
	//m_spriteInitData.m_width = 128;
	//m_spriteInitData.m_height = 128;
	//m_sprite.Init(m_spriteInitData);

	// Step1-2完成
	//spriteRender_ = new nsK2Engine::SpriteRender;
	//spriteRender_->Init("Assets/shader/sprite.fx", "Assets/texture/texture.dds", 128.0f, 128.0f);
	//spriteRender_->SetPosition(INITIALIZE_POSITION);
	//spriteRender_->SetRotation(rotation_);
	//spriteRender_->SetScale(INITIALIZE_SCALE);
	//spriteRender_->Update();

	// Step1-3完成
	// tkmファイルパスを設定
	//modelInitData_.m_tkmFilePath = "Assets/modelData/unityChan.tkm";
	// fxファイブパスを設定
	//modelInitData_.m_fxFilePath = "Assets/shader/model.fx";
	// モデルの初期化
	//model_.Init(modelInitData_);

	// Step1-4完成
	// 第3引数 が影を落とすかどうかのフラグで 第4引数 が影を受けるかどうかのフラグ。
	m_modelRender = new ModelRender;
	m_modelRender->Init("Assets/modelData/unityChan.tkm", true, false);
	m_modelRender->SetTRS(m_position, m_rotation, m_scale);
	m_modelRender->Update();

	// 地面のモデルレンダーを初期化
	m_groundModelRender = new ModelRender;
	m_groundModelRender->Init("Assets/modelData/ground.tkm", false, true);
	m_groundModelRender->SetTRS(m_gPosition, m_gRotation, m_gScale);
	m_groundModelRender->Update();

	// 後々に響きそうなのでゲームカメラを作成しておく。
	m_gameCamera = new app::camera::GameCamera;
	m_gameCamera->Start();
	m_gameCamera->SetTarget(m_modelRender);
	m_gameCamera->Update();

	return true;
}


void Game::Update()
{
	/** ModelRender */
	// 位置、回転、スケールを適当に動かしてみた。
	//position_.x += MOVE_SPEED;
	//m_modelRender->SetPosition(m_position);

	//if (g_pad[0]->IsPress(enButtonA))
	//{
	//	m_position.x += MOVE_SPEED;
	//}

	//if (g_pad[0]->IsPress(enButtonB))
	//{
	//	m_position.x -= MOVE_SPEED;
	//}

	m_rotation.SetRotationDegY(180.0f);
	//m_rotation.AddRotationDegY(ROTATE_SPEED);
	//m_gRotation.AddRotationDegY(ROTATE_SPEED);
	//m_rotation.SetRotationDegY(180.0f);
	m_modelRender->SetRotation(m_rotation);
	//m_modelRender->SetPosition(m_position);
	
	//m_groundModelRender->SetRotation(m_gRotation);

	//bool isPressA = g_pad[0]->IsPress(enButtonA);
	//bool isPressB = g_pad[0]->IsPress(enButtonB);
	//if (isPressA) scale_ += Vector3::One * SCALE_SPEED;
	//if (isPressB) scale_ -= Vector3::One * SCALE_SPEED;
	//modelRender_->SetScale(scale_);
	m_modelRender->Update();

	m_gameCamera->Update();

	/** SpriteRender */
	// 位置と回転と拡縮を適当に更新してみた。
	//position_ = spriteRender_->GetPosition();
	//rotation_ = spriteRender_->GetRotation();
	//scale_ = spriteRender_->GetScale();
	
	//position_.x += MOVE_SPEED;
	//rotation_.AddRotationDegZ(ROTATE_SPEED);
	//scale_.x += SCALE_SPEED;
	//scale_.y += SCALE_SPEED;
	
	//spriteRender_->SetTRS(position_, rotation_, scale_);
	//spriteRender_->Update();
}

void Game::Render(RenderContext& rc)
{
	//m_sprite.Draw(rc);

	// スプライト描画
	//spriteRender_->Draw(rc);

	// デバックログ出力。
	//K2_LOG("TEST:\n");

	//model_.Draw(rc);

	// モデル描画(ユニティちゃん)
	m_modelRender->Draw(rc);
	// モデル描画(地面)
	m_groundModelRender->Draw(rc);
}