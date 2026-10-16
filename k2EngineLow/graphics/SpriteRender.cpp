/**
 * @file SpriteRender.cpp
 * @brief スプライトレンダークラスの実装
 */
#include "k2EngineLowPreCompile.h"
#include "SpriteRender.h"


namespace nsK2EngineLow
{
	namespace
	{
		/** 既定のスプライト用シェーダー */
		constexpr const char* DEFAULT_SPRITE_FX_PATH = "Assets/shader/sprite.fx";
	}


	SpriteRender::SpriteRender()
		: m_spriteInitData()
		, m_sprite()
		, m_position(Vector3::Zero)
		, m_rotation(Quaternion::Identity)
		, m_scale(Vector3::One)
		, m_pivot(0.5f, 0.5f)
		, m_mulColor(Vector4::White)
	{}


	SpriteRender::~SpriteRender()
	{}


	void SpriteRender::Init(const char* fxFilePath, const char* ddsFilePath, float width, float height)
	{
		// fxファイルが指定されていなければ、既定のスプライト用シェーダーを使う
		m_spriteInitData.m_fxFilePath = (fxFilePath != nullptr) ? fxFilePath : DEFAULT_SPRITE_FX_PATH;
		m_spriteInitData.m_ddsFilePath[0] = ddsFilePath;
		m_spriteInitData.m_width = width;
		m_spriteInitData.m_height = height;

		m_sprite.Init(m_spriteInitData);
	}


	void SpriteRender::Init(const char* ddsFilePath, float width, float height)
	{
		Init(nullptr, ddsFilePath, width, height);
	}


	void SpriteRender::Update()
	{
		// 乗算カラーは、Sprite の初期化時にリセットされるため、更新のたびに設定する
		m_sprite.SetMulColor(m_mulColor);
		m_sprite.Update(m_position, m_rotation, m_scale, m_pivot);
	}


	void SpriteRender::Draw(RenderContext& rc)
	{
		// 描画を RenderingEngine に任せる(ModelRender::Draw と同じ形)
		// ※ コピーパスの後の2Dパスで描画される
		RenderingEngine::GetInstance().AddSpriteRender(*this);
	}


	void SpriteRender::OnRender2D(RenderContext& rc)
	{
		m_sprite.Draw(rc);
	}
}