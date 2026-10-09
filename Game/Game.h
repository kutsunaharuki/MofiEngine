#pragma once

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
	/** 写真撮影クラス */
	nsK2EngineLow::PhotoCapture m_photoCapture;

	/** アニメーションクリップ */
	AnimationClip m_animClips[2];
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
	/** 地面の位置 */
	Vector3 m_gPosition;
	/** 地面の大きさ */
	Vector3 m_gScale;
	/** 地面の回転 */
	Quaternion m_gRotation;
};
