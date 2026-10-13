/**
 * @file GameCamera.h
 * @brief ゲームカメラクラス
 */
#pragma once
#include "Source/Core/TransformNode.h"


namespace app
{
	namespace camera
	{
		/**
		 * @brief ゲームカメラクラス
		 */
		class GameCamera
		{
		public:
			GameCamera();
			~GameCamera() = default;
			bool Start();
			void Update();


			/**
			 * @brie 水平方向(XZ平面)の前方向を取得
			 * @return 前方向
			 * @details プレイヤーの移動に使う
			 */
			Vector3 GetForwardXZ() const;
			/**
			 * @brief 水平方向(XZ平面)の右方向を取得
			 * @return 右方向
			 * @details プレイヤーの移動に使う
			 */
			Vector3 GetRightXZ() const;

			/**
			 * @brief 追従対象を設定する
			 * @param target 追従対象のトランスフォーム
			 */
			void SetTarget(const core::TransformNode* target) { m_target = target; }


		private:
			/**
			 * @brief 1人称視点の計算を行う関数
			 * @return 視点
			 */
			Vector3 CalcEyePos();
			/**
			 * @brief 視線の向きを計算する
			 * @return 視線の向き
			 */
			Vector3 CalcForwardXZ() const;
			/**
			 * @brief スティック入力で回転させる
			 */
			void RotateCamera();


		private:
			/** モデルレンダー */
			ModelRender* m_modelRender;
			/** 追従対象 */
			const core::TransformNode* m_target;
			/** 左右の向き(度) */
			float m_yawDeg;
			/** 上下の向き(度。正で上向き) */
			float m_pitchDeg;
			/** 直前の視点 */
			Vector3 m_prevEyePos;
		};
	}
}

