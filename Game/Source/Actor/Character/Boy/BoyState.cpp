/**
 * @file BoyState.cpp
 * @brief 主人公のステート群
 */
#include "stdafx.h"
#include "BoyState.h"
#include "BoyStateMachine.h"
#include <algorithm>
#include "Object/BoyParameter.h"


namespace app
{
	namespace actor
	{
		/*************************************/
		// カメラを構えていないときの待機ステート
		/*************************************/


		void BoyIdleState::Enter()
		{
			m_stateMachine.PlayAnimation(EnBoyAnimation::Idle);
		}


		void BoyIdleState::Update()
		{
			m_stateMachine.StopMove();
		}


		void BoyIdleState::Exit()
		{}




		/*************************************/
		// カメラを構えていないときの歩行ステート
		/*************************************/


		void BoyWalkState::Enter()
		{
			m_stateMachine.PlayAnimation(EnBoyAnimation::Walk);
		}


		void BoyWalkState::Update()
		{
			m_stateMachine.Move(m_stateMachine.GetParameter().walkSpeed);
		}


		void BoyWalkState::Exit()
		{}




		/*************************************/
		// カメラを構えていないときの走行ステート
		/*************************************/


		void BoyRunState::Enter()
		{
			m_stateMachine.PlayAnimation(EnBoyAnimation::Run);
		}


		void BoyRunState::Update()
		{}


		void BoyRunState::Exit()
		{}




		/*************************************/
		// カメラを構えている時の待機ステート
		/*************************************/


		void BoyPhotoIdleState::Enter()
		{
			m_stateMachine.PlayAnimation(EnBoyAnimation::PhotoIdle);
		}


		void BoyPhotoIdleState::Update()
		{}


		void BoyPhotoIdleState::Exit()
		{}




		/*************************************/
		// カメラを構えている時の歩行ステート
		/*************************************/


		void BoyPhotoWalkState::Enter()
		{
			m_stateMachine.PlayAnimation(EnBoyAnimation::PhotoWalk);
		}

		
		void BoyPhotoWalkState::Update()
		{}


		void BoyPhotoWalkState::Exit()
		{}




		/*************************************/
		// 低い隠れステート
		/*************************************/


		void BoyLowHideState::Enter()
		{
			m_stateMachine.PlayAnimation(EnBoyAnimation::LowHide);
		}


		void BoyLowHideState::Update()
		{}


		void BoyLowHideState::Exit()
		{}




		/*************************************/
		// 高い隠れステート
		/*************************************/


		void BoyHighHideState::Enter()
		{
			m_stateMachine.PlayAnimation(EnBoyAnimation::HighHide);
		}


		void BoyHighHideState::Update()
		{}


		void BoyHighHideState::Exit()
		{}
	}
}
