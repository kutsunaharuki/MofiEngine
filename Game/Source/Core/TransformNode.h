/**
 * @file TransformNode.h
 * @brief 親子付けのクラス
 */
#pragma once


namespace app 
{
	namespace core
	{
		/**
		 * @brief TransformNodeクラス
		 * @details : 親子付けのクラス
		 */
		class TransformNode : public Noncopyable
		{
		public:
			/** コンストラクタ */
			TransformNode();
			/** デストラクタ */
			~TransformNode();


		public:
			/**
			 * @brief 親と子を繋ぐ
			 * @param parent 親ノード
			 */
			void SetParent(TransformNode* parent);
			/**
			 * @brief 特定の子ノードとの紐づけを外す
			 * @param child 子ノード
			 */
			void RemoveChild(TransformNode* child);
			/**
			 * @brief 全ての子を外す
			 */
			void Release();
			/**
			 * @brief ワールド値を更新
			 */
			void UpdateTransform();
			/**
			 * @brief ワールド行列を更新
			 */
			void UpdateWorldMatrix();


		public:
			/**
			 * @brief ワールド座標を取得
			 * @return ワールド座標
			 * @details : 親
			 */
			const Vector3& GetWorldPosition() { return m_worldPosition; }
			/**
			 * @brief ワールド回転を取得
			 * @return ワールド回転
			 * @details : 親
			 */
			const Quaternion& GetWorldRotation() { return m_worldRotation; }
			/**
			 * @brief ワールドスケールを取得
			 * @return ワールドスケール
			 * @details : 親
			 */
			const Vector3& GetWorldScale() { return m_worldScale; }
			
			/**
			 * @brief ローカル座標を取得
			 * @return ローカル座標
			 * @details : 子
			 */
			const Vector3& GetLocalPosition() { return m_localPosition; }
			/**
			 * @brief ローカル回転を取得
			 * @return ローカル回転
			 * @details : 子
			 */
			const Quaternion& GetLocalRotation() { return m_localRotation; }
			/**
			 * @brief ローカルスケールを取得
			 * @return ローカルスケール
			 * @details : 子
			 */
			const Vector3& GetLocalScale() { return m_localScale; }
			/**
			 * @brief ローカル座標を設定
			 * @param localPosition ローカル座標
			 * @details : 子
			 */
			void SetLocalPosition(const Vector3& localPosition) { m_localPosition = localPosition; }
			/**
			 * @brief ローカル回転を設定
			 * @param localRotation ローカル回転
			 * @details : 子
			 */
			void SetLocalRotation(const Quaternion& localRotation) { m_localRotation = localRotation; }
			/**
			 * @brief ローカルスケールを設定
			 * @param localScale ローカルスケール
			 * @details : 子
			 */
			void SetLocalScale(const Vector3& localScale) { m_localScale = localScale; }


		private:
			/** ワールド行列 */
			Matrix m_worldMatrix;

			/** 親ノード */
			TransformNode* m_parent;
			/** 子のリスト */
			std::vector<TransformNode*> m_children;


			/** 親のワールド座標 */
			Vector3 m_worldPosition;
			/** 親のワールド回転 */
			Quaternion m_worldRotation;
			/** 親のワールドスケール */
			Vector3 m_worldScale;

			/** 子のローカル座標 */
			Vector3 m_localPosition;
			/** 子のローカル回転 */
			Quaternion m_localRotation;
			/** 子のローカルスケール */
			Vector3 m_localScale;
		};
	}
}
