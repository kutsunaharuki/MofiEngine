/**
 * @file CharacterBase.cpp
 * @brief キャラクターの基底クラス
 */
#include "stdafx.h"
#include "CharacterBase.h"
#include "CharacterStateMachine.h"


namespace app
{
	namespace actor
	{
		namespace
		{
			/** キャラクターコントローラーの半径 */
			constexpr float CHARACTER_CONTROLLER_RADIUS = 25.0f;
			/** キャラクターコントローラーの高さ */
			constexpr float CHARACTER_CONTROLLER_HEIGHT = 75.0f;
		}


		CharacterBase::CharacterBase()
			: m_clipNum(0)
			, m_upAxis(enModelUpAxisZ)
			, m_characterStateMachine(nullptr)
			, m_moveSpeed(Vector3::Zero)
		{}
		
		
		CharacterBase::~CharacterBase()
		{}
		
		
		void CharacterBase::Init(const ModelData & data)
		{
			// クリップ数とupAxisを保持
			m_clipNum = data.clipNum;
			m_upAxis = data.upAxis;

			// アニメーションクリップ配列の確保と読み込み
			if (m_clipNum > 0 && data.animationData != nullptr)
			{
				m_animationClips = std::make_unique<AnimationClip[]>(m_clipNum);
				for (int i = 0; i < m_clipNum; ++i)
				{
					m_animationClips[i].Load(data.animationData[i].fileName);
					m_animationClips[i].SetLoopFlag(data.animationData[i].isLoop);
				}
			}

			// モデルの初期化
			m_modelRender.Init(data.fileName, m_animationClips.get(), m_upAxis, m_clipNum);
		}
		
		
		bool CharacterBase::Start()
		{
			// キャラクターコントローラーの初期化
			m_characterController.Init(
				CHARACTER_CONTROLLER_RADIUS, 
				CHARACTER_CONTROLLER_HEIGHT,
				m_transform.GetLocalPosition()
			);
			return true;
		}


		void CharacterBase::Update()
		{
			// ステートマシンが行動を決める(移動量やアニメーションの指示)
			m_characterStateMachine->Update();

			// キャラクターコントローラーを実行して、移動量を反映する
			m_characterController.Execute(m_moveSpeed, g_gameTime->GetFrameDeltaTime());

			// 位置をトランスフォームに書き込んで、ワールド値を更新する
			m_transform.SetLocalPosition(m_characterController.GetPosition());
			// UpdateTransform()を呼ぶのは、親がいない場合だけ呼ぶ
			m_transform.UpdateTransform();

			// ワールド値をモデルに反映する
			m_modelRender.SetTRS(
				m_transform.GetWorldPosition(),
				m_transform.GetWorldRotation(),
				m_transform.GetWorldScale()
			);

			// モデルの更新
			m_modelRender.Update();
		}
		
		
		void CharacterBase::Render(RenderContext & rc)
		{
			// モデルの描画
			m_modelRender.Draw(rc);
		}
	}
}