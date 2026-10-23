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
		// カメラを構えている時の待機ステート
		/*************************************/


		void BoyPhotoIdleState::Enter()
		{
			m_stateMachine.PlayAnimation(EnBoyAnimation::PhotoIdle);
		}


		void BoyPhotoIdleState::Update()
		{
			// 動かずに、カメラの向いている方向を向く
			m_stateMachine.StopMove();
			m_stateMachine.FaceCameraForward();
		}


		void BoyPhotoIdleState::Exit()
		{}




		/*************************************/
		// カメラを構えている時の前歩きステート
		/*************************************/


		void BoyPhotoWalkForwardState::Enter()
		{
			m_stateMachine.PlayAnimation(EnBoyAnimation::PhotoWalkForward);
		}


		void BoyPhotoWalkForwardState::Update()
		{
			m_stateMachine.SlowMove();
		}


		void BoyPhotoWalkForwardState::Exit()
		{}




		/*************************************/
		// カメラを構えている時の後ろ歩きステート
		/*************************************/


		void BoyPhotoWalkBackState::Enter()
		{
			m_stateMachine.PlayAnimation(EnBoyAnimation::PhotoWalkBack);
		}


		void BoyPhotoWalkBackState::Update()
		{
			m_stateMachine.SlowMove();
		}


		void BoyPhotoWalkBackState::Exit()
		{}




		/*************************************/
		// カメラを構えている時の左歩きステート
		/*************************************/


		void BoyPhotoWalkLeftState::Enter()
		{
			m_stateMachine.PlayAnimation(EnBoyAnimation::PhotoWalkLeft);
		}


		void BoyPhotoWalkLeftState::Update()
		{
			m_stateMachine.SlowMove();
		}


		void BoyPhotoWalkLeftState::Exit()
		{}




		/*************************************/
		// カメラを構えている時の右歩きステート
		/*************************************/


		void BoyPhotoWalkRightState::Enter()
		{
			m_stateMachine.PlayAnimation(EnBoyAnimation::PhotoWalkRight);
		}


		void BoyPhotoWalkRightState::Update()
		{
			m_stateMachine.SlowMove();
		}


		void BoyPhotoWalkRightState::Exit()
		{}




		/*************************************/
		// 低い隠れステート
		/*************************************/


		void BoyLowHideState::Enter()
		{
			m_stateMachine.PlayAnimation(EnBoyAnimation::LowHide);
		}


		void BoyLowHideState::Update()
		{
			// 屈んだまま、隠れている物の方を向く
			m_stateMachine.StopMove();
			m_stateMachine.RotateToward(m_stateMachine.GetDirectionToHideObject());
		}


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
		{
			// 壁を背にするので、壁と反対の方向を向く
			m_stateMachine.StopMove();
			Vector3 direction = m_stateMachine.GetDirectionToHideObject();
			direction.Scale(-1.0f);
			m_stateMachine.RotateToward(direction);
		}


		void BoyHighHideState::Exit()
		{}
	}
}
