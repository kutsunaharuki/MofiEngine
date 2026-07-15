#include "stdafx.h"
#include "SpriteRender.h"

SpriteRender::SpriteRender()
	: spriteInitData_()
	, sprite_()
	, position_(Vector3::Zero)
	, rotation_(Quaternion::Identity)
	, scale_(Vector3::One)
{}


SpriteRender::~SpriteRender()
{}


void SpriteRender::Init(const char* fxFilePath, const char* ddsFilePath, int height, int width)
{
	spriteInitData_.m_fxFilePath = fxFilePath;
	spriteInitData_.m_ddsFilePath[0] = ddsFilePath;
	spriteInitData_.m_width = width;
	spriteInitData_.m_height = height;

	sprite_.Init(spriteInitData_);
}


void SpriteRender::Update()
{
	sprite_.Update(position_, rotation_, scale_);
}


void SpriteRender::Draw(RenderContext& rc)
{
	sprite_.Draw(rc);
}