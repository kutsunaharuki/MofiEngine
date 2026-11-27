/**
 * @file BoyState.cpp
 * @brief 主人公のステート群
 */
#include "stdafx.h"
#include "BoyState.h"
#include "BoyStateMachine.h"
#include <algorithm>
#include "Object/BoyParameter.h"


namespace app
{
	namespace actor
	{
		/*************************************/
		// カメラを構えていないときの待機ステート
		/*************************************/


		void BoyIdleState::Enter()
		{
			m_stateMachine.PlayAnimation(EnBoyAnimation::Idle);
		}


		void BoyIdleState::Update()
		{
			m_stateMachine.StopMove();
		}


		void BoyIdleState::Exit()
		{}




		/*************************************/
		// カメラを構えていないときの歩行ステート
		/*************************************/


		void BoyWalkState::Enter()
		{
			m_stateMachine.PlayAnimation(EnBoyAnimation::Walk);
		}


		void BoyWalkState::Update()
		{
			const BoyParameter& parameter = m_stateMachine.GetParameter();
			m_stateMachine.Move(parameter.walkSpeed);
		}


		void BoyWalkState::Exit()
		{}




		/*************************************/
		// カメラを構えていないときの走行ステート
		/*************************************/


		void BoyRunState::Enter()
		{
			m_stateMachine.PlayAnimation(EnBoyAnimation::Run);
		}


		void BoyRunState::Update()
		{
			const BoyParameter& parameter = m_stateMachine.GetParameter();
			m_stateMachine.Move(parameter.runSpeed);
		}


		void BoyRunState::Exit()
		{}




		/*************************************/
		// カメラを構えている時の待機ステート
		/*************************************/


		void BoyPhotoIdleState::Enter()
		{
			m_stateMachine.PlayAnimation(EnBoyAnimation::PhotoIdle);
		}


		void BoyPhotoIdleState::Update()
		{
			// 動かずに、カメラの向いている方向を向く
			m_stateMachine.StopMove();
			m_stateMachine.FaceCameraForward();
		}


		void BoyPhotoIdleState::Exit()
		{}




		/*************************************/
		// カメラを構えている時の前歩きステート
		/*************************************/


		void BoyPhotoWalkForwardState::Enter()
		{
			m_stateMachine.PlayAnimation(EnBoyAnimation::PhotoWalkForward);
		}


		void BoyPhotoWalkForwardState::Update()
		{
			m_stateMachine.SlowMove();
		}


		void BoyPhotoWalkForwardState::Exit()
		{}




		/*************************************/
		// カメラを構えている時の後ろ歩きステート
		/*************************************/


		void BoyPhotoWalkBackState::Enter()
		{
			m_stateMachine.PlayAnimation(EnBoyAnimation::PhotoWalkBack);
		}


		void BoyPhotoWalkBackState::Update()
		{
			m_stateMachine.SlowMove();
		}


		void BoyPhotoWalkBackState::Exit()
		{}




		/*************************************/
		// カメラを構えている時の左歩きステート
		/*************************************/


		void BoyPhotoWalkLeftState::Enter()
		{
			m_stateMachine.PlayAnimation(EnBoyAnimation::PhotoWalkLeft);
		}


		void BoyPhotoWalkLeftState::Update()
		{
			m_stateMachine.SlowMove();
		}


		void BoyPhotoWalkLeftState::Exit()
		{}




		/*************************************/
		// カメラを構えている時の右歩きステート
		/*************************************/


		void BoyPhotoWalkRightState::Enter()
		{
			m_stateMachine.PlayAnimation(EnBoyAnimation::PhotoWalkRight);
		}


		void BoyPhotoWalkRightState::Update()
		{
			m_stateMachine.SlowMove();
		}


		void BoyPhotoWalkRightState::Exit()
		{}




		/*************************************/
		// しゃがんで止まっているステート
		/*************************************/


		void BoyCrouchIdleState::Enter()
		{
			m_stateMachine.PlayAnimation(EnBoyAnimation::CrouchHide);
		}


		void BoyCrouchIdleState::Update()
		{
			// しゃがんだまま止まる(向きは今のまま)
			m_stateMachine.StopMove();
		}


		void BoyCrouchIdleState::Exit()
		{}




		/*************************************/
		// しゃがんだまま歩くステート
		/*************************************/


