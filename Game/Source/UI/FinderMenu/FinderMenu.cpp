/***
 * @file FinderMenu.cpp
 * @brief ファインダーを構えている時の UI
 */
#include "stdafx.h"
#include "FinderMenu.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			/** UI の名前(JSON の "name") */
			constexpr const char* FOCUS_BAR_NAME = "FocusBar";
			constexpr const char* FOCUS_KNOB_NAME = "FocusKnob";
			constexpr const char* ZOOM_BAR_NAME = "ZoomBar";
			constexpr const char* ZOOM_KNOB_NAME = "ZoomKnob";
			constexpr const char* FOCUS_FRAME_NAME = "FinderFrame";
			constexpr const char* GRID_NAME = "FocusGrid";
			constexpr const char* GRID_CENTER_NAME = "FocusGridCenter";
			constexpr const char* IN_RANGE_CIRCLE_NAME = "InRangeCircle";
			constexpr const char* OUT_RANGE_CIRCLE_NAME = "OutRangeCircle";

			/**
			 * @brief つまみが動く範囲の、バーの端からの余白(バーの高さに対する割合)
			 * @details バーの画像は、上下の端が丸くなっているので、端ぴったりまでは動かさない
			 */
			constexpr float KNOB_MARGIN_RATE = 0.05f;
		}


		FinderMenu::FinderMenu()
			: m_focusBar(nullptr)
			, m_focusKnob(nullptr)
			, m_zoomBar(nullptr)
			, m_zoomKnob(nullptr)
			, m_grid(nullptr)
			, m_gridCenter(nullptr)
			, m_inRangeCircle(nullptr)
			, m_outRangeCircle(nullptr)
			, m_focusFrame(nullptr)
			, m_focusKnobBasePosition(0.0f, 0.0f, 0.0f)
			, m_zoomKnobBasePosition(0.0f, 0.0f, 0.0f)
			, m_focusRate(0.0f)
			, m_zoomRate(0.0f)
			, m_isOpen(false)
		{}


		FinderMenu::~FinderMenu()
		{}


		void FinderMenu::Update()
		{
			// 閉じている時は、何もしない(描画もされない)
			if (!m_isOpen) return;

			// つまみを、今の量の位置に動かす
			ApplyKnob(m_focusKnob, m_focusKnobBasePosition, m_focusBar, m_focusRate);
			ApplyKnob(m_zoomKnob, m_zoomKnobBasePosition, m_zoomBar, m_zoomRate);

			UIMenuBase::Update();
		}


		void FinderMenu::InitializeLogic()
		{
			// ホットリロードで UI が作り直されたので、全て探し直す
			// ※ JSON の "type" と型が違うと nullptr になる(その UI は動かなくなるので、JSON を確認する)
			m_focusBar = FindUI<UIIcon>(FOCUS_BAR_NAME);
			m_focusKnob = FindUI<UIIcon>(FOCUS_KNOB_NAME);
			m_zoomBar = FindUI<UIIcon>(ZOOM_BAR_NAME);
			m_zoomKnob = FindUI<UIIcon>(ZOOM_KNOB_NAME);
			m_focusFrame = FindUI<UIIcon>(FOCUS_FRAME_NAME);
			m_grid = FindUI<UIIcon>(GRID_NAME);
			m_gridCenter = FindUI<UIIcon>(GRID_CENTER_NAME);
			m_inRangeCircle = FindUI<UIIcon>(IN_RANGE_CIRCLE_NAME);
			m_outRangeCircle = FindUI<UIIcon>(OUT_RANGE_CIRCLE_NAME);

			// つまみの x は JSON の位置のまま使うので、覚えておく(y は量に合わせて動かす)
			if (m_focusKnob) m_focusKnobBasePosition = m_focusKnob->GetPosition();
			if (m_zoomKnob) m_zoomKnobBasePosition = m_zoomKnob->GetPosition();

			// 丸は、被写体が見つかるまで出さない
			HideSubjectCircle();

			// 作り直したキャンバスにも、開いているかを反映する
			GetCanvas()->SetIsDraw(m_isOpen);
		}


		void FinderMenu::Open()
		{
			m_isOpen = true;
			GetCanvas()->SetIsDraw(true);
		}


		void FinderMenu::Close()
		{
			m_isOpen = false;
			GetCanvas()->SetIsDraw(false);
			HideSubjectCircle();
		}


		void FinderMenu::ShowSubjectCircle(const Vector2& screenPosition, const float radius, const bool isInRange)
		{
			// 範囲内なら細い丸、範囲外なら太い丸を出す(片方だけ)
			if (m_inRangeCircle)
			{
				m_inRangeCircle->SetIsDraw(isInRange);
				ApplyCircle(m_inRangeCircle, screenPosition, radius);
			}
			if (m_outRangeCircle)
			{
				m_outRangeCircle->SetIsDraw(!isInRange);
				ApplyCircle(m_outRangeCircle, screenPosition, radius);
			}
		}


		void FinderMenu::HideSubjectCircle()
		{
			if (m_inRangeCircle) m_inRangeCircle->SetIsDraw(false);
			if (m_outRangeCircle) m_outRangeCircle->SetIsDraw(false);
		}


		Vector2 FinderMenu::GetGridHalfSize()
		{
			if (m_grid == nullptr) return Vector2(0.0f, 0.0f);

			const Vector2 size = m_grid->GetSize();
			const Vector3& scale = m_grid->GetScale();
			return Vector2(size.x * scale.x * 0.5f, size.y * scale.y * 0.5f);
		}


		void FinderMenu::ApplyKnob(UIIcon* knob, const Vector3& knobBasePosition, UIIcon* bar, const float rate)
		{
			if (knob == nullptr || bar == nullptr) return;

			// バーの基底軸は中央(0.5, 0.5)の前提。バーの下端から上端までを、量に合わせて動く
			const float barHeight = bar->GetSize().y * bar->GetScale().y;
			const float movableHeight = barHeight * (1.0f - KNOB_MARGIN_RATE * 2.0f);
			const float bottom = bar->GetPosition().y - movableHeight * 0.5f;

			Vector3 position = knobBasePosition;
			position.y = bottom + movableHeight * TClamp<float>::Clamp(rate, 0.0f, 1.0f);
			knob->SetPosition(position);
		}


		void FinderMenu::ApplyCircle(UIIcon* circle, const Vector2& screenPosition, const float radius)
		{
			circle->SetPosition(Vector3(screenPosition.x, screenPosition.y, 0.0f));
			
			// 丸の画像の横幅が、直径(半径 × 2)になるように拡大する
			const Vector2 size = circle->GetSize();
			if (size.x > 0.0f)
			{
				const float scale = (radius * 2.0f) / size.x;
				circle->SetScale(Vector3(scale, scale, 1.0f));
			}
		}
	}
}
