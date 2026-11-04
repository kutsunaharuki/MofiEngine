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
			// 警戒中は警戒の速さ、普段は歩く速さ
			const float speed = m_stateMachine.IsAlert() ? parameter.alertSpeed : parameter.moveSpeed;
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
	}
}
