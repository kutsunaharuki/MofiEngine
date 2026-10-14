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
			/** 目の高さ */
			constexpr float EYE_HEIGHT = 120.0f;
			/** 入力閾値 */
			constexpr float EPSILON = 0.001f;
			/** カメラ回転速度 */
			constexpr float CAMERA_ROTATE_SPEED = 1.7f;
			/** 上下の向きの最小値(度) */
			constexpr float PITCH_MIN_DEG = -60.0f;
			/** 上下の向きの最大値(度) */
			constexpr float PITCH_MAX_DEG = 60.0f;

			/** 円周率 */
			constexpr float PI = 3.14159265f;
			/** 度からラジアンへの変換係数 */
			constexpr float DEG_TO_RAD = PI / 180.0f;

			/** 3人称の時の、目の位置からカメラまでの位置 */
			constexpr float THIRD_PERSON_DISTANCE = 250.0f;
			/** 3人称の上下の向きの最小値(度。下向き) */
			constexpr float THIRD_PERSON_PITCH_MIN_DEG = -50.0f;
			/** 3人称の上下の向きの最大値(度。上向き) */
			constexpr float THIRD_PERSON_PITCH_MAX_DEG = 15.0f;
		}


		GameCamera::GameCamera()
			: m_modelRender(nullptr)
			, m_prevEyePos(Vector3::Zero)
			, m_target(nullptr)
			, m_yawDeg(0.0f)
			, m_pitchDeg(0.0f)
			, m_isFirstPerson(false)
		{}


		bool GameCamera::Start()
		{
			// カメラの近平面を設定
			g_camera3D->SetNear(CAMERA_NEAR);
			// カメラの遠平面を設定
			g_camera3D->SetFar(CAMERA_FAR);
			return true;
		}


		void GameCamera::Update()
		{
			// Rスティックの入力でカメラを回転
			RotateCamera();

			// 視点と視線の向きから、注視点を求める
			const Vector3 eyePos = CalcEyePos();
			const Vector3 forward = CalcForwardXZ();


			if (m_isFirstPerson)
			{
				// 1人称
				g_camera3D->SetPosition(eyePos);
				g_camera3D->SetTarget(eyePos + forward);
			}
			else
			{
				// 3人称
				g_camera3D->SetPosition(eyePos - forward * THIRD_PERSON_DISTANCE);
				g_camera3D->SetTarget(eyePos);
			}

			g_camera3D->Update();
		}


		void GameCamera::SetFirstPeerson(bool isFirstPerson)
		{
			m_isFirstPerson = isFirstPerson;

			// 3人称に戻すときは、3人称の範囲に収める
			if (!m_isFirstPerson)
			{
				m_pitchDeg = nsK2EngineLow::FloatClamp::Clamp(m_pitchDeg, THIRD_PERSON_PITCH_MIN_DEG, THIRD_PERSON_PITCH_MAX_DEG);
			}
		}


		Vector3 GameCamera::CalcEyePos()
		{
			// 追従対象がない場合は直前の値を返す
			if (!m_target) return m_prevEyePos;

			m_prevEyePos = m_target->GetWorldPosition();
			m_prevEyePos.y += EYE_HEIGHT;

			return m_prevEyePos;
		}


		Vector3 GameCamera::CalcForwardXZ() const
		{
			// YawとPitchをラジアンに変換
			const float yaw = m_yawDeg * DEG_TO_RAD;
			const float pitch = m_pitchDeg * DEG_TO_RAD;
			
			// yaw = 0, pitch = 0 のとき +Z方向を向く
			Vector3 forward;
			forward.x = sinf(yaw) * cosf(pitch);
			forward.y = sinf(pitch);
			forward.z = cosf(yaw) * cosf(pitch);
			
			return forward;
		}


		void GameCamera::RotateCamera()
		{
			// Rスティックの入力を取得
			float x = g_pad[0]->GetRStickXF();
			float y = g_pad[0]->GetRStickYF();

			// 入力がない場合は回転しない
			if (fabs(x) < EPSILON && fabs(y) < EPSILON) return;

			// 左右
			m_yawDeg += CAMERA_ROTATE_SPEED * x;
			// 上下
			m_pitchDeg += CAMERA_ROTATE_SPEED * y;

			// 上下は真上・真下を向かないようにクランプする(float型clamp)
			m_pitchDeg = nsK2EngineLow::FloatClamp::Clamp(m_pitchDeg, PITCH_MIN_DEG, PITCH_MAX_DEG);
		}


		Vector3 GameCamera::GetForwardXZ() const
		{
			// Yawをラジアンに変換
			const float yaw = m_yawDeg * DEG_TO_RAD;
			return Vector3(sinf(yaw), 0.0f, cosf(yaw));
		}


		Vector3 GameCamera::GetRightXZ() const
		{
			// Yawをラジアンに変換
			const float yaw = m_yawDeg * DEG_TO_RAD;
			return Vector3(sinf(yaw), 0.0f, -sinf(yaw));
		}
	}
}