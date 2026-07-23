/**
 * @file GameCamera.cpp
 * @brief ゲームカメラクラス
 */
#include "stdafx.h"
#include "GameCamera.h"


namespace app
{
	namespace camera
	{
		namespace
		{
			/** カメラの近平面 */
			constexpr float CAMERA_NEAR = 0.1f;
			/** カメラの遠平面 */
			constexpr float CAMERA_FAR = 10000.0f;
			/** 注視点から視点までの位置 */
			const Vector3 TO_CAMERA_POS = { 0.0f, 120.0f, -200.0f };
			/** 注視点のY軸を上げる */
			constexpr float TARGET_CAMERA_Y_UP = 100.0f;
			/** 入力閾値 */
			constexpr float EPSILON = 0.001f;
			/** カメラ回転速度 */
			constexpr float CAMERA_ROTATE_SPEED = 1.3f;
			/** カメラの最小Y角度 */
			constexpr float CAMERA_ANGLE_MIN_Y = -0.2f;
			/** カメラの最大Y角度 */
			constexpr float CAMERA_ANGLE_MAX_Y = 0.9f;
		}


		GameCamera::GameCamera()
			: m_modelRender(nullptr)
			, m_toCameraPos(TO_CAMERA_POS)
			, m_prevTarget(Vector3::Zero)
		{}


		bool GameCamera::Start()
		{
			// 注視点から視点までの位置を設定
			m_toCameraPos.Set(TO_CAMERA_POS);
			// カメラの近平面を設定
			g_camera3D->SetNear(CAMERA_NEAR);
			// カメラの遠平面を設定
			g_camera3D->SetFar(CAMERA_FAR);

			return true;
		}


		void GameCamera::Update()
		{
			// 注視点の計算
			Vector3 target = CalcTargetPos();
			g_camera3D->SetTarget(target);

			// Rスティックの入力でカメラを回転
			RotateCamera();
			
			// 視点の計算
			Vector3 pos = target + m_toCameraPos;
			g_camera3D->SetPosition(pos);

			g_camera3D->Update();
		}


		Vector3 GameCamera::CalcTargetPos()
		{
			if (!m_modelRender) return m_prevTarget;

			// 注視点
			Vector3 target;
			target = m_modelRender->GetPosition();
			target.y += TARGET_CAMERA_Y_UP;
			m_prevTarget = target;
			return m_prevTarget;
		}


		void GameCamera::RotateCamera()
		{
			// Rスティックの入力を取得
			float x = g_pad[0]->GetRStickXF();
			float y = g_pad[0]->GetRStickYF();

			// 入力がない場合は回転しない
			if (fabs(x) < EPSILON && fabs(y) < EPSILON) return;

			// 回転前の状態をローカル変数で保持
			Vector3 toCameraPosOld = m_toCameraPos;

			// Y軸周りの回転
			Quaternion qrotY;
			qrotY.SetRotationDeg(Vector3::AxisY, CAMERA_ROTATE_SPEED * x);
			qrotY.Apply(m_toCameraPos);

			// X軸周りの回転
			Vector3 axisX;
			axisX.Cross(Vector3::AxisY, m_toCameraPos);
			axisX.Normalize();

			// 求めた軸周りの回転
			Quaternion qrotX;
			qrotX.SetRotationDeg(axisX, CAMERA_ROTATE_SPEED * y);
			qrotX.Apply(m_toCameraPos);

			// 回転後のカメラの角度が許容範囲外なら元の状態に戻す
			if (!IsCameraAngleValid(m_toCameraPos))
			{
				m_toCameraPos = toCameraPosOld;
			}
		}
		
		
		bool GameCamera::IsCameraAngleValid(const Vector3& toCameraPos)
		{
			// 注視点から視点までのベクトルを正規化する
			Vector3 toPosDir = toCameraPos;
			toPosDir.Normalize();

			// 下に向きすぎか上に向きすぎかを判定
			if (toPosDir.y < CAMERA_ANGLE_MIN_Y || toPosDir.y > CAMERA_ANGLE_MAX_Y)
			{
				return false;
			}
			return true	;
		}
	}
}