/**
 * @file EnemyStateMachine.cpp
 * @brief 敵のステートマシン
 */
#include "stdafx.h"
#include "EnemyStateMachine.h"
#include "EnemyState.h"


namespace app
{
	namespace actor
	{
		namespace
		{
			/** 移動量の閾値 */
			constexpr float MOVE_AMOUNT_THRESHOLD = 0.1f;
			/** 向きを変える速さ */
			constexpr float ROTATE_SPEED = 10.0f;
			/** 回転の下限値 */
			constexpr float ROTATION_VALUE_LOW = 0.0f;
			/** 回転の上限値 */
			constexpr float ROTATION_VALUE_HIGH = 1.0f;
		}


		EnemyStateMachine::EnemyStateMachine(EnemyBase* enemyOwner)
			: CharacterStateMachine(enemyOwner)
			, m_enemyOwner(enemyOwner)
			, m_moveAmount(0.0f)
			, m_moveDirection(Vector3::Zero)
			, m_isAlert(false)
			, m_rotation(Quaternion::Identity)
		{
			// 敵のステートを追加
			AddState<EnemyIdleState>(*this);
			AddState<EnemyWalkState>(*this);
			AddState<EnemyAlertState>(*this);
		}
		
		
		void EnemyStateMachine::Move(const float speed)
		{
			Vector3 direction = m_moveDirection;
			direction.y = 0.0f;

			// 移動量が小さいまたは向きがない時は止まる
			if (m_moveAmount < MOVE_AMOUNT_THRESHOLD || direction.LengthSq() <= 0.0f)
			{
				// 移動を止める
				StopMove();
				return;
			}

			direction.Normalize();

			Vector3 moveSpeed = m_enemyOwner->GetMoveSpeed();
			moveSpeed.x = direction.x * speed * m_moveAmount;
			moveSpeed.z = direction.z * speed * m_moveAmount;
			m_enemyOwner->SetMoveSpeed(moveSpeed);

			// 向きを変える
			RotateToward(m_moveDirection);
		}


		void EnemyStateMachine::StopMove()
		{
			// 移動速度を取得
			Vector3 moveSpeed = m_enemyOwner->GetMoveSpeed();
			moveSpeed.x = 0.0f;
			moveSpeed.z = 0.0f;
			m_enemyOwner->SetMoveSpeed(moveSpeed);
		}


		void EnemyStateMachine::RotateToward(const Vector3& direction)
		{
			// Y軸周りの回転を設定
			Quaternion target;
			target.SetRotationYFromDirectionXZ(direction);
			
			// 1フレームの経過時間を取得
			const float deltaTime = g_gameTime->GetFrameDeltaTime();

			// フレームレートが変わっても同じ速さで向く
			const float t = TClamp<float>::Clamp(ROTATE_SPEED * deltaTime, ROTATION_VALUE_LOW, ROTATION_VALUE_HIGH);
			m_rotation.Slerp(t, m_rotation, target);
			SetRotation(m_rotation);
		}


		core::IState* EnemyStateMachine::GetChangeState()
		{
			// 上になるほど優先度が高い
			if (m_isAlert) return FindState(EnemyAlertState::ID());
			if (m_moveAmount > MOVE_AMOUNT_THRESHOLD) return FindState(EnemyWalkState::ID());

			return FindState(EnemyIdleState::ID());
		}
	}
}