/**
 * @file LoadingMenu.cpp
 * @brief ロード画面の UI(暗くなった画面に「読み込み中...」とヒントを出す)
 */
#include "stdafx.h"
#include "LoadingMenu.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			/** UI の名前(JSON の "name") */
			constexpr const char* LOADING_TEXT_NAME = "LoadingText";

			/** 点が 1 つ増えるまでの時間(秒) */
			constexpr float DOT_INTERVAL = 0.25f;
			/** 点の一番多い数(これを超えたら 0 に戻る) */
			constexpr int DOT_MAX = 3;
		}


		LoadingMenu::LoadingMenu()
			: m_loadingText(nullptr)
			, m_timer(0.0f)
		{}


		LoadingMenu::~LoadingMenu()
		{}


		void LoadingMenu::InitializeLogic()
		{
			// ホットリロードで UI が作り直されたので、探し直す
			m_loadingText = FindUI<UIText>(LOADING_TEXT_NAME);
			// 点を付ける前の文字は、JSON に書いた文字にする(作り直した直後なので、まだ点は付いていない)
			if (m_loadingText) m_baseText = m_loadingText->GetText();

			ApplyText();
		}


		void LoadingMenu::Update()
		{
			m_timer += g_gameTime->GetFrameDeltaTime();
			ApplyText();

			UIMenuBase::Update();
		}


		void LoadingMenu::Restart()
		{
			m_timer = 0.0f;
			ApplyText();
		}


		void LoadingMenu::ApplyText()
		{
			if (m_loadingText == nullptr) return;

			// 時間で点の数を決める(0 → 1 → 2 → 3 → 0 …)
			const int dotCount = static_cast<int>(m_timer / DOT_INTERVAL) % (DOT_MAX + 1);
			const std::wstring text = m_baseText + std::wstring(dotCount, L'.');
			m_loadingText->SetText(text.c_str());
		}
	}
}
