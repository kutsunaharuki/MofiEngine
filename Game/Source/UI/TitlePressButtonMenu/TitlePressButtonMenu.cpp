/**
 * @file TitlePressButtonMenu.cpp
 * @brief タイトルの「Press A Button」 の表示のみ
 */
#include "stdafx.h"
#include "TitlePressButtonMenu.h"
#include <cmath>


namespace app
{
	namespace ui
	{
		namespace
		{
			/** 文字の UI の名前(JSON で書いた name) */
			constexpr const char* PRESS_TEXT_NAME = "TitlePressButtonText";
			/** 点滅の速さ(1 秒に何回。ゆっくりにして、ふわふわさせる) */
			constexpr float BLINK_SPEED = 0.5f;
			/** 一番うすい時の濃さ(0 にすると消えて見えなくなるので、少し残す) */
			constexpr float BLINK_MIN_ALPHA = 0.3f;
			/** 上下にゆれる幅(ピクセル) */
			constexpr float FLOAT_HEIGHT = 6.0f;
			/** 縁(影)のずらす幅(JSON の shadowOffset と同じにする。縁の濃さも点滅させるので、ここで入れ直す) */
			constexpr float SHADOW_OFFSET = 3.0f;
		}


		TitlePressButtonMenu::TitlePressButtonMenu()
			: m_pressText(nullptr)
			, m_pressTextBasePosition(Vector3::Zero)
			, m_pressTextBaseColor(Vector4::White)
			, m_blinkTimer(0.0f)
			, m_isOpen(true)
		{}


		TitlePressButtonMenu::~TitlePressButtonMenu()
		{}


		void TitlePressButtonMenu::Update()
		{
			if (m_isOpen && m_pressText)
			{
				m_blinkTimer += g_gameTime->GetFrameDeltaTime();

				const float angle = m_blinkTimer * BLINK_SPEED * 2.0f * Math::PI;

				// はっきり見える所から始めて、ゆっくりうすくなったり濃くなったりする
				const float wave = 0.5f + 0.5f * std::cos(angle);
				// 0〜1 の間を、端でゆっくりになるようにして、ふわっと変わるようにする
				const float smoothWave = wave * wave * (3.0f - 2.0f * wave);
				const float alpha = BLINK_MIN_ALPHA + (1.0f - BLINK_MIN_ALPHA) * smoothWave;

				// 文字は、色(r,g,b)にも濃さを掛ける
				// ※ フォントの描き方は、色に濃さを先に掛けておく決まり(premultiplied alpha)なので、
				//    w だけを下げても白い文字はうすくならない
				m_pressText->SetColor(Vector4(
					m_pressTextBaseColor.x * alpha,
					m_pressTextBaseColor.y * alpha,
					m_pressTextBaseColor.z * alpha,
					m_pressTextBaseColor.w * alpha));
				// 縁の黒も、文字と同じ濃さにする(黒は r,g,b が 0 なので、w だけ変えればよい)
				m_pressText->SetShadow(true, SHADOW_OFFSET, Vector4(0.0f, 0.0f, 0.0f, alpha));

				// 濃くなる時に少し上がって、うすくなる時に少し下がる(ふわふわ浮いて見える)
				Vector3 position = m_pressTextBasePosition;
				position.y += FLOAT_HEIGHT * std::cos(angle);
				m_pressText->SetPosition(position);
			}

			UIMenuBase::Update();
		}


		void TitlePressButtonMenu::InitializeLogic()
		{
			// ホットリロードで作り直されたので、探し直して、出すか消すかも入れ直す
			m_pressText = FindUI<UIText>(PRESS_TEXT_NAME);
			if (m_pressText)
			{
				m_pressTextBasePosition = m_pressText->GetPosition();
				m_pressTextBaseColor = m_pressText->GetColor();
			}
			GetCanvas()->SetIsDraw(m_isOpen);
		}


		void TitlePressButtonMenu::SetIsOpen(const bool isOpen)
		{
			m_isOpen = isOpen;
			GetCanvas()->SetIsDraw(isOpen);
			m_blinkTimer = 0.0f;
		}
	}
}