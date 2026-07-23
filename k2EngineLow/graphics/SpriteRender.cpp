/**
 * @file SpriteRender.cpp
 * @brief スプライトレンダークラスの実装
 */
#include "k2EngineLowPreCompile.h"
#include "SpriteRender.h"


namespace nsK2EngineLow
{
	SpriteRender::SpriteRender()
		: m_spriteInitData()
		, m_sprite()
		, m_position(Vector3::Zero)
		, m_rotation(Quaternion::Identity)
		, m_scale(Vector3::One)
	{}


	SpriteRender::~SpriteRender()
	{}


	void SpriteRender::Init(const char* fxFilePath, const char* ddsFilePath, float height, float width)
	{
		m_spriteInitData.m_fxFilePath = fxFilePath;
		m_spriteInitData.m_ddsFilePath[0] = ddsFilePath;
		m_spriteInitData.m_width = width;
		m_spriteInitData.m_height = height;

		m_sprite.Init(m_spriteInitData);
	}


	void SpriteRender::Update()
	{
		m_sprite.Update(m_position, m_rotation, m_scale);
	}


	void SpriteRender::Draw(RenderContext& rc)
	{
		m_sprite.Draw(rc);
	}
}