#include "stdafx.h"
#include "Game.h"


namespace {
	/** 移動速度 */
	constexpr float MOVE_SPEED = 1.0f;
	/** 回転速度 */
	constexpr float ROTATE_SPEED = 5.0f;
	/** 拡縮速度 */
	constexpr float SCALE_SPEED = 0.01f;
	/** 初期位置 */
	const Vector3 INITIALIZE_POSITION = { -300.0f,0.0f,0.0f };
	/** 初期スケール */
	const Vector3 INITIALIZE_SCALE = { 2.0f,2.0f,2.0f };
}


Game::Game()
	: spriteRender_(nullptr)
	, position_(Vector3::Zero)
	, rotation_(Quaternion::Identity)
	, scale_(Vector3::One)
{}


Game::~Game()
{
	if (spriteRender_ != nullptr)
	{
		delete spriteRender_;
		spriteRender_ = nullptr;
	}
}


bool Game::Start()
{
	// Step1-1完成。(Step1-2にてSpriteRenderを自作)
	//spriteInitData_.m_fxFilePath = "Assets/shader/sprite.fx";
	//spriteInitData_.m_ddsFilePath[0] = "Assets/texture/texture.dds";
	//spriteInitData_.m_width = 128;
	//spriteInitData_.m_height = 128;
	//sprite_.Init(spriteInitData_);

	// Step1-3完成。
	// tkmファイルパスを設定
	modelInitData_.m_tkmFilePath = "Assets/modelData/unityChan.tkm";
	// fxファイブパスを設定
	modelInitData_.m_fxFilePath = "Assets/shader/model.fx";
	// モデルの初期化
	model_.Init(modelInitData_);

	//spriteRender_ = new SpriteRender;
	//spriteRender_->Init("Assets/shader/sprite.fx", "Assets/texture/texture.dds", 128, 128);
	//spriteRender_->SetPosition(INITIALIZE_POSITION);
	//spriteRender_->SetRotation(rotation_);
	//spriteRender_->SetScale(INITIALIZE_SCALE);
	//spriteRender_->Update();

	return true;
}


void Game::Update()
{
	// 位置と回転と拡縮を適当に更新してみた。
	//position_ = spriteRender_->GetPosition();
	//rotation_ = spriteRender_->GetRotation();
	//scale_ = spriteRender_->GetScale();
	//
	//position_.x += MOVE_SPEED;
	//rotation_.AddRotationDegZ(ROTATE_SPEED);
	//scale_.x += SCALE_SPEED;
	//scale_.y += SCALE_SPEED;
	//
	//spriteRender_->SetTRS(position_, rotation_, scale_);
	//spriteRender_->Update();
}

void Game::Render(RenderContext& rc)
{
	//sprite_.Draw(rc);

	// スプライト描画
	//spriteRender_->Draw(rc);

	// デバックログ出力。
	K2_LOG("TEST:\n");

	// モデル描画。
	model_.Draw(rc);
}