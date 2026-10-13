#include "stdafx.h"
#include "Enemy.h"


namespace app
{
	namespace
	{
		/** 敵のモデルパス */
		constexpr const char* ENEMY_MODEL_PATH = "Assets/modelData/Enemy.tkm";

		/** 敵の初期位置 */
		const Vector3 ENEMY_POSITION = { 0.0f,0.0f,300.0f };

		/** 敵の初期スケール */
		const Vector3 ENEMY_SCALE = { 2.0f,2.0f,2.0f };
	}


	Enemy::Enemy()
		: m_modelRender(nullptr)
		, m_transform()
	{
		// 初期位置を設定
		m_transform.SetLocalPosition(ENEMY_POSITION);
		// 初期スケールを設定
		m_transform.SetLocalScale(ENEMY_SCALE);
	}


	Enemy::~Enemy()
	{}
	

	void Enemy::Init()
	{
		// ワールド値を確定させてからモデルに反映する
		m_transform.UpdateTransform();

		m_modelRender = std::make_unique<ModelRender>();
		m_modelRender->Init(ENEMY_MODEL_PATH, nullptr, enModelUpAxisY, 0);
		m_modelRender->SetTRS(
			m_transform.GetWorldPosition(),
			m_transform.GetWorldRotation(),
			m_transform.GetWorldScale()
		);

		m_modelRender->Update();
	}
	
	void Enemy::Update()
	{
		// ローカル値からワールド値を計算する(子がいれば子も更新される)
		m_transform.UpdateTransform();

		// ワールド値をモデルに反映
		m_modelRender->SetTRS(
			m_transform.GetWorldPosition(),
			m_transform.GetWorldRotation(),
			m_transform.GetWorldScale()
		);

		m_modelRender->Update();
	}
	
	
	void Enemy::Render(RenderContext & rc)
	{
		m_modelRender->Draw(rc);
	}
}