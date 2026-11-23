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




		/*******************************************/


		/**
		 * @brief 敵の見つけたステート
		 * @details 主人公を見つけたら、少しの間止まって主人公の方を向き(頭の上に「！」が出る)、
		 *          その後、主人公の方へ走っていく。近づいたら(または走る時間が終わったら)止まる
		 */
		class EnemyFoundState : public EnemyState
		{
			appState(EnemyFoundState);

		public:
			explicit EnemyFoundState(EnemyStateMachine& stateMachine) : EnemyState(stateMachine), m_timer(0.0f), m_isRunning(false) {}
			~EnemyFoundState() override = default;

			void Enter() override;
			void Update() override;
			void Exit() override;


		private:
			/** 見つけてからの時間(秒) */
			float m_timer;
			/** 走り出したか */
			bool m_isRunning;
		};

	}
}