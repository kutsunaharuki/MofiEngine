/**
 * @file BoyState.h
 * @brief 主人公のステート群
 */
#pragma once
#include "Source/Core/StateMachineBase.h"


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class BoyStateMachine;


		/**
		 * @brief 主人公のステートの基底クラス
		 * @details ステートマシンの共通の処理(移動・重力など)を使うために、ステートマシンを持つ
		 */
		class BoyStateBase : public core::IState
		{
		public:
			explicit BoyStateBase(BoyStateMachine& stateMachine) : m_stateMachine(stateMachine) {}
			~BoyStateBase() override = default;


		protected:
			/** 主人公のステートマシン */
			BoyStateMachine& m_stateMachine;
		};




		/*****************************************************/


		/**
		 * @brief 待機ステート
		 */
		class BoyIdleState : public BoyStateBase
		{
			appState(BoyIdleState);

		public:
			explicit BoyIdleState(BoyStateMachine& stateMachine) : BoyStateBase(stateMachine) {}

			void Enter() override;
			void Update() override;
			void Exit() override;
		};




		/*****************************************************/


		/**
		 * @brief 歩きステート
		 */
		class BoyWalkState : public BoyStateBase
		{
			appState(BoyWalkState);

		public:
			explicit BoyWalkState(BoyStateMachine& stateMachine) : BoyStateBase(stateMachine) {}

			void Enter() override;
			void Update() override;
			void Exit() override;
		};




		/*****************************************************/


		/**
		 * @brief 走りステート
		 */
		class BoyRunState : public BoyStateBase
		{
			appState(BoyRunState);

		public:
			explicit BoyRunState(BoyStateMachine& stateMachine) : BoyStateBase(stateMachine) {}

			void Enter() override;
			void Update() override;
			void Exit() override;
		};




		/*****************************************************/


		/**
		 * @brief ジャンプステート
		 * @details 着地したら、ステートマシンにジャンプの終わりを伝える
		 */
		class BoyJumpState : public BoyStateBase
		{
			appState(BoyJumpState);

		public:
			explicit BoyJumpState(BoyStateMachine& stateMachine) : BoyStateBase(stateMachine), m_elapsedTime(0.0f), m_airMoveSpeed(0.0f) {}

			void Enter() override;
			void Update() override;
			void Exit() override;


		private:
			/** ジャンプしてからの時間 */
			float m_elapsedTime;
			/** 空中で動く速さ(跳んだ瞬間の速さを引き継ぐ) */
			float m_airMoveSpeed;
		};




		/*****************************************************/


		/**
		 * @brief 撮影ステート(ファインダーを覗いている)
		 * @details 移動を止めて、カメラの向いている方向を向く
		 */
		class BoyPhotoState : public BoyStateBase
		{
			appState(BoyPhotoState);

		public:
			explicit BoyPhotoState(BoyStateMachine& stateMachine) : BoyStateBase(stateMachine) {}

			void Enter() override;
			void Update() override;
			void Exit() override;
		};
	}
}
