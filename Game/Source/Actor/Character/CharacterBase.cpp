/**
 * @file CharacterBase.cpp
 * @brief キャラクターの基底クラス
 */
#include "stdafx.h"
#include "CharacterBase.h"
#include "CharacterStateMachine.h"
#include "Source/Sound/SoundManager.h"


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
			/**
			 * キャラクターコントローラーで上れる段差の高さ
			 * ※ 実際に上れるのは、これ + 約 10(半径 25 の約 0.4 倍)≒ 30 まで。
			 *    美術館の階段は 1 段 19.5 なので、初期値(高さの 0.1 倍 = 7.5、上れるのは約 17.8 まで)では上れない
			 */
			constexpr float CHARACTER_CONTROLLER_STEP_HEIGHT = 20.0f;
			/** 1 フレームでこれより大きく動いたら、ワープとして足音の長さに入れない */
			constexpr float FOOTSTEP_WARP_DISTANCE = 100.0f;
		}


		CharacterBase::CharacterBase()
			: m_clipNum(0)
			, m_upAxis(enModelUpAxisZ)
			, m_characterStateMachine(nullptr)
			, m_moveSpeed(Vector3::Zero)
			, m_footstepDistance(0.0f)
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
			// 階段を上れるように、上れる段差の高さを上げる(Init の後に呼ぶ。Init で初期値に戻るため)
			m_characterController.SetStepHeight(CHARACTER_CONTROLLER_STEP_HEIGHT);
			return true;
		}


		void CharacterBase::Update()
		{
			// ステートマシンが行動を決める(移動量やアニメーションの指示)
			m_characterStateMachine->Update();

			// 時間の倍率(スローモーション。主人公は 1.0)
			const float timeScale = GetTimeScale();

			// 足音のために、動く前の位置を覚えておく
			const Vector3 prevPosition = m_characterController.GetPosition();

			// キャラクターコントローラーを実行して、移動量を反映する
			m_characterController.Execute(m_moveSpeed, g_gameTime->GetFrameDeltaTime() * timeScale);

			// 歩いた長さで足音を鳴らす
			UpdateFootstep(prevPosition);

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

			// アニメーションも同じ倍率で遅くする(止めている時は 0 のまま)
			m_modelRender.SetAnimationSpeed(m_animationSpeed * timeScale);

			// モデルの更新
			m_modelRender.Update();
		}
		
		
		void CharacterBase::UpdateFootstep(const Vector3& prevPosition)
		{
			const float stride = GetFootstepStride();
			if (stride <= 0.0f) return;

			// 横に動いた長さだけを見る(落ちたり、上ったりした分は入れない)
			Vector3 moved = m_characterController.GetPosition() - prevPosition;
			moved.y = 0.0f;
			const float distance = moved.Length();

			// ワープした時のような大きな動きは、歩いたことにしない
			if (distance > FOOTSTEP_WARP_DISTANCE) return;
			// 空中では鳴らさない
			if (!m_characterController.IsOnGround()) return;

			m_footstepDistance += distance;
			if (m_footstepDistance < stride) return;

			m_footstepDistance = 0.0f;
			if (CanPlayFootstep())
			{
				sound::SoundManager::GetInstance()->Play(sound::EnSoundType::SE, sound::se::FOOTSTEP);
			}
		}


		void CharacterBase::Render(RenderContext & rc)
		{
			// モデルの描画
			m_modelRender.Draw(rc);
		}
	}
}