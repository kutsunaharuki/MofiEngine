/**
 * @file StageObject.cpp
 * @brief ステージオブジェクトの基底クラス
 */
#include "stdafx.h"
#include "StageObject.h"
// ImGuizmo.h は imgui.h を先に読み込む必要がある
#include "imgui.h"
#include "ImGuizmo.h"


namespace app
{
	namespace actor
	{
		StageObject::StageObject()
			: m_rotationDeg(Vector3::Zero)
			, m_isCollision(false)
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

			// 置物はアニメーションしない。影は落とさず受けるだけ(地面と同じ設定)
			m_modelRender.Init(m_modelPath.c_str(), nullptr, enModelUpAxisZ, 0, false, true);
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
		}
	}
}
