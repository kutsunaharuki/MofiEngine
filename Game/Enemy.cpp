#include "stdafx.h"
#include "Enemy.h"


namespace app
{
	namespace
	{
		/**
		 * @brief 敵のモデルパス
		 * @details スケルトン(.tks)は、.tkm と同じフォルダーの同じ名前のものが読まれる(ModelRender::Init)。
		 *          テクスチャも .tkm と同じフォルダーから読まれるので、3つとも同じフォルダーに置く
		 */
		constexpr const char* ENEMY_MODEL_PATH = "Assets/modelData/Enemy/BigMan/Enemy.tkm";
		/** 敵の初期位置 */
		const Vector3 ENEMY_POSITION = { 0.0f,0.0f,300.0f };
		/** 敵の初期スケール */
		const Vector3 ENEMY_SCALE = { 0.5f,0.5f,0.5f };
		/** 敵のアニメーションクリップ数(インデックスで使用) */
		const int ENEMY_ANIMATION_CLIP_NUM = static_cast<int>(Enemy::EnemyAnimationState::Max);
		/** ピントを合わせる高さ(足元から。体の中心 ~ 顔のあたり) */
		constexpr float ENEMY_FOCUS_POSITION_Y = 80.0f;
	}


	Enemy::Enemy()
		: m_modelRender(nullptr)
		, m_transform()
		, m_currentState(EnemyAnimationState::Idle)
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
		
		// モデルレンダーの初期化
		m_modelRender = std::make_unique<ModelRender>();
		
		// 敵のアニメーションの初期化
		m_animationClip = std::make_unique<AnimationClip>();
		m_animationClip->Load("Assets/animData/Enemy/BigMan/idle.tka");
		m_animationClip->SetLoopFlag(true);
		
		m_modelRender->Init(ENEMY_MODEL_PATH, m_animationClip.get(),enModelUpAxisZ, 1);

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

		// 今の状態のアニメーションを再生する(クリップの数ではなく、番号を渡す)
		m_modelRender->PlayAnimation(static_cast<int>(m_currentState));

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


	Vector3 Enemy::GetFocusPosition() const
	{
		Vector3 focusPos = m_transform.GetWorldPosition();
		focusPos.y += ENEMY_FOCUS_POSITION_Y;
		return focusPos;
	}
}