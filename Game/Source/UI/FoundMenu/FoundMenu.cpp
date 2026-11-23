/**
 * @file FoundMenu.cpp
 * @brief 悪党に見つかった時の UI(悪党の頭の上の「！」と、画面を暗くして出す「見つかってしまった...」)
 */
#include "stdafx.h"
#include "FoundMenu.h"
#include <algorithm>
#include <cmath>


namespace app
{
	namespace ui
	{
		namespace
		{
			/** UI の名前(JSON の "name") */
			constexpr const char* BACK_NAME = "FoundBack";
			constexpr const char* MARK_NAME = "FoundMark";
			constexpr const char* MESSAGE_NAME = "FoundMessage";

			// 「！」
			/** 0 から一番大きくなるまでの時間(秒) */
			constexpr float MARK_POP_TIME = 0.12f;
			/** 一番大きい所から、元の大きさにもどるまでの時間(秒) */
			constexpr float MARK_SETTLE_TIME = 0.15f;
			/** 一番大きい時の大きさ(倍) */
			constexpr float MARK_POP_SCALE = 1.5f;
			/** 出た後のゆれの角度(度) */
			constexpr float MARK_SHAKE_DEG = 8.0f;
			/** ゆれの速さ(1 秒に何回) */
			constexpr float MARK_SHAKE_SPEED = 6.0f;
			/** ゆれが止まるまでの時間(秒) */
			constexpr float MARK_SHAKE_TIME = 0.6f;
			/** 暗くし始めてから「！」が消えるまでの時間(秒) */
			constexpr float MARK_FADE_TIME = 0.3f;

			// 暗くする
			/** 一番暗くした時の濃さ(うっすら) */
			constexpr float BACK_MAX_ALPHA = 0.55f;
			/** 暗くなりきるまでの時間(秒) */
			constexpr float BACK_FADE_TIME = 1.0f;

			// 「見つかってしまった...」
			/** 暗くし始めてから、文字が出始めるまでの時間(秒) */
			constexpr float MESSAGE_DELAY = 0.4f;
			/** 文字が出きるまでの時間(秒) */
			constexpr float MESSAGE_FADE_TIME = 0.8f;
			/** 文字が出る時に、下から上がってくる距離 */
			constexpr float MESSAGE_RISE = 30.0f;
			/** 文字のかげのずれ */
			constexpr float MESSAGE_SHADOW_OFFSET = 3.0f;

			/** 暗くし始めてから、演出が終わる(リザルトへ移ってよい)までの時間(秒) */
			constexpr float FINISH_TIME = 3.2f;


			/**
			 * @brief 0〜1 におさめる
			 * @param value 値
			 * @return 0〜1 におさめた値
			 */
			float ClampRate(const float value)
			{
				return (std::min)((std::max)(value, 0.0f), 1.0f);
			}
		}


		FoundMenu::FoundMenu()
			: m_backFrame(nullptr)
			, m_markIcon(nullptr)
			, m_messageText(nullptr)
			, m_markBaseScale(Vector3::One)
			, m_messageBaseScale(Vector3::One)
			, m_messageBasePosition(Vector3::Zero)
			, m_isMarkShown(false)
			, m_isMarkVisible(false)
			, m_markTimer(0.0f)
			, m_isGameOver(false)
			, m_gameOverTimer(0.0f)
		{}


		FoundMenu::~FoundMenu()
		{}


		void FoundMenu::InitializeLogic()
		{
			// ホットリロードで UI が作り直されたので、探し直す
			// ※ JSON の "type" と型が違うと nullptr になる(その UI は出なくなるので、JSON を確認する)
			m_backFrame = FindUI<UIFrame>(BACK_NAME);
			m_markIcon = FindUI<UIIcon>(MARK_NAME);
			m_messageText = FindUI<UIText>(MESSAGE_NAME);

			// 動きの基準は JSON に書いた大きさと位置(作り直した直後なので、まだ動きで変わっていない)
			if (m_markIcon) m_markBaseScale = m_markIcon->GetScale();
			if (m_messageText)
			{
				m_messageBaseScale = m_messageText->GetScale();
				m_messageBasePosition = m_messageText->GetPosition();
			}

			ApplyLook();
		}


