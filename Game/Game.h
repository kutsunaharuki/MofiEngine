#pragma once
#include "SpriteRender.h"
#include "ModelRender.h"


// The starting point of your game.
// This is a plain IGameObject running on K2EngineLow only.
// Right now it draws nothing, so you will just see the gray clear color.
// Add your own rendering (a triangle, a sprite, a model...) step by step.


class Game : public IGameObject
{
public:
	Game();
	~Game();
	bool Start() override;
	void Update()override;
	void Render(RenderContext& rc)override;


private:
	/** スプライトレンダー */
	nsK2Engine::SpriteRender* m_spriteRender;
	/** モデルレンダー */
	nsK2Engine::ModelRender* m_modelRender;
	/** 位置 */
	Vector3 m_position;
	/** 回転 */
	Quaternion m_rotation;
	/** スケール */
	Vector3 m_scale;

	/** モデル */
	//Model model_;
	/** モデル初期化データ */
	//ModelInitData modelInitData_;
	/** スプライト初期化データ */
	//SpriteInitData spriteInitData_;
	/** スプライト */
	//Sprite sprite_;
};
