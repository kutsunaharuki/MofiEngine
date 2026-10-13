/**
 * @file CharacterStateMachine.cpp
 * @brief キャラクターステートマシン
 */
#include "stdafx.h"
#include "CharacterStateMachine.h"


namespace app
{
	namespace actor
	{
		CharacterStateMachine::CharacterStateMachine(CharacterBase* owner)
			: ActorStateMachine(owner)
			, m_characterOwner(owner)
		{}
	}
}
