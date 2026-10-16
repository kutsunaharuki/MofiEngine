/**
 * @file UIMenu.cpp
 * @brief UIのメニュー
 */
#include "stdafx.h"
#include "UIMenu.h"


namespace app
{
	namespace ui
	{
		UIMenuBase::UIMenuBase()
			: m_canvas(std::make_unique<UICanvas>())
		{}


		UIMenuBase::~UIMenuBase()
		{}


		void UIMenuBase::Update()
		{
			m_canvas->Update();
		}


		void UIMenuBase::Render(RenderContext& rc)
		{
			m_canvas->Render(rc);
		}


		void UIMenuBase::ResetCanvas()
		{
			m_canvas = std::make_unique<UICanvas>();
		}
	}
}
