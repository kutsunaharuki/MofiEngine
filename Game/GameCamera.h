/**
 * @file GameCamera.h
 * @brief ゲームカメラクラス
 */
#pragma once
#include "Source/Core/Transform.h"


namespace app
{
	namespace camera
	{
		class GameCamera : public IGameObject
		{
		public:
			GameCamera();
			~GameCamera() = default;
			bool Start() override;
			void Update() override;

			/**
			 * @brief 注視点の計算を行う関数
			 */
			Vector3 CalcTargetPos();

			/**
			 * @brief スティック入力での回転
			 */
			void RotateCamera();

			/**
			 * @brief カメラの上下角度の制限チェックを行う関数
			 * @param toCameraPos 注視点からカメラまでの位置
			 */
			bool IsCameraAngleValid(const Vector3& toCameraPos);

			/**
			 * @brief 追従対象を設定する
			 * @param target 追従対象のモデルレンダー
			 */
			void SetTarget(ModelRender* target) { m_modelRender = target; }


		private:
			/** モデルレンダー */
			ModelRender* m_modelRender;
			/** 注視点から視点までの位置 */
			Vector3 m_toCameraPos;
			/** 直前の値を保持 */
			Vector3 m_prevTarget;
		};
	}
}

