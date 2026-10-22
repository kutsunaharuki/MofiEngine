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


		BoyStateMachine::BoyStateMachine(Boy* owner)
			: CharacterStateMachine(owner)
			, m_boy(owner)
			, m_rotation(Quaternion::Identity)
		{
			// ステートの追加
			AddState<BoyIdleState>(*this);
			AddState<BoyWalkState>(*this);
			AddState<BoyRunState>(*this);
			AddState<BoyPhotoIdleState>(*this);
			AddState<BoyPhotoWalkState>(*this);
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
				// ファインダーをのぞきながら移動入力があれば撮影歩行ステート
				if (HasMoveInput()) return FindState(BoyPhotoWalkState::ID());

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


		void BoyStateMachine::FaceCameraForward()
		{
			const camera::GameCamera* gameCamera = m_boy->GetGameCamera();
			if (gameCamera == nullptr) return;

			RotateToward(gameCamera->GetForwardXZ());
		}


		void BoyStateMachine::SlowMove()
		{
			// ファインダーを覗いていない時は歩行速度はそのまま
			if (!m_boy->IsFinderActive()) return;

			Move(GetParameter().photoWalkSpeed);
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
