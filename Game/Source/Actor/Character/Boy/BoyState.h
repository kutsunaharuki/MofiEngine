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
		 * @details ステートマシンの共通の処理を使うために、ステートマシンを持つ
		 */
		class BoyStateBase : public core::IState
		{
		public:
			/**
			 * @brief コンストラクタ
			 * @param stateMachine ステートマシン
			 */
			explicit BoyStateBase(BoyStateMachine& stateMachine) : m_stateMachine(stateMachine) {}
			~BoyStateBase() override = default;


		protected:
			/** 主人公のステートマシン */
			BoyStateMachine& m_stateMachine;
		};




		/*****************************************************/


		/**
		 * @brief カメラを構えていない待機ステート
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
		 * @brief カメラを構えていない歩行ステート
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
		 * @brief カメラを構えながらの待機ステート
		 */
		class BoyPhotoIdleState : public BoyStateBase
		{
			appState(BoyPhotoIdleState);

		public:
			explicit BoyPhotoIdleState(BoyStateMachine& stateMachine) : BoyStateBase(stateMachine) {}

			void Enter() override;
			void Update() override;
			void Exit() override;
		};




		/*****************************************************/


		/**
		 * @brief カメラを構えながら前に歩くステート
		 */
		class BoyPhotoWalkForwardState : public BoyStateBase
		{
			appState(BoyPhotoWalkForwardState);

		public:
			explicit BoyPhotoWalkForwardState(BoyStateMachine& stateMachine) : BoyStateBase(stateMachine) {}

			void Enter() override;
			void Update() override;
			void Exit() override;
		};




		/*****************************************************/


		/**
		 * @brief カメラを構えながら後ろに歩くステート
		 */
		class BoyPhotoWalkBackState : public BoyStateBase
		{
			appState(BoyPhotoWalkBackState);

		public:
			explicit BoyPhotoWalkBackState(BoyStateMachine& stateMachine) : BoyStateBase(stateMachine) {}

			void Enter() override;
			void Update() override;
			void Exit() override;
		};




		/*****************************************************/


		/**
		 * @brief カメラを構えながら左に歩くステート
		 */
		class BoyPhotoWalkLeftState : public BoyStateBase
		{
			appState(BoyPhotoWalkLeftState);

		public:
			explicit BoyPhotoWalkLeftState(BoyStateMachine& stateMachine) : BoyStateBase(stateMachine) {}

			void Enter() override;
			void Update() override;
			void Exit() override;
		};




		/*****************************************************/


		/**
		 * @brief カメラを構えながら右に歩くステート
		 */
		class BoyPhotoWalkRightState : public BoyStateBase
		{
			appState(BoyPhotoWalkRightState);

		public:
			explicit BoyPhotoWalkRightState(BoyStateMachine& stateMachine) : BoyStateBase(stateMachine) {}

			void Enter() override;
			void Update() override;
			void Exit() override;
		};



		/*****************************************************/


		/**
		 * @brief 小さい物陰に隠れるステート
		 * @details 屈んで、隠れている物の方を向く
		 */
		class BoyLowHideState : public BoyStateBase
		{
			appState(BoyLowHideState);

		public:
			explicit BoyLowHideState(BoyStateMachine& stateMachine) : BoyStateBase(stateMachine) {}

			void Enter() override;
			void Update() override;
			void Exit() override;
		};




		/*****************************************************/

		
		/**
		 * @brief 高い物陰に隠れるステート
		 * @details 立ったまま、壁を背にする
		 */
		class BoyHighHideState : public BoyStateBase
		{
			appState(BoyHighHideState);

		public:
			explicit BoyHighHideState(BoyStateMachine& stateMachine) : BoyStateBase(stateMachine) {}

			void Enter() override;
			void Update() override;
			void Exit() override;
		};
	}
}
