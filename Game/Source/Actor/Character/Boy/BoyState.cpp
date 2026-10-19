/**
 * @file BoyState.cpp
 * @brief 主人公のステート群
 */
#include "stdafx.h"
#include "BoyState.h"
#include "BoyStateMachine.h"
#include <algorithm>


namespace app
{
	namespace actor
	{
		// ※ 速さなどの調整する値は、Assets/parameter/Actor/Boy.json にある(BoyParameter)


		/*************************************/
		// BoyIdleState
		/*************************************/


		void BoyIdleState::Enter()
		{
			m_stateMachine.PlayAnimation(EnBoyAnimation::Idle);
		}


		void BoyIdleState::Update()
		{
			m_stateMachine.StopMove();
			m_stateMachine.ApplyGravity();
		}


		void BoyIdleState::Exit()
		{}




		/*************************************/
		// BoyWalkState
		/*************************************/


		void BoyWalkState::Enter()
		{
			m_stateMachine.PlayAnimation(EnBoyAnimation::Walk);
		}


		void BoyWalkState::Update()
		{
			m_stateMachine.Move(m_stateMachine.GetParameter().walkSpeed);
			m_stateMachine.ApplyGravity();
		}


		void BoyWalkState::Exit()
		{}




		/*************************************/
		// BoyRunState
		/*************************************/


		void BoyRunState::Enter()
		{
			m_stateMachine.PlayAnimation(EnBoyAnimation::Run);
		}


		void BoyRunState::Update()
		{
			m_stateMachine.Move(m_stateMachine.GetParameter().runSpeed);
			m_stateMachine.ApplyGravity();
		}


		void BoyRunState::Exit()
		{}




		/*************************************/
		// BoyJumpState
		/*************************************/


		void BoyJumpState::Enter()
		{
			m_elapsedTime = 0.0f;

			// 走りながら跳んだときに、急に遅くならないように、跳んだ瞬間の速さを引き継ぐ
			// ※ 止まって跳んだときも、空中では airMoveSpeed までは動ける
			m_airMoveSpeed = (std::max)(m_stateMachine.GetHorizontalSpeed(), m_stateMachine.GetParameter().airMoveSpeed);

			m_stateMachine.PlayAnimation(EnBoyAnimation::Jump, 0.1f);
			m_stateMachine.StartJump();
		}


		void BoyJumpState::Update()
		{
			m_elapsedTime += g_gameTime->GetFrameDeltaTime();

			// 空中でも、入力の方向に動ける
			m_stateMachine.Move(m_airMoveSpeed);
			m_stateMachine.ApplyGravity();

			// 着地したら、ジャンプを終わる(次のフレームで待機か移動に変わる)
			// ※ ジャンプした直後は、まだ地面に着いている扱いなので、少し待ってから判定する
			if (m_elapsedTime > m_stateMachine.GetParameter().jumpLandingCheckTime && m_stateMachine.IsOnGround())
			{
				m_stateMachine.EndJump();
			}
		}


		void BoyJumpState::Exit()
		{}




		/*************************************/
		// BoyPhotoState
		/*************************************/


		void BoyPhotoState::Enter()
		{
			m_stateMachine.PlayAnimation(EnBoyAnimation::Idle);
		}


		void BoyPhotoState::Update()
		{
			// 撮影中は動かずに、カメラの向いている方向を向く
			m_stateMachine.StopMove();
			m_stateMachine.FaceCameraForward();
			m_stateMachine.ApplyGravity();
		}


		void BoyPhotoState::Exit()
		{}
	}
}
