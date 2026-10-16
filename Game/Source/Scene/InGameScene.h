/**
 * @file InGameScene.h
 * @brief インゲームシーン
 */
#pragma once
#include "Source/Core/TransformNode.h"
#include "IScene.h"


namespace app
{
	/** 前方宣言 */
	class Enemy;

	namespace camera
	{
		class GameCamera;
	}

	namespace core
	{
		class TransformNode;
	}



	/**
	 * @brief インゲームシーン
	 */
	class InGameScene : public IScene
	{
		appScene(InGameScene);


	public:
		InGameScene();
		~InGameScene() override;

		bool Start() override;
		void Update() override;
		void Render(RenderContext& rc) override;

		/**
		 * @brief シーンの移行を要求する
		 * @param id シーンID
		 * @param waitTime シーン遷移するまでの時間
		 * @return true:移行する, false:移行しない
		 */
		bool RequestScene(uint32_t& id, float& waitTime) override;


	private:
		/** 写真撮影クラス */
		nsK2EngineLow::PhotoCapture m_photoCapture;
		/** トランスフォーム */
		app::core::TransformNode m_transform;
		/** 敵 */
		std::unique_ptr<app::Enemy> m_enemy;

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
		/** ピントの判定を記録 */
		bool m_isLastShotInFocus;
	};
}