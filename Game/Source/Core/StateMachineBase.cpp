/**
 * @file ActorStateMachineBase.cpp
 * @brief アクターのステートマシンの基底クラス群
 */
#include "stdafx.h"
#include "StateMachineBase.h"


namespace app
{
	namespace core
	{
		StateMachineBase::StateMachineBase()
			: m_currentState(nullptr)
		{
			// ステートマップをクリア
			m_stateMap.clear();
		}


		void StateMachineBase::Update()
		{
			// ステートを変更
			ChangeState();

			// 現在のステートなら更新する
			if (m_currentState)
			{
				m_currentState->Update();
			}
		}


		void StateMachineBase::ChangeState()
		{
			// 変更先のステートを取得
			m_nextState = GetChangeState();

			// 変更先のステートがあり、かつ今のステートと違うなら
			if (m_nextState && m_currentState != m_nextState)
			{
				// 今のステートなら
				if (m_currentState)
				{
					// 退去する
					m_currentState->Exit();
				}

				m_currentState = m_nextState;
				// 新しいステートに入る
				m_currentState->Enter();
				// 変更先のステートはnullptr(空)にする
				m_nextState = nullptr;
			}
		}


		void StateMachineBase::ReEnterCurrentState()
		{
			// 今のステートなら Enter() を再度呼び出してアニメーションを適用
			if (m_currentState)
			{
				m_currentState->Enter();
			}
		}


		bool StateMachineBase::IsEqualState(const uint32_t stateID) const
		{
			return m_currentState == m_stateMap.at(stateID).get();
		}

		
		IState* StateMachineBase::FindState(const uint32_t stateID)
		{
			// 指定したIDを取得
			const auto& it = m_stateMap.find(stateID);
			// IDが外れ値の場合はnullptrを返す
			if(it == m_stateMap.end()) return nullptr;
			// ステートを返す
			return it->second.get();
		}
	}
}