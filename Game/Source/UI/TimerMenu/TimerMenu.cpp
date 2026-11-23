/**
 * @file TimerMenu.cpp
 * @brief 制限時間の UI(画面の上の真ん中に、のこり時間を出す)
 */
#include "stdafx.h"
#include "TimerMenu.h"
#include <algorithm>
#include <cmath>
#include <string>


namespace app
{
	namespace ui
	{
		namespace
		{
			/** UI の名前(JSON の "name") */
			constexpr const char* PANEL_NAME = "TimerPanel";
			constexpr const char* LABEL_NAME = "TimerLabel";
			constexpr const char* TIME_NAME = "TimerText";

			/** この秒数以下になったら、黄色にする */
			constexpr int WARNING_SECOND = 30;
			/** この秒数以下になったら、赤にして点滅させる */
			constexpr int DANGER_SECOND = 10;

			/** まだある時の色 */
			const Vector4 COLOR_NORMAL = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
			/** 少なくなってきた時の色 */
			const Vector4 COLOR_WARNING = Vector4(1.0f, 0.8f, 0.25f, 1.0f);
			/** もうすぐ 0 の時の色 */
			const Vector4 COLOR_DANGER = Vector4(1.0f, 0.3f, 0.3f, 1.0f);

			/** 点滅の速さ(1 秒に何回) */
			constexpr float BLINK_SPEED = 2.0f;
			/** 点滅で一番うすくなった時の濃さ */
			constexpr float BLINK_MIN_ALPHA = 0.45f;

			/** 1 秒ごとに数字が大きくなる時の、最初の大きさ(倍) */
			constexpr float PULSE_SCALE = 1.3f;
			/** 大きくなった数字が、元の大きさに戻るまでの時間(秒) */
			constexpr float PULSE_TIME = 0.25f;
		}


		TimerMenu::TimerMenu()
			: m_panel(nullptr)
			, m_labelText(nullptr)
			, m_timeText(nullptr)
			, m_timeBaseScale(Vector3::One)
			, m_remainTime(0.0f)
			, m_shownSecond(0)
			, m_pulseTimer(PULSE_TIME)
			, m_blinkTimer(0.0f)
		{}


		TimerMenu::~TimerMenu()
		{}


		void TimerMenu::InitializeLogic()
		{
			// ホットリロードで UI が作り直されたので、探し直す
			// ※ JSON の "type" と型が違うと nullptr になる(その UI は出なくなるので、JSON を確認する)
			m_panel = FindUI<UIFrame>(PANEL_NAME);
			m_labelText = FindUI<UIText>(LABEL_NAME);
			m_timeText = FindUI<UIText>(TIME_NAME);

			// 動きの基準は JSON に書いた大きさ(作り直した直後なので、まだ動きで変わっていない)
			if (m_timeText) m_timeBaseScale = m_timeText->GetScale();

			// 作り直した UI は値を持っていないので、入れ直す
			ApplyText();
			ApplyLook();
		}


		void TimerMenu::Update()
		{
			// 点滅と、数字が大きくなる動きを進める
			const float deltaTime = g_gameTime->GetFrameDeltaTime();
			m_blinkTimer += deltaTime;
			m_pulseTimer += deltaTime;
			ApplyLook();

			UIMenuBase::Update();
		}


		void TimerMenu::SetRemainTime(const float remainTime)
		{
			m_remainTime = (std::max)(remainTime, 0.0f);

			// 出す秒数は切り上げる(2.3 秒なら 3。0 は本当に 0 になった時だけ出る)
			const int second = static_cast<int>(std::ceil(m_remainTime));
			if (second == m_shownSecond) return;

			m_shownSecond = second;
			ApplyText();

			// もうすぐ 0 の時は、1 秒ごとに数字をドンと大きくする(0 になった時はしない)
			if (GetTone() == Tone::Danger && m_shownSecond > 0)
			{
				m_pulseTimer = 0.0f;
			}
		}


		TimerMenu::Tone TimerMenu::GetTone() const
		{
			if (m_shownSecond <= DANGER_SECOND)  return Tone::Danger;
			if (m_shownSecond <= WARNING_SECOND) return Tone::Warning;
			return Tone::Normal;
		}


		void TimerMenu::ApplyText()
		{
			if (m_timeText == nullptr) return;

			// 「分:秒」にする(秒は 2 けた。例: 3:05)
			const int minute = m_shownSecond / 60;
			const int second = m_shownSecond % 60;
			std::wstring text = std::to_wstring(minute) + L":";
			if (second < 10) text += L"0";
			text += std::to_wstring(second);
			m_timeText->SetText(text.c_str());
		}


		void TimerMenu::ApplyLook()
		{
			// 段階で色を決める
			const Tone tone = GetTone();
			Vector4 color = COLOR_NORMAL;
			if (tone == Tone::Warning) color = COLOR_WARNING;
			if (tone == Tone::Danger)  color = COLOR_DANGER;

			// もうすぐ 0 の時は点滅させる(0 になったら点滅は止めて、赤のままにする)
			Vector4 timeColor = color;
			if (tone == Tone::Danger && m_shownSecond > 0)
			{
				const float wave = std::sin(m_blinkTimer * BLINK_SPEED * 2.0f * Math::PI) * 0.5f + 0.5f;
				timeColor.w *= BLINK_MIN_ALPHA + (1.0f - BLINK_MIN_ALPHA) * wave;
			}

			if (m_timeText)
			{
				m_timeText->SetColor(timeColor);

				// 1 秒ごとに大きくなった数字を、だんだん元の大きさに戻す
				const float rate = (std::min)(m_pulseTimer / PULSE_TIME, 1.0f);
				const float scale = PULSE_SCALE + (1.0f - PULSE_SCALE) * rate;
				m_timeText->SetScale(Vector3(m_timeBaseScale.x * scale, m_timeBaseScale.y * scale, m_timeBaseScale.z));
			}
			if (m_labelText)
			{
				m_labelText->SetColor(color);
			}
			// 枠の線も、まだある時以外は同じ色にする(まだある時は JSON / UIStyle の色のまま)
			if (m_panel && tone != Tone::Normal)
			{
				m_panel->SetBorderColor(color);
			}
		}
	}
}
