/**
 * @file ActorStateMachine.h
 * @brief アクターのステートマシン
 */
#pragma once
#include "Actor.h"
#include "Source/Core/StateMachineBase.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief アクターのステートマシン
		 */
		class ActorStateMachine : public app::core::StateMachineBase
		{
		public:
			/**
			 * @brief アニメーションの再生速度を設定
			 * @param speed アニメーション速度
			 */
			inline void SetAnimationSpeed(float speed)
			{
				m_owner->SetAnimationSpeed(speed);
			}
			/**
			 * @brief アニメーション再生中かどうか
			 * @retunr 再生中ならtrue
			 */
			inline bool IsPlayingAnimation() const
			{
				return m_owner->GetModelRender()->IsPlayingAnimation();
			}
			
			/**
			 * @brief トランスフォームを取得
			 * @return トランスフォームの参照
			 */
			inline const core::TransformNode& GetTransform() const 
			{
				return m_owner->GetTransform();
			}
			/**
			 * @brief 位置を設定
			 * @param pos 位置
			 * @details トランスフォームのローカル座標は親がいないなら
			 * ワールドと同じになるのでローカルで設定する
			 */
			inline void SetPosition(const Vector3& pos)
			{
				m_owner->SetPosition(pos);
			}
			/**
			 * @brief 回転を設定
			 * @param rot 回転
			 */
			inline void SetRotation(const Quaternion& rot)
			{
				m_owner->SetRotation(rot);
			}
			/**
			 * @brief スケールを設定
			 * @param scale スケール
			 */
			inline void SetScale(const Vector3& scale)
			{
				m_owner->SetScale(scale);
			}


		public:
			/**
			 * @brief ステートの変更先を取得
			 * @return 変更先のステートポインタ
			 */
			virtual app::core::IState* GetChangeState() override;


		public:
			explicit ActorStateMachine(Actor* owner);
			virtual ~ActorStateMachine() override = default;


		protected:
			/** アクターのオーナー */
			Actor* m_owner;
		};
	}
}

