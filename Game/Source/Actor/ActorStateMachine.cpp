/**
 * @file ActorStateMachine.cpp
 * @brief アクターのステートマシン
 */
#include "stdafx.h"
#include "ActorStateMachine.h"


namespace app
{
	namespace actor
	{
		core::IState* ActorStateMachine::GetChangeState()
		{
			// 何もしない
			return nullptr;
		}


		ActorStateMachine::ActorStateMachine(Actor* owner)
			: m_owner(owner)
		{}
	}
}