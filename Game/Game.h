#pragma once


// The starting point of your game.
// This is a plain IGameObject running on K2EngineLow only.
// Right now it draws nothing, so you will just see the gray clear color.
// Add your own rendering (a triangle, a sprite, a model...) step by step.

namespace app
{
	namespace camera
	{
		class GameCamera;
	}
}

class Game : public IGameObject
{
public:
	Game();
	~Game();
	bool Start() override;
	void Update()override;
	void Render(RenderContext& rc)override;


private:
	/** ゲームカメラ */
	app::camera::GameCamera* m_gameCamera;
	/** スプライトレンダー */
	SpriteRender* m_spriteRender;
	/** モデルレンダー */
	ModelRender* m_modelRender;
	/** 地面のモデルレンダー */
	ModelRender* m_groundModelRender;
	/** 位置 */
	Vector3 m_position;
	/** 回転 */
	Quaternion m_rotation;
	/** スケール */
	Vector3 m_scale;

	Vector3 m_gPosition;
	Vector3 m_gScale;
	Quaternion m_gRotation;
};
