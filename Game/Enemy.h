#pragma once
#include "Source/Core/TransformNode.h"
#include "Source/Camera/PhotoSubject.h"


namespace app
{
	class Enemy : public camera::IPhotoSubject
	{
	public:

		Enemy();
		~Enemy();

		void Init();
		void Update();
		void Render(RenderContext& rc);


	public:
		/**
		 * @brief TransformNodeを取得する
		 * @return TransformNode
		 */
		core::TransformNode& GetTransform() { return m_transform; }

		/**
		 * @brief 敵の位置を設定する
		 * @param position 位置
		 */
		void SetPosition(const Vector3& position)
		{
			m_transform.SetLocalPosition(position);
		}
		/**
		 * @brief 敵の位置を取得する
		 * @return 敵の位置
		 */
		const Vector3& GetPosition()
		{
			return m_transform.GetWorldPosition();
		}
		/**
		 * @brief 敵のスケールを設定する
		 * @param scale スケール
		 */
		void SetScale(const Vector3& scale)
		{
			m_transform.SetLocalScale(scale);
		}
		/**
		 * @brief 敵の回転を設定する
		 * @param rotation 回転
		 */
		void SetRotation(const Quaternion& rotation)
		{
			m_transform.SetLocalRotation(rotation);
		}


		// 敵のアニメーション
		enum class EnemyAnimationState : uint8_t
		{
			Idle,      // 待機
			Max
		};


	public:
		/**
		 * @brief 敵の現在のステートを取得
		 */
		EnemyAnimationState& GetCurrentState() { return m_currentState; }

		/**
		 * @brief 敵に被写体のピントを合わせる位置を取得
		 * @return 被写体のピントを合わせる位置(定義した場所で体の中心や顔のあたりの位置が変わる)
		 */
		Vector3 GetFocusPosition() const override;


	private:
		/** 敵のモデルレンダー */
		std::unique_ptr<ModelRender> m_modelRender;
		/** 敵の位置・回転・スケール */
		core::TransformNode m_transform;
		/** 現在のステート */
		EnemyAnimationState m_currentState;
		/** アニメーションクリップ */
		std::unique_ptr<AnimationClip> m_animationClip;
		/** 敵のアニメーションのリスト */
		std::vector<std::unique_ptr<AnimationClip[]>> m_enemyAnimationClipList;
	};
}