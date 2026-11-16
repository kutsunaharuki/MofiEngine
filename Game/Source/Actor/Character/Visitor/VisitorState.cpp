/**
 * @file VisitorState.cpp
 * @brief 訪問者(一般客)のステート
 */
#include "stdafx.h"
#include "VisitorState.h"
#include "VisitorStateMachine.h"



namespace app
{
	namespace actor
	{
		void VisitorIdleState::Enter()
		{
			m_stateMachine.PlayAnimation(Visitor::VisitorAnimation::Idle);
		}


		void VisitorIdleState::Update()
		{
			// 止まって、美術品の方を向く
			m_stateMachine.StopMove();
			m_stateMachine.FaceLookDirection();
		}


		void VisitorIdleState::Exit()
		{}




		/***************************************************/


		void VisitorWalkState::Enter()
		{
			m_stateMachine.PlayAnimation(Visitor::VisitorAnimation::Walk);
		}


		void VisitorWalkState::Update()
		{
			// 歩く速さで、入力された方向へ進む
			m_stateMachine.Move(m_stateMachine.GetParameter().walkSpeed);
		}


		void VisitorWalkState::Exit()
		{}




		/**************************************************/


		void VisitorTalkState::Enter()
		{
			m_stateMachine.PlayAnimation(Visitor::VisitorAnimation::Talk);
		}


		void VisitorTalkState::Update()
		{
			// その場で話す(止まって、美術品の方を向く)
			m_stateMachine.StopMove();
			m_stateMachine.FaceLookDirection();
		}


		void VisitorTalkState::Exit()
		{}




		/*********************************************/


		void VisitorWalkTalkState::Enter()
		{
			m_stateMachine.PlayAnimation(Visitor::VisitorAnimation::WalkTalk);
		}


		void VisitorWalkTalkState::Update()
		{
			// 歩きながら話す(歩幅が違うので、歩きとは別の速さ)
			m_stateMachine.Move(m_stateMachine.GetParameter().walkTalkSpeed);
		}


		void VisitorWalkTalkState::Exit()
		{}
	}
}
