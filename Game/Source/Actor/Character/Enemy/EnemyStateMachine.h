/**
 * @file EnemyStateMachine.h
 * @brief 敵のステートマシン
 */
#pragma once
#include "Source/Actor/Character/CharacterStateMachine.h"
#include "Source/Actor/Character/Enemy/EnemyBase.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief 敵のステートマシン
		 */
		class EnemyStateMachine : public CharacterStateMachine
		{
		public:
			explicit EnemyStateMachine(EnemyBase* enemyOwner);
			~EnemyStateMachine() override = default;


		public:
			/**
			 * @brief アニメーションを再生
			 * @param animation 再生するアニメーション
			 * @param interpolateTime 補間時間(/s(秒))
			 */
			void PlayAnimation(EnemyBase::EnemyAnimation animation, const float interpolateTime = 0.2f)
			{
				CharacterStateMachine::PlayAnimation(static_cast<int>(animation), interpolateTime);
			}

			/**
			 * @brief 移動方向を設定
			 * @param direction 移動方向
			 */
			inline void SetMoveDirection(const Vector3& direction) { m_moveDirection = direction; }
			/**
			 * @brief 警戒状態かどうかを設定
			 * @param isAlert 警戒状態かどうか
			 */
			inline void SetAlert(bool isAlert) { m_isAlert = isAlert; }
			/**
			 * @brief 移動量を設定
			 * @param amount 移動量
			 */
			inline void SetMoveAmount(const float amount) { m_moveAmount = amount; }

			/**
			 * @brief 移動制御
			 */
			void Move(const float speed);
			/**
			 * @brief 横の移動を止める
			 */
			void StopMove();
			/**
			 * @brief 指定した方向に回転する
			 * @param direction 回転する方向
			 */
			void RotateToward(const Vector3& direction);


		public:
			/**
			 * @brief 敵の調整パラメーターを取得
			 * @return 調整パラメーター(種類ごとの JSON の値)
			 */
			const EnemyParameter& GetParameter() { return m_enemyOwner->GetParameter(); }


		protected:
			/**
			 * @biref ステートの変更先を取得
			 * @return 変更先のステート
			 */
			core::IState* GetChangeState() override;


		private:
			/** 敵のオーナー */
			EnemyBase* m_enemyOwner;
			/** 移動量 */
			float m_moveAmount;
			/** 移動方向 */
			Vector3 m_moveDirection;
			/** 警戒状態かどうか */
			bool m_isAlert;
			/** 回転 */
			Quaternion m_rotation;
		};
	}
}

