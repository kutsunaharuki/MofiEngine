/**
 * @file EnemyState.cpp
 * @brief 敵のステート
 */
#include "stdafx.h"
#include "EnemyState.h"
#include "Object/EnemyParameter.h"


namespace app
{
	namespace actor
	{
		void app::actor::EnemyIdleState::Enter()
		{
			// 待機アニメーションを再生
			m_stateMachine.PlayAnimation(EnemyBase::EnemyAnimation::Idle);
		}
		
		
		void EnemyIdleState::Update()
		{
			m_stateMachine.StopMove();
		}
		
		
		void EnemyIdleState::Exit()
		{}




		/*************************************************/


		void EnemyWalkState::Enter()
		{
			// 歩行アニメーションを再生
			m_stateMachine.PlayAnimation(EnemyBase::EnemyAnimation::Walk);
		}
		
		
		void EnemyWalkState::Update()
		{
			// 敵のパラメーターを取得(大男とねずみ男で違う値)
			const EnemyParameter& parameter = m_stateMachine.GetParameter();
			// 警戒中は警戒の速さ、普段と、調べに行く時(音がした所・見失った所)は歩く速さ
			const bool isAlertSpeed = m_stateMachine.IsAlert() && !m_stateMachine.IsInvestigating();
			const float speed = isAlertSpeed ? parameter.alertSpeed : parameter.moveSpeed;
			m_stateMachine.Move(speed);
		}
		
		
		void EnemyWalkState::Exit()
		{}




		/************************************************/


		void EnemyAlertState::Enter()
		{
			// 警戒アニメーションを再生
			m_stateMachine.PlayAnimation(EnemyBase::EnemyAnimation::Alert);
		}
		
		
		void EnemyAlertState::Update()
		{
			// 警戒中は足を止める
			m_stateMachine.StopMove();
		}
		
		
		void EnemyAlertState::Exit()
		{}




		/************************************************/


		void EnemyStealState::Enter()
		{
			// 盗むアニメーションを再生
			m_stateMachine.PlayAnimation(EnemyBase::EnemyAnimation::Steal);
		}


		void EnemyStealState::Update()
		{
			// 盗んでいる間はその場で止まる
			m_stateMachine.StopMove();
		}


		void EnemyStealState::Exit()
		{}




		/************************************************/


		void EnemyFoundState::Enter()
		{
			m_timer = 0.0f;
			m_isRunning = false;
			m_stateMachine.StopMove();
			// 止まって、主人公を指さす
			m_stateMachine.SetAnimationSpeed(1.0f);
			m_stateMachine.PlayAnimation(EnemyBase::EnemyAnimation::Point, 0.1f);
		}


		void EnemyFoundState::Update()
		{
			const EnemyParameter& parameter = m_stateMachine.GetParameter();
			m_timer += util::WorldTime::GetDeltaTime();

			// 主人公への向き(高さは見ない)
			Vector3 toTarget = m_stateMachine.GetFoundTarget() - m_stateMachine.GetTransform().GetWorldPosition();
			toTarget.y = 0.0f;
			const float distance = toTarget.Length();

			// ① 指さしている間は、その場で主人公の方を向くだけ
			if (m_timer < parameter.foundReactTime)
			{
				m_stateMachine.StopMove();
				if (distance > 0.0f) m_stateMachine.RotateToward(toTarget);
				return;
			}

			// ③ 近づいたら、つかまえた。止まって主人公の方を向いたままにする
			// ※ 見失った時は、EnemyBase が見つけたステートをやめさせる(つかまえない)
			if (m_stateMachine.IsCaught() || distance <= parameter.foundCatchDistance)
			{
				m_stateMachine.StopMove();
				if (distance > 0.0f) m_stateMachine.RotateToward(toTarget);
				if (!m_stateMachine.IsCaught())
				{
					m_stateMachine.SetCaught(true);
					m_stateMachine.SetAnimationSpeed(1.0f);
					// 追いついたら、もう一度主人公を指さして、そのまま止まる
					m_stateMachine.PlayAnimation(EnemyBase::EnemyAnimation::Point, 0.15f);
				}
				return;
			}

			// ② 主人公の方へ走る(走るアニメーションが無いので、歩くアニメーションを速く回す)
			if (!m_isRunning)
			{
				m_isRunning = true;
				m_stateMachine.PlayAnimation(EnemyBase::EnemyAnimation::Walk, 0.15f);
				m_stateMachine.SetAnimationSpeed(parameter.foundRunAnimationSpeed);
			}
			// 道順があれば、道順に沿って走る(ベンチなどをよける。方向は EnemyBase が UpdatePathFollow で入れている)
			// 道順が無い・着いてしまった時は、まっすぐ主人公の方へ走る
			if (!m_stateMachine.IsFollowingPath() || m_stateMachine.IsArrived())
			{
				m_stateMachine.SetMoveDirection(toTarget);
			}
			m_stateMachine.SetMoveAmount(1.0f);
			m_stateMachine.Move(parameter.foundRunSpeed);
		}


		void EnemyFoundState::Exit()
		{
			// アニメーションの速さを元にもどす
			m_stateMachine.SetAnimationSpeed(1.0f);
		}
	}
}
