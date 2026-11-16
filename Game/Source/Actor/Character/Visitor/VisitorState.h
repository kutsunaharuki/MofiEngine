/**
 * @file VisitorState.h
 * @brief 訪問者(一般客)のステート
 */
#pragma once
#include "Source/Core/StateMachineBase.h"


namespace app
{
	namespace actor 
	{
		/** 前方宣言 */
		class VisitorStateMachine;


		/**
		 * @brief 訪問者(一般客)のステート
		 */
		class VisitorState : public core::IState
		{
		public:
			explicit VisitorState(VisitorStateMachine& stateMachine) : m_stateMachine(stateMachine) {}
			~VisitorState() override = default;


		protected:
			/** 訪問者(一般客)のステートマシン */
			VisitorStateMachine& m_stateMachine;
		};




		/******************************************************/


		/**
		 * @brief 訪問者(一般客)の待機ステート
		 */
		class VisitorIdleState : public VisitorState
		{
			appState(VisitorIdleState);

		public:
			explicit VisitorIdleState(VisitorStateMachine& stateMachine) : VisitorState(stateMachine) {}
			~VisitorIdleState() override = default;

			void Enter() override;
			void Update() override;
			void Exit() override;
		};




		/*****************************************************/


		/**
		 * @brief 訪問者(一般客)の歩行ステート
		 */
		class VisitorWalkState : public VisitorState
		{
			appState(VisitorWalkState);

		public:
			explicit VisitorWalkState(VisitorStateMachine& stateMachine) : VisitorState(stateMachine) {}
			~VisitorWalkState() override = default;

			void Enter() override;
			void Update() override;
			void Exit() override;
		};




		/****************************************************/


		/**
		 * @brief 訪問者(一般客)のその場で話すステート
		 */
		class VisitorTalkState : public VisitorState
		{
			appState(VisitorTalkState);

		public:
			explicit VisitorTalkState(VisitorStateMachine& stateMachine) : VisitorState(stateMachine) {}
			~VisitorTalkState() override = default;

			void Enter() override;
			void Update() override;
			void Exit() override;
		};




		/*********************************************************************/


		/**
		 * @brief 訪問者(一般客)の歩きながら話すステート
		 */
		class VisitorWalkTalkState : public VisitorState
		{
			appState(VisitorWalkTalkState);

		public:
			explicit VisitorWalkTalkState(VisitorStateMachine& stateMachine) : VisitorState(stateMachine) {}
			~VisitorWalkTalkState() override = default;

			void Enter() override;
			void Update() override;
			void Exit() override;
		};
	}
}