		void FoundMenu::Update()
		{
			const float deltaTime = g_gameTime->GetFrameDeltaTime();
			if (m_isMarkShown) m_markTimer += deltaTime;
			if (m_isGameOver)  m_gameOverTimer += deltaTime;
			ApplyLook();

			UIMenuBase::Update();
		}


		void FoundMenu::ShowMark()
		{
			m_isMarkShown = true;
			m_markTimer = 0.0f;
			ApplyLook();
		}


		void FoundMenu::SetMarkPosition(const Vector2& screenPosition, const bool isVisible)
		{
			m_isMarkVisible = isVisible;
			if (m_markIcon) m_markIcon->SetPosition(Vector3(screenPosition.x, screenPosition.y, 0.0f));
		}


		void FoundMenu::StartGameOver()
		{
			if (m_isGameOver) return;
			m_isGameOver = true;
			m_gameOverTimer = 0.0f;
			ApplyLook();
		}


		bool FoundMenu::IsFinished() const
		{
			return m_isGameOver && m_gameOverTimer >= FINISH_TIME;
		}


		void FoundMenu::ApplyLook()
		{
			// 「！」: ポンと大きく出て、元の大きさにもどり、少しゆれる。暗くし始めたら消えていく
			if (m_markIcon)
			{
				const float fadeRate = m_isGameOver ? 1.0f - ClampRate(m_gameOverTimer / MARK_FADE_TIME) : 1.0f;
				m_markIcon->SetIsDraw(m_isMarkShown && m_isMarkVisible && fadeRate > 0.0f);

				float scale = 1.0f;
				if (m_markTimer < MARK_POP_TIME)
				{
					scale = MARK_POP_SCALE * (m_markTimer / MARK_POP_TIME);
				}
				else if (m_markTimer < MARK_POP_TIME + MARK_SETTLE_TIME)
				{
					const float rate = (m_markTimer - MARK_POP_TIME) / MARK_SETTLE_TIME;
					scale = MARK_POP_SCALE + (1.0f - MARK_POP_SCALE) * rate;
				}
				m_markIcon->SetScale(Vector3(m_markBaseScale.x * scale, m_markBaseScale.y * scale, m_markBaseScale.z));

				// ゆれはだんだん小さくなって止まる
				const float shakeRate = 1.0f - ClampRate(m_markTimer / MARK_SHAKE_TIME);
				const float shakeDeg = std::sin(m_markTimer * MARK_SHAKE_SPEED * 2.0f * Math::PI) * MARK_SHAKE_DEG * shakeRate;
				Quaternion rotation = Quaternion::Identity;
				rotation.SetRotationDegZ(shakeDeg);
				m_markIcon->SetRotation(rotation);

				m_markIcon->SetColor(Vector4(1.0f, 1.0f, 1.0f, fadeRate));
			}

			// 画面を暗くする板: うっすら暗くなっていく
			if (m_backFrame)
			{
				m_backFrame->SetIsDraw(m_isGameOver);
				const float rate = ClampRate(m_gameOverTimer / BACK_FADE_TIME);
				m_backFrame->SetFillColor(Vector4(0.0f, 0.0f, 0.0f, BACK_MAX_ALPHA * rate));
			}

			// 「見つかってしまった...」: 少し遅れて、下から上がりながら出てくる
			if (m_messageText)
			{
				const float rate = ClampRate((m_gameOverTimer - MESSAGE_DELAY) / MESSAGE_FADE_TIME);
				m_messageText->SetIsDraw(m_isGameOver && rate > 0.0f);

				const Vector4& baseColor = m_messageText->GetColor();
				m_messageText->SetColor(Vector4(baseColor.x, baseColor.y, baseColor.z, rate));
				// かげも一緒にうすくする(かげだけ先に見えないように)
				m_messageText->SetShadow(true, MESSAGE_SHADOW_OFFSET, Vector4(0.0f, 0.0f, 0.0f, rate));
				m_messageText->SetPosition(Vector3(
					m_messageBasePosition.x,
					m_messageBasePosition.y - MESSAGE_RISE * (1.0f - rate),
					m_messageBasePosition.z));
				m_messageText->SetScale(m_messageBaseScale);
			}
		}
	}
}
