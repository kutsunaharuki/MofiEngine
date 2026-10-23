/**
 * @file BoyStateMachine.cpp
 * @brief 主人公のステートマシン
 */
#include "stdafx.h"
#include "BoyStateMachine.h"
#include "BoyState.h"
#include "Source/Camera/GameCamera.h"
#include <cmath>
#include <cfloat>


namespace app
{
	namespace actor
	{
		namespace
		{
			/** これより小さいスティックの入力は、入力なしとみなす */
			constexpr float MOVE_INPUT_THRESHOLD = 0.01f;
		}


		BoyStateMachine::BoyStateMachine(Boy* owner)
			: CharacterStateMachine(owner)
			, m_boy(owner)
			, m_rotation(Quaternion::Identity)
			, m_hideObjectPosition(Vector3::Zero)
		{
			// ステートの追加
			AddState<BoyIdleState>(*this);
			AddState<BoyWalkState>(*this);
			AddState<BoyPhotoIdleState>(*this);
			AddState<BoyPhotoWalkForwardState>(*this);
			AddState<BoyPhotoWalkBackState>(*this);
			AddState<BoyPhotoWalkLeftState>(*this);
			AddState<BoyPhotoWalkRightState>(*this);
			AddState<BoyLowHideState>(*this);
			AddState<BoyHighHideState>(*this);
		}


		core::IState* BoyStateMachine::GetChangeState()
		{
			// ファインダー状態かを見る
			const bool isFinderActive = m_boy->IsFinderActive();

			// ファインダーを覗いている時
			if (isFinderActive)
			{
				// ファインダーをのぞきながら移動入力があれば、スティックの向きの撮影歩行ステート
				if (HasMoveInput()) return FindPhotoWalkState();

				// 止まっている時に、近くに隠れられる物があれば隠れる
				const EnHideType hideType = CheckHideType();
				if (hideType == EnHideType::Low) return FindState(BoyLowHideState::ID());
				if (hideType == EnHideType::High) return FindState(BoyHighHideState::ID());

				// ファインダーを覗きっぱなしなら撮影待機ステート
				return FindState(BoyPhotoIdleState::ID());
			}


			// ファインダーを覗いていない時
			// 移動入力があるなら歩行ステートにする
			if (HasMoveInput()) return FindState(BoyWalkState::ID());


			// 5. 何もしていなければ待機
			return FindState(BoyIdleState::ID());
		}


		bool BoyStateMachine::HasMoveInput() const
		{
			return CalcMoveDirection().LengthSq() > 0.0f;
		}


		void BoyStateMachine::Move(const float speed, const bool isRotate)
		{
			const Vector3 direction = CalcMoveDirection();

			Vector3 moveSpeed = m_boy->GetMoveSpeed();
			moveSpeed.x = direction.x * speed;
			moveSpeed.z = direction.z * speed;
			m_boy->SetMoveSpeed(moveSpeed);

			if (isRotate && direction.LengthSq() > 0.0f)
			{
				RotateToward(direction);
			}
		}


		float BoyStateMachine::GetHorizontalSpeed() const
		{
			const Vector3& moveSpeed = m_boy->GetMoveSpeed();
			return std::sqrt(moveSpeed.x * moveSpeed.x + moveSpeed.z * moveSpeed.z);
		}


		void BoyStateMachine::StopMove()
		{
			Vector3 moveSpeed = m_boy->GetMoveSpeed();
			moveSpeed.x = 0.0f;
			moveSpeed.z = 0.0f;
			m_boy->SetMoveSpeed(moveSpeed);
		}


		void BoyStateMachine::FaceCameraForward()
		{
			const camera::GameCamera* gameCamera = m_boy->GetGameCamera();
			if (gameCamera == nullptr) return;

			RotateToward(gameCamera->GetForwardXZ());
		}


		void BoyStateMachine::SlowMove()
		{
			// 撮影中は横や後ろにも歩くので、移動する方向は向かずにカメラの方を向く
			Move(GetParameter().photoWalkSpeed, false);
			FaceCameraForward();
		}


		Vector3 BoyStateMachine::GetDirectionToHideObject() const
		{
			Vector3 direction = m_hideObjectPosition;
			direction.Subtract(m_boy->GetTransform().GetWorldPosition());
			direction.y = 0.0f;

			// 重なっていて方向が出せない時は、今の向きのままにする
			if (direction.LengthSq() <= 0.0f)
			{
				Vector3 forward(0.0f, 0.0f, 1.0f);
				m_rotation.Apply(forward);
				return forward;
			}

			direction.Normalize();
			return direction;
		}


