/**
 * @file PauseMenu.cpp
 * @brief ポーズ画面の UI(つづける・サウンド設定・タイトルへ)
 */
#include "stdafx.h"
#include "PauseMenu.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			/** ボタンの名前(JSON の "name")。SelectType の順に並べる */
			constexpr const char* SELECT_BUTTON_NAME_LIST[] =
			{
				"PauseResume",
				"PauseSound",
				"PauseToTitle"
			};
			static_assert(sizeof(SELECT_BUTTON_NAME_LIST) / sizeof(SELECT_BUTTON_NAME_LIST[0]) == static_cast<size_t>(PauseMenu::SelectType::Max),
				"ボタンの名前の数と、SelectType の数をそろえる");
		}


		PauseMenu::PauseMenu()
			: m_selectButtonList{}
			, m_selectType(SelectType::Resume)
		{}


		PauseMenu::~PauseMenu()
		{}


		void PauseMenu::InitializeLogic()
		{
			// ホットリロードで UI が作り直されたので、探し直す
			// ※ JSON の "type" と型が違うと nullptr になる(そのボタンは出なくなるので、JSON を確認する)
			for (size_t i = 0; i < m_selectButtonList.size(); ++i)
			{
				m_selectButtonList[i] = FindUI<UIButton>(SELECT_BUTTON_NAME_LIST[i]);
			}
			 
			// 作り直したボタンは選んでいる形を持っていないので、入れ直す
			ApplySelect();
		}


		void PauseMenu::SetSelect(const SelectType type)
		{
			m_selectType = type;
			ApplySelect();
		}


		void PauseMenu::PressSelect()
		{
			UIButton* button = m_selectButtonList[static_cast<size_t>(m_selectType)];
			if (button) button->Press();
		}


		void PauseMenu::ApplySelect()
		{
			for (size_t i = 0; i < m_selectButtonList.size(); ++i)
			{
				UIButton* button = m_selectButtonList[i];
				if (button == nullptr) continue;

				const bool isSelected = i == static_cast<size_t>(m_selectType);
				button->SetState(isSelected ? UIButton::ButtonState::Selected : UIButton::ButtonState::Normal);
			}
		}
	}
}
