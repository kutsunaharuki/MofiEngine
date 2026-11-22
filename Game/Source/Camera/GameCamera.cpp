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
			/**
			 * カメラの近平面
			 * ※ 小さすぎると、奥行きの細かさが足りなくなり、近い面どうし(床と物・壁と絵画)がちらつく。
			 *    0.1 だと 10m 先で約 0.6cm の差が見分けられない(額縁と壁は 0.2cm しか離れていない所がある)。5 にすると 50 倍細かくなる。
			 *    カメラの当たり判定の半径(GameCamera.json の collisionRadius = 15)より小さくしておく(壁が切れて見えないように)
			 */
			constexpr float CAMERA_NEAR = 5.0f;
			/** カメラの遠平面 */
			constexpr float CAMERA_FAR = 10000.0f;
			/** 入力閾値 */
			constexpr float EPSILON = 0.001f;
			/** 調整用パラメーターの JSON(回る速さ・距離・目の高さ) */
			constexpr const char* GAME_CAMERA_PARAMETER_PATH = "Assets/parameter/Camera/GameCamera.json";
			/** 上下の向きの最小値(度) */
			constexpr float PITCH_MIN_DEG = -60.0f;
			/** 上下の向きの最大値(度) */
			constexpr float PITCH_MAX_DEG = 60.0f;

			/** 円周率 */
			constexpr float PI = 3.14159265f;
			/** 度からラジアンへの変換係数 */
			constexpr float DEG_TO_RAD = PI / 180.0f;

			/** 3人称の上下の向きの最小値(度。下向き) */
			constexpr float THIRD_PERSON_PITCH_MIN_DEG = -50.0f;
			/** 3人称の上下の向きの最大値(度。上向き) */
			constexpr float THIRD_PERSON_PITCH_MAX_DEG = 15.0f;


			/**
			 * @brief 球を飛ばして、一番近くで当たった所を残すコールバック(キャラクターには当たらない)
			 * @details 主人公・悪党・一般客の体でカメラが縮まないように、キャラクターは無視する
			 */
			struct CameraSweepCallback : public btCollisionWorld::ClosestConvexResultCallback
			{
				CameraSweepCallback(const btVector3& from, const btVector3& to)
					: btCollisionWorld::ClosestConvexResultCallback(from, to)
				{}


				bool needsCollision(btBroadphaseProxy* proxy) const override
				{
					const btCollisionObject* object = static_cast<const btCollisionObject*>(proxy->m_clientObject);
					// キャラクターの体には当たらない
					if (object->getUserIndex() == EnCollisionAttr::enCollisionAttr_Character) return false;

					return btCollisionWorld::ClosestConvexResultCallback::needsCollision(proxy);
				}
			};
		}


		GameCamera::GameCamera()
			: m_modelRender(nullptr)
			, m_prevEyePos(Vector3::Zero)
			, m_target(nullptr)
			, m_yawDeg(0.0f)
			, m_pitchDeg(0.0f)
			, m_isFirstPerson(false)
			, m_zoomRatio(1.0f)
			, m_currentDistance(-1.0f)
		{}


		bool GameCamera::Start()
		{
			// 調整用パラメーターの読み込み
			m_parameter.Initialize(GAME_CAMERA_PARAMETER_PATH);

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
				// 3人称(壁や手すりの向こうに行かない距離にする)
				const Vector3 backward = forward * -1.0f;
				const float distance = CalcThirdPersonDistance(eyePos, backward);
				g_camera3D->SetPosition(eyePos + backward * distance);
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
				m_pitchDeg = nsK2EngineLow::TClamp<float>::Clamp(m_pitchDeg, THIRD_PERSON_PITCH_MIN_DEG, THIRD_PERSON_PITCH_MAX_DEG);
			}
		}


		Vector3 GameCamera::CalcEyePos()
		{
			// 追従対象がない場合は直前の値を返す
			if (!m_target) return m_prevEyePos;

			m_prevEyePos = m_target->GetWorldPosition();
			m_prevEyePos.y += m_parameter.Get().eyeHeight;

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

			// ファインダーを覗いているときは、狙いやすいようにゆっくり回す
			// さらにズームしているほど遅くする(見えている範囲が狭い分だけ遅くして、景色が流れる速さをそろえる)
			const GameCameraParameter& parameter = m_parameter.Get();
			const float rotateSpeed = m_isFirstPerson ? parameter.finderRotateSpeed * m_zoomRatio : parameter.rotateSpeed;

			// フレームレートが変わっても、同じ速さで回るようにする(速さは 1 秒に回る角度)
			const float deltaTime = g_gameTime->GetFrameDeltaTime();
			// 左右
			m_yawDeg += rotateSpeed * x * deltaTime;
			// 上下
			m_pitchDeg += rotateSpeed * y * deltaTime;

			// 上下は真上・真下を向かないようにクランプする(float型clamp)
			// ※ 3人称のときは、カメラが地面に潜らないように、3人称の範囲に収める
			if (m_isFirstPerson)
			{
				m_pitchDeg = nsK2EngineLow::TClamp<float>::Clamp(m_pitchDeg, PITCH_MIN_DEG, PITCH_MAX_DEG);
			}
			else
			{
				m_pitchDeg = nsK2EngineLow::TClamp<float>::Clamp(m_pitchDeg, THIRD_PERSON_PITCH_MIN_DEG, THIRD_PERSON_PITCH_MAX_DEG);
			}
		}


		float GameCamera::CalcThirdPersonDistance(const Vector3& eyePos, const Vector3& backward)
		{
			const GameCameraParameter& parameter = m_parameter.Get();
			const float maxDistance = parameter.thirdPersonDistance;

			// 最初のフレームは、元の距離から始める
			if (m_currentDistance < 0.0f) m_currentDistance = maxDistance;

			// 目の位置から、カメラを置きたい所まで球を飛ばす
			// ※ レイ(細い線)だと手すりの隙間を抜けたり、壁ぎりぎりで近平面が壁にめり込んだりするので、太さのある球にする
			const Vector3 end = eyePos + backward * maxDistance;
			btSphereShape sphere(parameter.collisionRadius);
			btTransform from;
			btTransform to;
			from.setIdentity();
			to.setIdentity();
			from.setOrigin(btVector3(eyePos.x, eyePos.y, eyePos.z));
			to.setOrigin(btVector3(end.x, end.y, end.z));

			CameraSweepCallback callback(from.getOrigin(), to.getOrigin());
			PhysicsWorld::GetInstance()->GetDynamicWorld()->convexSweepTest(&sphere, from, to, callback);

			// 当たらなければ元の距離、当たったら当たった所まで(m_closestHitFraction は 0 ~ 1 の割合)
			float targetDistance = maxDistance;
			if (callback.hasHit())
			{
				targetDistance = maxDistance * callback.m_closestHitFraction;
			}
			// 主人公に近づきすぎない(頭の中が写らないように)
			// ※ Windows.h の max マクロとぶつかるので、std::max は使わない
			if (targetDistance < parameter.minDistance) targetDistance = parameter.minDistance;

			if (targetDistance < m_currentDistance)
			{
				// 壁が来たら、すぐ縮める(1 フレームでも壁の向こうが見えないように)
				m_currentDistance = targetDistance;
			}
			else
			{
				// 壁が無くなったら、少しずつ戻す(急に後ろへ跳ねないように)
				const float step = parameter.returnSpeed * g_gameTime->GetFrameDeltaTime();
				m_currentDistance += step;
				if (m_currentDistance > targetDistance) m_currentDistance = targetDistance;
			}

			return m_currentDistance;
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
			// 前方向(sin, 0, cos)を、Y軸で右に90度回したもの
			return Vector3(cosf(yaw), 0.0f, -sinf(yaw));
		}
	}
}