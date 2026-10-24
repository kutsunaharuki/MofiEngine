/**
 * @file StageObject.cpp
 * @brief ステージオブジェクトの基底クラス
 */
#include "stdafx.h"
#include "StageObject.h"
#include "imgui.h"
#include "ImGuizmo.h"
#include <cfloat>


namespace app
{
	namespace actor
	{
		StageObject::StageObject()
			: m_rotationDeg(Vector3::Zero)
			, m_isCollision(false)
			, m_triangleCount(0)
			, m_boundingCenter(Vector3::Zero)
			, m_physicsObject(nullptr)
			, m_boundingRadius(0.0f)
		{}


		StageObject::~StageObject()
		{}


		void StageObject::Initialize(
			const std::string& name,
			const std::string& modelPath,
			const Vector3& position,
			const Vector3& rotationDeg,
			const Vector3& scale,
			const bool isCollision)
		{
			m_name = name;
			m_modelPath = modelPath;
			m_isCollision = isCollision;

			SetPosition(position);
			SetRotationDeg(rotationDeg);
			SetScale(scale);

			// 反転(スケールがマイナス)していると、ポリゴンの表と裏が入れ替わるので、表側を描かない設定にする
			// ※ モデルを作る時にしか決められないので、あとで反転の有無が変わったら、読み直す必要がある
			const D3D12_CULL_MODE cullMode = IsMirrored(scale) ? D3D12_CULL_MODE_FRONT : D3D12_CULL_MODE_BACK;

			// 置物はアニメーションしない。影は落とさず受けるだけ(地面と同じ設定)
			m_modelRender.Init(m_modelPath.c_str(), nullptr, enModelUpAxisZ, 0, false, true, cullMode);


			/** 1. ローカルのボックス(AABBB)を作る */
			// モデルを囲むボックスを作る
			// floatの最大値で初期化
			Vector3 vMin(FLT_MAX, FLT_MAX, FLT_MAX);
			// floatの最大値の負で初期化
			Vector3 vMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);

			// モデルメッシュを1つずつ見る
			for (const auto& mesh : m_modelRender.GetModel().GetTkmFile().GetMeshParts())
			{
				// 頂点バッファの中の頂点を1つずつ見る
				for (const auto& vertex : mesh.vertexBuffer)
				{
					// Windows.h の min/max はマクロで定義されているため括弧で囲む
					vMin.x = (std::min)(vMin.x, vertex.pos.x);
					vMin.y = (std::min)(vMin.y, vertex.pos.y);
					vMin.z = (std::min)(vMin.z, vertex.pos.z);
					vMax.x = (std::max)(vMax.x, vertex.pos.x);
					vMax.y = (std::max)(vMax.y, vertex.pos.y);
					vMax.z = (std::max)(vMax.z, vertex.pos.z);
				}
			}

			// ローカルボックスを初期化する
			// 頂点の座標は「モデルの座標系」なので、置いた場所や向きはまだ入っていないので、
			// 1回作れば、あとは使いまわせる
			m_localAABB.Init(vMax, vMin);


			// ポリゴンの数を数える(インデックス3つで三角形1枚。マテリアルごとにインデックスバッファが分かれている)
			m_triangleCount = 0;
			for (const auto& mesh : m_modelRender.GetModel().GetTkmFile().GetMeshParts())
			{
				for (const auto& indexBuffer : mesh.indexBuffer16Array)
				{
					m_triangleCount += static_cast<int>(indexBuffer.indices.size() / 3);
				}
				for (const auto& indexBuffer : mesh.indexBuffer32Array)
				{
					m_triangleCount += static_cast<int>(indexBuffer.indices.size() / 3);
				}
			}
		}


		bool StageObject::Start()
		{
			RebuildCollision();
			return true;
		}


		void StageObject::Update()
		{
			ApplyTransform();
		}


		void StageObject::Render(RenderContext& rc)
		{
			m_modelRender.Draw(rc);
		}


		void StageObject::RebuildCollision()
		{
			// 前のコリジョンを消す(RigidBody のデストラクタで、物理ワールドから外れる)
			m_physicsObject.reset();

			if (!m_isCollision) return;

			// ワールド行列が決まってから作る
			ApplyTransform();

			Model& model = m_modelRender.GetModel();
			m_physicsObject = std::make_unique<PhysicsStaticObject>();
			m_physicsObject->CreateFromModel(model, model.GetWorldMatrix());
		}


		void StageObject::SetRotationDeg(const Vector3& rotationDeg)
		{
			m_rotationDeg = rotationDeg;
			SetRotation(ToQuaternion(m_rotationDeg));
		}


		bool StageObject::IsMirrored(const Vector3& scale)
		{
			// マイナスの軸が奇数本なら反転している(2本なら、180度回したのと同じなので反転ではない)
			return scale.x * scale.y * scale.z < 0.0f;
		}


		Quaternion StageObject::ToQuaternion(const Vector3& rotationDeg)
		{
			// 回転だけの行列を作ってから、Quaternion にする
			const float translation[3] = { 0.0f, 0.0f, 0.0f };
			const float scale[3] = { 1.0f, 1.0f, 1.0f };
			Matrix rotationMatrix;
			ImGuizmo::RecomposeMatrixFromComponents(translation, rotationDeg.v, scale, &rotationMatrix.m[0][0]);

			Quaternion rotation;
			rotation.SetRotation(rotationMatrix);
			return rotation;
		}


		Vector3 StageObject::ToRotationDeg(const Quaternion& rotation)
		{
			Matrix rotationMatrix;
			rotationMatrix.MakeRotationFromQuaternion(rotation);

			float translation[3];
			float scale[3];
			Vector3 rotationDeg;
			ImGuizmo::DecomposeMatrixToComponents(&rotationMatrix.m[0][0], translation, rotationDeg.v, scale);
			return rotationDeg;
		}


		void StageObject::ApplyTransform()
		{
			// ワールド値を更新して、モデルに反映する
			m_transform.UpdateTransform();
			m_modelRender.SetTRS(
				m_transform.GetWorldPosition(),
				m_transform.GetWorldRotation(),
				m_transform.GetWorldScale()
			);
			m_modelRender.Update();

			// ワールド行列が変わったので、囲む球を計算し直す
			UpdateBoundingSphere();
		}


		void StageObject::UpdateBoundingSphere()
		{
			// ボックスの8つの角をワールド座標ににする
			// ※ GetWorldMatrix には Z-up の回転・スケール・回転・位置が全部入っている
			Vector3 corners[8];
			// 8つの角の座標を計算する
			m_localAABB.CalcVertexPositions(corners, m_modelRender.GetModel().GetWorldMatrix());

			// 中心 = 8つの角の平均
			m_boundingCenter = Vector3::Zero;
			for (const Vector3& corner : corners)
			{
				m_boundingCenter.Add(corner);
			}
			// 8つの角の平均なので8で割る(ベクトルをスカラーで拡大)
			m_boundingCenter.Scale(1.0f / 8.0f);


			// 半径 = 中心から一番遠い角までの距離
			m_boundingRadius = 0.0f;
			for (const Vector3 corner : corners)
			{
				Vector3 toCorner = corner;
				toCorner.Subtract(m_boundingCenter);
				m_boundingRadius = (std::max)(m_boundingRadius, toCorner.Length());
			}
		}
	}
}
