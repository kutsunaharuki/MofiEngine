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
		 * @brief カメラを構えていない走るステート
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




		/****************************************************/


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
		 * @brief しゃがんで止まっているステート
		 * @details R3 を押すと、しゃがむ・立つが切りかわる
		 */
		class BoyCrouchIdleState : public BoyStateBase
		{
			appState(BoyCrouchIdleState);

		public:
			explicit BoyCrouchIdleState(BoyStateMachine& stateMachine) : BoyStateBase(stateMachine) {}

			void Enter() override;
			void Update() override;
			void Exit() override;
		};




		/*****************************************************/


		/**
		 * @brief しゃがんだまま歩くステート
		 */
		class BoyCrouchWalkState : public BoyStateBase
		{
			appState(BoyCrouchWalkState);

		public:
			explicit BoyCrouchWalkState(BoyStateMachine& stateMachine) : BoyStateBase(stateMachine) {}

			void Enter() override;
			void Update() override;
			void Exit() override;
		};




		/*****************************************************/


		/**
		 * @brief しゃがんでカメラを構えるステート(柱の近くではない時)
		 */
		class BoyCrouchCameraState : public BoyStateBase
		{
			appState(BoyCrouchCameraState);

		public:
			explicit BoyCrouchCameraState(BoyStateMachine& stateMachine) : BoyStateBase(stateMachine) {}

			void Enter() override;
			void Update() override;
			void Exit() override;
		};




		/*****************************************************/


		/**
		 * @brief しゃがんでカメラを構えたまま歩くステート
		 * @details 撮影中の歩きと同じく、カメラの方を向いたまま、ゆっくり動く
		 */
		class BoyCrouchPhotoWalkState : public BoyStateBase
		{
			appState(BoyCrouchPhotoWalkState);

		public:
			explicit BoyCrouchPhotoWalkState(BoyStateMachine& stateMachine) : BoyStateBase(stateMachine) {}

			void Enter() override;
			void Update() override;
			void Exit() override;
		};




		/*****************************************************/


		/**
		 * @brief 柱の左からのぞいて、カメラを構えるステート
		 * @details しゃがんでいれば、しゃがんだままのぞく(R3 で切りかえると、アニメーションも変わる)
		 */
		class BoyLeanLeftState : public BoyStateBase
		{
			appState(BoyLeanLeftState);

		public:
			explicit BoyLeanLeftState(BoyStateMachine& stateMachine) : BoyStateBase(stateMachine), m_isCrouchAnimation(false) {}

			void Enter() override;
			void Update() override;
			void Exit() override;

		private:
			/** しゃがんでのぞくアニメーションを再生しているか */
			bool m_isCrouchAnimation;
		};




		/*****************************************************/


		/**
		 * @brief 柱の右からのぞいて、カメラを構えるステート
		 * @details しゃがんでいれば、しゃがんだままのぞく(R3 で切りかえると、アニメーションも変わる)
		 */
		class BoyLeanRightState : public BoyStateBase
		{
			appState(BoyLeanRightState);

		public:
			explicit BoyLeanRightState(BoyStateMachine& stateMachine) : BoyStateBase(stateMachine), m_isCrouchAnimation(false) {}

			void Enter() override;
			void Update() override;
			void Exit() override;

		private:
			/** しゃがんでのぞくアニメーションを再生しているか */
			bool m_isCrouchAnimation;
		};
	}
}
