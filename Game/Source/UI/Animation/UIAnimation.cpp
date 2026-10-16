/**
 * @file UIAnimation.cpp
 * @brief UIAnimationの機能群
 */
#include "stdafx.h"
#include "UIAnimation.h"
#include "Source/UI/UIParts.h"


namespace app
{
	namespace ui
	{
		/*************************************/
		// UIColorAnimation
		/*************************************/


		UIColorAnimation::UIColorAnimation()
		{
			SetFunc([this](const Vector4& color)
				{
					m_ui->SetColor(color);
				});
		}




		/*************************************/
		// UIScaleAnimation
		/*************************************/


		UIScaleAnimation::UIScaleAnimation()
		{
			SetFunc([this](const Vector3& scale)
				{
					m_ui->SetScale(scale);
				});
		}




		/*************************************/
		// UITranslateAnimation
		/*************************************/


		UITranslateAnimation::UITranslateAnimation()
		{
			SetFunc([this](const Vector3& position)
				{
					m_ui->SetPosition(position);
				});
		}




		/*************************************/
		// UITranslateOffsetAnimation
		/*************************************/


		UITranslateOffsetAnimation::UITranslateOffsetAnimation()
			: m_basePosition(Vector3::Zero)
		{
			SetFunc([this](const Vector3& offset)
				{
					// 毎フレーム足し続けると、どんどんずれていくので、開始時の座標を基準にする
					Vector3 position = m_basePosition;
					position.Add(offset);
					m_ui->SetPosition(position);
				});
		}


		void UITranslateOffsetAnimation::OnPlay()
		{
			if (m_ui == nullptr) return;
			m_basePosition = m_ui->GetPosition();
		}




		/*************************************/
		// UIRotationAnimation
		/*************************************/


		UIRotationAnimation::UIRotationAnimation()
		{
			SetFunc([this](const float angle)
				{
					Quaternion rotation;
					rotation.SetRotationDegZ(angle);
					m_ui->SetRotation(rotation);
				});
		}




		/*************************************/
		// UIAnimationSequence
		/*************************************/


		void UIAnimationSequence::Update()
		{
			if (!m_isPlaying || m_animator == nullptr) return;

			// ステップ開始前の待機
			if (m_waitingDelay)
			{
				m_delayTimer -= g_gameTime->GetFrameDeltaTime();
				if (m_delayTimer > 0.0f) return;

				m_waitingDelay = false;
				StartCurrentStep();
				return;
			}

			if (!IsValidIndex()) return;

			// 現在のステップのアニメーションが終わったら、次に進む
			const UIAnimationStep& step = m_steps[m_currentIndex];
			UIAnimationBase* animation = m_animator->FindAnimation(step.animationKey);
			if (animation && !animation->IsPlayAnimation())
			{
				if (step.onComplete) step.onComplete();
				AdvanceToNext();
			}
		}


		void UIAnimationSequence::Stop()
		{
			// 再生中のステップのアニメーションも止める
			if (m_isPlaying && !m_waitingDelay && IsValidIndex() && m_animator)
			{
				UIAnimationBase* animation = m_animator->FindAnimation(m_steps[m_currentIndex].animationKey);
				if (animation) animation->StopAnimation();
			}

			m_isPlaying = false;
			m_currentIndex = -1;
			m_waitingDelay = false;
		}


		void UIAnimationSequence::AdvanceToNext()
		{
			m_currentIndex++;
			if (m_currentIndex >= static_cast<int>(m_steps.size()))
			{
				// 全ステップ完了
				m_isPlaying = false;
				if (m_onSequenceComplete) m_onSequenceComplete();
				return;
			}

			const UIAnimationStep& step = m_steps[m_currentIndex];
			if (step.delayBefore > 0.0f)
			{
				m_delayTimer = step.delayBefore;
				m_waitingDelay = true;
			}
			else
			{
				StartCurrentStep();
			}
		}


		void UIAnimationSequence::StartCurrentStep()
		{
			if (!IsValidIndex()) return;

			const UIAnimationStep& step = m_steps[m_currentIndex];
			UIAnimationBase* animation = m_animator->FindAnimation(step.animationKey);
			if (animation == nullptr)
			{
				// アニメーションが見つからない場合は、スキップする
				AdvanceToNext();
				return;
			}

			if (step.onStart) step.onStart();
			animation->PlayAnimation();
		}
	}
}
