/**
 * @file VisitorStateMachine.h
 * @brief 訪問者(一般客)のステートマシン
 */
#pragma once
#include "Source/Actor/Character/CharacterStateMachine.h"
#include "Visitor.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief 訪問者(一般客)のステートマシン
		 * @details どこへ動くか・話すかはグループ(VisitorGroup)が決めて、Visitor に入力として渡す。
		 *          ステートマシンはその入力を見て、ステートを選んで動かす
		 */
		class VisitorStateMachine : public CharacterStateMachine
		{
		public:
			explicit VisitorStateMachine(Visitor* visitorOwner);
			~VisitorStateMachine() override;


			/**
			 * @brief アニメーションを設定
			 * @param animation 訪問者(一般客)のアニメーション
			 * @param interpolationTime 補間時間(単位 / 秒)
			 */
			void PlayAnimation(Visitor::VisitorAnimation animation, const float interpolationTime = 0.2f)
			{
				CharacterStateMachine::PlayAnimation(static_cast<int>(animation), interpolationTime);
			}


		public:
			/**
			 * @brief 入力された方向へ動かす
			 * @param speed 移動速度(歩く・歩きながら話すで変える)
			 */
			void Move(const float speed);
			/**
			 * @brief 横の移動を止める
			 */
			void StopMove();
			/**
			 * @brief 止まっている時に、向く方向(美術品の方)へ向きを変える
			 */
			void FaceLookDirection();
			/**
			 * @brief 指定した方向に回転する
			 * @param direction 回転する方向
			 */
			void RotateToward(const Vector3& direction);
			/**
			 * @brief 訪問者の調整パラメーターを取得
			 * @return 調整パラメーター(種類ごとの JSON の値)
			 */
			const VisitorParameter& GetParameter() { return m_visitorOwner->GetParameter(); }


		protected:
			/**
			 * @brief ステートの変更先を取得
			 * @return 変更先のステート
			 */
			core::IState* GetChangeState() override;


		private:
			/** 訪問者(一般客)のオーナー */
			Visitor* m_visitorOwner;
			/** 回転 */
			Quaternion m_rotation;
		};
	}
}
