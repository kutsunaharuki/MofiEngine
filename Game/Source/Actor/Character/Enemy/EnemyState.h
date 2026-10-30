/**
 * @file EnemyState.h
 * @brief 敵のステート
 */
#pragma once
#include "Source/Core/StateMachineBase.h"
#include "Source/Actor/Character/Enemy/EnemyStateMachine.h"
#include "Object/EnemyParameter.h"


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class EnemyStateMachine;

		/**
		 * @brief 敵のステート
		 */
		class EnemyState : public core::IState
		{
		public:
			explicit EnemyState(EnemyStateMachine& stateMachine) : m_stateMachine(stateMachine) {}
			~EnemyState() override = default;


		protected:
			EnemyStateMachine& m_stateMachine;
		};




		/********************************************/


		/**
		 * @brief 敵の待機ステート
		 */
		class EnemyIdleState : public EnemyState
		{
			appState(EnemyIdleState);

		public:
			explicit EnemyIdleState(EnemyStateMachine& stateMachine) : EnemyState(stateMachine) {}
			~EnemyIdleState() override = default;
		
			void Enter() override;
			void Update() override;
			void Exit() override;
		};




		/********************************************/


		/**
		 * @brief 敵の歩行ステート
		 */
		class EnemyWalkState : public EnemyState
		{
			appState(EnemyWalkState);

		public:
			explicit EnemyWalkState(EnemyStateMachine& stateMachine) : EnemyState(stateMachine) {}
			~EnemyWalkState() override = default;

			void Enter() override;
			void Update() override;
			void Exit() override;
		};




		/*******************************************/


		/**
		 * @brief 敵の警戒ステート
		 */
		class EnemyAlertState : public EnemyState
		{
			appState(EnemyAlertState);

		public:
			explicit EnemyAlertState(EnemyStateMachine& stateMachine) : EnemyState(stateMachine) {}
			~EnemyAlertState() override = default;

			void Enter() override;
			void Update() override;
			void Exit() override;
		};




		/*******************************************/


		/**
		 * @brief 敵の盗むステート
		 * @details 美術品の前で止まって、盗むアニメーションを再生する
		 */
		class EnemyStealState : public EnemyState
		{
			appState(EnemyStealState);

		public:
			explicit EnemyStealState(EnemyStateMachine& stateMachine) : EnemyState(stateMachine) {}
			~EnemyStealState() override = default;

			void Enter() override;
			void Update() override;
			void Exit() override;
		};

	}
}