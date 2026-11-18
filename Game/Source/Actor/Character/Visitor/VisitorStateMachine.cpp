/**
 * @file VisitorStateMachine.cpp
 * @brief 訪問者(一般客)のステートマシン
 */
#include "stdafx.h"
#include "VisitorStateMachine.h"
#include "VisitorState.h"


namespace app
{
	namespace actor
	{
		namespace
		{
			/** 移動量の閾値(これより小さければ止まっているとみなす) */
			constexpr float MOVE_AMOUNT_THRESHOLD = 0.1f;
			/** 向きを変える速さ */
			constexpr float ROTATE_SPEED = 8.0f;
			/** 回転の下限値 */
			constexpr float ROTATION_VALUE_LOW = 0.0f;
			/** 回転の上限値 */
			constexpr float ROTATION_VALUE_HIGH = 1.0f;
		}


		VisitorStateMachine::VisitorStateMachine(Visitor* visitorOwner)
			: CharacterStateMachine(visitorOwner)
			, m_visitorOwner(visitorOwner)
			, m_rotation(Quaternion::Identity)
		{
			// 訪問者のステートを追加
			AddState<VisitorIdleState>(*this);
			AddState<VisitorWalkState>(*this);
			AddState<VisitorTalkState>(*this);
			AddState<VisitorWalkTalkState>(*this);
		}


		VisitorStateMachine::~VisitorStateMachine()
		{}


		void VisitorStateMachine::Move(const float speed)
		{
			Vector3 direction = m_visitorOwner->GetMoveDirection();
			direction.y = 0.0f;
			const float amount = m_visitorOwner->GetMoveAmount();

			// 移動量が小さいまたは向きがない時は止まる
			if (amount < MOVE_AMOUNT_THRESHOLD || direction.LengthSq() <= 0.0f)
			{
				StopMove();
				return;
			}

			direction.Normalize();

			// 縦(y)はそのまま残して、横だけ変える
			Vector3 moveSpeed = m_visitorOwner->GetMoveSpeed();
			moveSpeed.x = direction.x * speed * amount;
			moveSpeed.z = direction.z * speed * amount;
			m_visitorOwner->SetMoveSpeed(moveSpeed);

			// 進む方向を向く
			RotateToward(direction);
		}


		void VisitorStateMachine::StopMove()
		{
			Vector3 moveSpeed = m_visitorOwner->GetMoveSpeed();
			moveSpeed.x = 0.0f;
			moveSpeed.z = 0.0f;
			m_visitorOwner->SetMoveSpeed(moveSpeed);
		}


		void VisitorStateMachine::FaceLookDirection()
		{
			Vector3 direction = m_visitorOwner->GetLookDirection();
			direction.y = 0.0f;

			// 向く方向が決まっていなければ、今の向きのまま
			if (direction.LengthSq() <= 0.0f) return;

			direction.Normalize();
			RotateToward(direction);
		}


		void VisitorStateMachine::RotateToward(const Vector3& direction)
		{
			// Y軸周りの回転を設定
			Quaternion target;
			target.SetRotationYFromDirectionXZ(direction);

			// 1フレームの経過時間を取得
			const float deltaTime = util::WorldTime::GetDeltaTime();

			// フレームレートが変わっても同じ速さで向く
			const float t = TClamp<float>::Clamp(ROTATE_SPEED * deltaTime, ROTATION_VALUE_LOW, ROTATION_VALUE_HIGH);
			m_rotation.Slerp(t, m_rotation, target);
			SetRotation(m_rotation);
		}


		core::IState* VisitorStateMachine::GetChangeState()
		{
			const bool isMoving = m_visitorOwner->GetMoveAmount() >= MOVE_AMOUNT_THRESHOLD;
			const bool isTalking = m_visitorOwner->IsTalking();

			// 動いているか × 話しているか の 4 通り
			if (isMoving)
			{
				if (isTalking) return FindState(VisitorWalkTalkState::ID());
				return FindState(VisitorWalkState::ID());
			}

			if (isTalking) return FindState(VisitorTalkState::ID());
			return FindState(VisitorIdleState::ID());
		}
	}
}
