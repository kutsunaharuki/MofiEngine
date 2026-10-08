#include "stdafx.h"
#include "TransformNode.h"
#include <algorithm>


namespace app
{
	namespace core
	{
		TransformNode::TransformNode()
			: m_worldPosition(Vector3::Zero)
			, m_worldRotation(Quaternion::Identity)
			, m_worldScale(Vector3::One)
			, m_localPosition(Vector3::Zero)
			, m_localRotation(Quaternion::Identity)
			, m_localScale(Vector3::One)
			, m_parent(nullptr)
		{}


		TransformNode::~TransformNode()
		{
			// 親がいれば
			if (m_parent)
			{
				// 親の子リストから自分を外す
				m_parent->RemoveChild(this);
			}

			// 全て子との紐づけを外す
			Release();
		}
		
		
		void TransformNode::SetParent(TransformNode* parent)
		{
			// 既に親がいるなら、古い親から外す
			// 新しい親を見ているとここに入らない
			if (m_parent) m_parent->RemoveChild(this);

			// 親がnullなら親なし
			if (parent == nullptr) return;

			// 新しく親を設定
			m_parent = parent;
			m_parent->m_children.push_back(this);
		}


		void TransformNode::RemoveChild(TransformNode* child)
		{
			auto it = std::find(m_children.begin(), m_children.end(), child);
			
			// 見つからなかったら終了
			if (it == m_children.end()) return;

			// 子の値をnullに
			(*it)->m_parent = nullptr;
			
			// 子を外す
			m_children.erase(it);
		}


		void TransformNode::Release()
		{
			for (auto& children : m_children)
			{
				// 子の親を外す
				children->m_parent = nullptr;
			}

			// 自分の子のリストを空にする
			m_children.clear();
		}
		
		
		void TransformNode::UpdateTransform()
		{
			if (m_parent)
			{
				// ローカル位置
				Matrix localPos;
				localPos.MakeTranslation(m_localPosition);

				Matrix pos;
				// 移動行列と親のワールド行列を掛ける
				pos.Multiply(localPos, m_parent->m_worldMatrix);

				// 行列の平行移動成分(ワールド座標)を取り出す
				m_worldPosition.x = pos.m[3][0];
				m_worldPosition.y = pos.m[3][1];
				m_worldPosition.z = pos.m[3][2];

				// スケール
				m_worldScale.x = m_localScale.x * m_parent->m_worldScale.x;
				m_worldScale.y = m_localScale.y * m_parent->m_worldScale.y;
				m_worldScale.z = m_localScale.z * m_parent->m_worldScale.z;

				// 回転
				m_worldRotation = m_parent->m_worldRotation * m_localRotation;
			}
			else
			{
				// 親がいない場合はローカル = ワールド
				m_worldPosition = m_localPosition;
				m_worldRotation = m_localRotation;
				m_worldScale = m_localScale;
			}
			
			// ワールド行列更新
			UpdateWorldMatrix();
		}


		void TransformNode::UpdateWorldMatrix()
		{
			Matrix scal, rot ,pos, world;

			// ワールドスケールからスケール行列(拡大行列)を作成
			scal.MakeScaling(m_worldScale);
			// ワールド回転から回転行列を作成
			rot.MakeRotationFromQuaternion(m_worldRotation);
			// ワールド位置から移動行列を作成
			pos.MakeTranslation(m_worldPosition);

			// スケール * 回転
			world.Multiply(scal, rot);
			// スケール * 回転 * 移動
			m_worldMatrix.Multiply(world, pos);

			// 子の更新
			for (TransformNode* child : m_children)
			{
				child->UpdateTransform();
			}
		}
	}
}