		core::IState* BoyStateMachine::FindPhotoWalkState()
		{
			const float stickX = g_pad[0]->GetLStickXF();
			const float stickY = g_pad[0]->GetLStickYF();

			// 縦と横で、大きく倒している方を優先する
			if (std::fabs(stickY) >= std::fabs(stickX))
			{
				if (stickY >= 0.0f) return FindState(BoyPhotoWalkForwardState::ID());
				return FindState(BoyPhotoWalkBackState::ID());
			}

			if (stickX >= 0.0f) return FindState(BoyPhotoWalkRightState::ID());
			return FindState(BoyPhotoWalkLeftState::ID());
		}


		BoyStateMachine::EnHideType BoyStateMachine::CheckHideType()
		{
			const camera::GameCamera* gameCamera = m_boy->GetGameCamera();
			if (gameCamera == nullptr) return EnHideType::None;

			const BoyParameter& parameter = GetParameter();
			const Vector3& position = m_boy->GetTransform().GetWorldPosition();

			// カメラから見た前後左右の4方向を調べる
			Vector3 forward = gameCamera->GetForwardXZ();
			Vector3 right = gameCamera->GetRightXZ();
			Vector3 back = forward;
			back.Scale(-1.0f);
			Vector3 left = right;
			left.Scale(-1.0f);
			const Vector3 directions[] = { forward, back, left, right };

			EnHideType result = EnHideType::None;
			float nearestDistanceSq = FLT_MAX;

			for (const Vector3& direction : directions)
			{
				Vector3 toEnd = direction;
				toEnd.Scale(parameter.hideCheckDistance);

				// 膝の高さのレイ。当たらなければ、この方向には何もない
				Vector3 lowStart = position;
				lowStart.y += parameter.hideLowRayHeight;
				Vector3 lowEnd = lowStart;
				lowEnd.Add(toEnd);

				// TODO: 現在のレイキャストは、当たってほしくないコリジョンにも当たるため改造の余地がある。
				Vector3 lowHitPosition;
				if (!PhysicsWorld::GetInstance()->RayTest(lowStart, lowEnd, lowHitPosition)) continue;

				// 一番近い物に隠れる
				Vector3 toHit = lowHitPosition;
				toHit.Subtract(lowStart);
				const float distanceSq = toHit.LengthSq();
				if (distanceSq >= nearestDistanceSq) continue;

				// 頭の高さのレイ。当たれば壁や高い物、当たらなければ小さい物
				Vector3 highStart = position;
				highStart.y += parameter.hideHighRayHeight;
				Vector3 highEnd = highStart;
				highEnd.Add(toEnd);

				Vector3 highHitPosition;
				const bool isHighHit = PhysicsWorld::GetInstance()->RayTest(highStart, highEnd, highHitPosition);

				nearestDistanceSq = distanceSq;
				m_hideObjectPosition = lowHitPosition;
				result = isHighHit ? EnHideType::High : EnHideType::Low;
			}

			return result;
		}


		Vector3 BoyStateMachine::CalcMoveDirection() const
		{
			const float stickX = g_pad[0]->GetLStickXF();
			const float stickY = g_pad[0]->GetLStickYF();
			if (stickX * stickX + stickY * stickY < MOVE_INPUT_THRESHOLD * MOVE_INPUT_THRESHOLD)
			{
				return Vector3::Zero;
			}

			// カメラが無ければ、ワールドの軸を使う
			Vector3 forward(0.0f, 0.0f, 1.0f);
			Vector3 right(1.0f, 0.0f, 0.0f);
			if (const camera::GameCamera* gameCamera = m_boy->GetGameCamera())
			{
				forward = gameCamera->GetForwardXZ();
				right = gameCamera->GetRightXZ();
			}

			// スティックの上 = カメラの前、右 = カメラの右
			forward.Scale(stickY);
			right.Scale(stickX);
			Vector3 direction = forward;
			direction.Add(right);
			direction.y = 0.0f;

			// 入力がないときは向きは変えない
			if (direction.LengthSq() <= 0.0f) return Vector3::Zero;
			
			direction.Normalize();
			return direction;
		}


		void BoyStateMachine::RotateToward(const Vector3& direction)
		{
			const float deltaTime = g_gameTime->GetFrameDeltaTime();

			Quaternion target;
			target.SetRotationYFromDirectionXZ(direction);

			// フレームレートが変わっても、同じ速さで向くようにする
			const float t = TClamp<float>::Clamp(GetParameter().rotateSpeed * deltaTime, 0.0f, 1.0f);
			m_rotation.Slerp(t, m_rotation, target);
			SetRotation(m_rotation);
		}
	}
}
