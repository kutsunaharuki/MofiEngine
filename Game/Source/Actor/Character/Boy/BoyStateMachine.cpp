/**
 * @file BoyStateMachine.cpp
 * @brief 主人公のステートマシン
 */
#include "stdafx.h"
#include "BoyStateMachine.h"
#include "BoyState.h"
#include "Source/Camera/GameCamera.h"
#include <cmath>


namespace app
{
	namespace actor
	{
		namespace
		{
			/** これより小さいスティックの入力は、入力なしとみなす */
			constexpr float MOVE_INPUT_THRESHOLD = 0.01f;
		}
		// ※ 速さや重力などの調整する値は、Assets/parameter/Actor/Boy.json にある(BoyParameter)


		BoyStateMachine::BoyStateMachine(Boy* owner)
			: CharacterStateMachine(owner)
			, m_boy(owner)
			, m_rotation(Quaternion::Identity)
			, m_isJumping(false)
		{
			// ステートの追加
			AddState<BoyIdleState>(*this);
			AddState<BoyWalkState>(*this);
			AddState<BoyRunState>(*this);
			AddState<BoyJumpState>(*this);
			AddState<BoyPhotoState>(*this);
		}


		core::IState* BoyStateMachine::GetChangeState()
		{
			const bool isFinderActive = m_boy->IsFinderActive();

			// 1. ファインダーを覗いている(着地してから撮影に入る)
			if (isFinderActive && IsOnGround())
			{
				m_isJumping = false;
				return FindState(BoyPhotoState::ID());
			}

			// 2. ジャンプ中は、着地するまでジャンプのまま
			if (m_isJumping)
			{
				return FindState(BoyJumpState::ID());
			}

			// 3. 地面にいるときに、Aボタンでジャンプ
			if (!isFinderActive && IsOnGround() && g_pad[0]->IsTrigger(enButtonA))
			{
				m_isJumping = true;
				return FindState(BoyJumpState::ID());
			}

			// 4. 移動の入力があれば、Bボタンを押している間は走り、それ以外は歩き
			if (!isFinderActive && HasMoveInput())
			{
				return g_pad[0]->IsPress(enButtonB) ? FindState(BoyRunState::ID()) : FindState(BoyWalkState::ID());
			}

			// 5. 何もしていなければ待機
			return FindState(BoyIdleState::ID());
		}


		bool BoyStateMachine::HasMoveInput() const
		{
			return CalcMoveDirection().LengthSq() > 0.0f;
		}


		void BoyStateMachine::Move(const float speed)
		{
			const Vector3 direction = CalcMoveDirection();

			Vector3 moveSpeed = m_boy->GetMoveSpeed();
			moveSpeed.x = direction.x * speed;
			moveSpeed.z = direction.z * speed;
			m_boy->SetMoveSpeed(moveSpeed);

			if (direction.LengthSq() > 0.0f)
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


		void BoyStateMachine::ApplyGravity()
		{
			Vector3 moveSpeed = m_boy->GetMoveSpeed();
			moveSpeed.y -= GetParameter().gravity * g_gameTime->GetFrameDeltaTime();
			m_boy->SetMoveSpeed(moveSpeed);
		}


		void BoyStateMachine::StartJump()
		{
			Vector3 moveSpeed = m_boy->GetMoveSpeed();
			moveSpeed.y = GetParameter().jumpSpeed;
			m_boy->SetMoveSpeed(moveSpeed);
		}


		void BoyStateMachine::FaceCameraForward()
		{
			const camera::GameCamera* gameCamera = m_boy->GetGameCamera();
			if (gameCamera == nullptr) return;

			RotateToward(gameCamera->GetForwardXZ());
		}


		Vector3 BoyStateMachine::CalcMoveDirection() const
		{
			const float stickX = g_pad[0]->GetLStickXF();
			const float stickY = g_pad[0]->GetLStickYF();
			if (stickX * stickX + stickY * stickY < MOVE_INPUT_THRESHOLD * MOVE_INPUT_THRESHOLD)
			{
				return Vector3(0.0f, 0.0f, 0.0f);
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

			if (direction.LengthSq() <= 0.0f)
			{
				return Vector3(0.0f, 0.0f, 0.0f);
			}
			direction.Normalize();
			return direction;
		}


		void BoyStateMachine::RotateToward(const Vector3& direction)
		{
			Quaternion target;
			target.SetRotationYFromDirectionXZ(direction);

			// フレームレートが変わっても、同じ速さで向くようにする
			const float t = TClamp<float>::Clamp(GetParameter().rotateSpeed * g_gameTime->GetFrameDeltaTime(), 0.0f, 1.0f);
			m_rotation.Slerp(t, m_rotation, target);
			SetRotation(m_rotation);
		}
	}
}
