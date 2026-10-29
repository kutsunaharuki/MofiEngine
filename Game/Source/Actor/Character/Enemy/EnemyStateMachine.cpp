/**
 * @file EnemyStateMachine.cpp
 * @brief 敵のステートマシン
 */
#include "stdafx.h"
#include "EnemyStateMachine.h"
#include "EnemyState.h"
#include "Source/AI/Navigation/NavGrid.h"
#include "Source/AI/Navigation/Pathfinder.h"
#include "Source/AI/Navigation/PathFollower.h"


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
			, m_pathfinder(nullptr)
			, m_pathFollower(std::make_unique<ai::PathFollower>())
		{
			// 敵のステートを追加
			AddState<EnemyIdleState>(*this);
			AddState<EnemyWalkState>(*this);
			AddState<EnemyAlertState>(*this);
		}
		
		
		EnemyStateMachine::~EnemyStateMachine()
		{}


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

		void EnemyStateMachine::SetNavigation(const ai::Pathfinder* pathfinder, const ai::NavGrid* navGrid)
		{
			m_pathfinder = pathfinder;
			// 地図はうろつきの壁チェックに使う
			m_pathFollower->SetNavGrid(navGrid);
		}

		bool EnemyStateMachine::MoveTo(const Vector3 & goal)
		{
			// Pathfinder を借りていなければ探せない
			if (m_pathfinder == nullptr) return false;

			m_goal = goal;
			
			std::vector<Vector3> path;
			if (!m_pathfinder->FindPath(GetTransform().GetWorldPosition(), goal, path))
			{
				// 道が見つからなければ止まる
				StopPathFollow();
				return false;
			}

			// 道順を設定
			m_pathFollower->SetPath(path);
			return true;
		}


		void EnemyStateMachine::StopPathFollow()
		{
			// 止まらせる
			m_pathFollower->Clear();
			m_moveDirection = Vector3::Zero;
			m_moveAmount = 0.0f;
		}


		void EnemyStateMachine::UpdatePathFollow()
		{
			// 道順が無ければ、外から設定された移動方向・移動量をそのまま使用する
			if (!m_pathFollower->HasPath()) return;

			const Vector3& position = GetTransform().GetWorldPosition();
			const float deltaTime = g_gameTime->GetFrameDeltaTime();

			m_pathFollower->Update(position, deltaTime);

			// 詰まったら、今の位置から道順を求め直す(敵同士がぶつかった時などの保険)
			if (m_pathFollower->IsStuck())
			{
				if (MoveTo(m_goal))
				{
					// 求め直した場所でこのフレームの方向を出し直す
					m_pathFollower->Update(position, deltaTime);
				}
			}

			// 既存の入り口に流すだけ
			m_moveDirection = m_pathFollower->GetDirection();
			m_moveAmount = m_pathFollower->GetMoveAmount();
		}

		
		void EnemyStateMachine::SetWanderSetting(const ai::WanderSetting & setting)
		{
			m_pathFollower->SetWanderSetting(setting);
		}

		
		bool EnemyStateMachine::IsFollowingPath() const
		{
			return m_pathFollower->HasPath();
		}

		
		bool EnemyStateMachine::IsArrived() const
		{
			return m_pathFollower->IsArrived();
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