		void BoyCrouchWalkState::Enter()
		{
			m_stateMachine.PlayAnimation(EnBoyAnimation::CrouchWalk);
		}


		void BoyCrouchWalkState::Update()
		{
			// ふつうに歩くより遅く、歩く方向を向いて進む
			const BoyParameter& parameter = m_stateMachine.GetParameter();
			m_stateMachine.Move(parameter.crouchWalkSpeed);
		}


		void BoyCrouchWalkState::Exit()
		{}




		/*************************************/
		// しゃがんでカメラを構えるステート
		/*************************************/


		void BoyCrouchCameraState::Enter()
		{
			m_stateMachine.PlayAnimation(EnBoyAnimation::CrouchCamera);
		}


		void BoyCrouchCameraState::Update()
		{
			// 動かずに、カメラの向いている方向を向く
			m_stateMachine.StopMove();
			m_stateMachine.FaceCameraForward();
		}


		void BoyCrouchCameraState::Exit()
		{}




		/*************************************/
		// しゃがんでカメラを構えたまま歩くステート
		/*************************************/


		void BoyCrouchPhotoWalkState::Enter()
		{
			// しゃがんで構えたまま歩くアニメーションは無いので、しゃがみ歩きを使う
			m_stateMachine.PlayAnimation(EnBoyAnimation::CrouchWalk);
		}


		void BoyCrouchPhotoWalkState::Update()
		{
			// カメラの方を向いたまま、しゃがみ歩きの速さまでで動く
			const BoyParameter& parameter = m_stateMachine.GetParameter();
			m_stateMachine.SlowMove(parameter.crouchWalkSpeed);
		}


		void BoyCrouchPhotoWalkState::Exit()
		{}




		/*************************************/
		// 柱からのぞいてカメラを構えるステート
		/*************************************/


		namespace
		{
			/**
			 * @brief のぞいている時の、毎フレームの処理(左右で同じ)
			 * @param stateMachine ステートマシン
			 * @param isCrouchAnimation しゃがんでのぞくアニメーションを再生しているか(変わったら書きかえる)
			 * @param standAnimation 立ってのぞくアニメーション
			 * @param crouchAnimation しゃがんでのぞくアニメーション
			 */
			void UpdateLean(BoyStateMachine& stateMachine, bool& isCrouchAnimation, const EnBoyAnimation standAnimation, const EnBoyAnimation crouchAnimation)
			{
				// 動かずに、カメラの向いている方向を向く
				stateMachine.StopMove();
				stateMachine.FaceCameraForward();

				// のぞいている間に R3 でしゃがむ・立つを切りかえたら、アニメーションも切りかえる
				const bool isCrouching = stateMachine.IsCrouching();
				if (isCrouching == isCrouchAnimation) return;
				isCrouchAnimation = isCrouching;
				stateMachine.PlayAnimation(isCrouching ? crouchAnimation : standAnimation);
			}
		}


		// ※ アニメーションの名前の左右は、のぞく向きと反対(LeanCameraRight が、左からのぞく動き)
		void BoyLeanLeftState::Enter()
		{
			m_isCrouchAnimation = m_stateMachine.IsCrouching();
			m_stateMachine.PlayAnimation(m_isCrouchAnimation ? EnBoyAnimation::CrouchLeanCameraRight : EnBoyAnimation::LeanCameraRight);
		}


		void BoyLeanLeftState::Update()
		{
			UpdateLean(m_stateMachine, m_isCrouchAnimation, EnBoyAnimation::LeanCameraRight, EnBoyAnimation::CrouchLeanCameraRight);
		}


		void BoyLeanLeftState::Exit()
		{}


		void BoyLeanRightState::Enter()
		{
			m_isCrouchAnimation = m_stateMachine.IsCrouching();
			m_stateMachine.PlayAnimation(m_isCrouchAnimation ? EnBoyAnimation::CrouchLeanCameraLeft : EnBoyAnimation::LeanCameraLeft);
		}


		void BoyLeanRightState::Update()
		{
			UpdateLean(m_stateMachine, m_isCrouchAnimation, EnBoyAnimation::LeanCameraLeft, EnBoyAnimation::CrouchLeanCameraLeft);
		}


		void BoyLeanRightState::Exit()
		{}
    }
}
