#pragma once
#include "Source/Core/TransformNode.h"


namespace app
{
	class Enemy
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


	private:
		/** 敵のモデルレンダー */
		std::unique_ptr<ModelRender> m_modelRender;
		/** 敵の位置・回転・スケール */
		core::TransformNode m_transform;
	};
}