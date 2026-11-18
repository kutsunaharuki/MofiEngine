/***
 * @file SampleMenu.cpp
 * @brief ミッションの見本の絵を出す UI
 */
#include "stdafx.h"
#include "SampleMenu.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			/** UI の名前(JSON の "name") */
			constexpr const char* NORMAL_FRAME_NAME = "SampleNormal";
			constexpr const char* FINDER_FRAME_NAME = "SampleFinder";
		}


		SampleMenu::SampleMenu()
			: m_normalFrame(nullptr)
			, m_finderFrame(nullptr)
			, m_texture(nullptr)
			, m_isFinderActive(false)
		{}


		SampleMenu::~SampleMenu()
		{}


		void SampleMenu::InitializeLogic()
		{
			// ホットリロードで UI が作り直されたので、探し直す
			// ※ JSON の "type" と型が違うと nullptr になる(その枠は出なくなるので、JSON を確認する)
			m_normalFrame = FindUI<UIIcon>(NORMAL_FRAME_NAME);
			m_finderFrame = FindUI<UIIcon>(FINDER_FRAME_NAME);

			// 作り直した UI は絵を持っていないので、渡し直す
			Apply();
		}


		void SampleMenu::SetTexture(Texture* texture)
		{
			m_texture = texture;
			Apply();
		}


		void SampleMenu::SetIsFinderActive(const bool isFinderActive)
		{
			m_isFinderActive = isFinderActive;
			Apply();
		}


		void SampleMenu::Apply()
		{
			// 覗いている時はファインダー用の枠だけ、覗いていない時は普段の枠だけ出す
			ApplyFrame(m_normalFrame, !m_isFinderActive);
			ApplyFrame(m_finderFrame, m_isFinderActive);
		}


		void SampleMenu::ApplyFrame(UIIcon* frame, const bool isShow)
		{
			if (frame == nullptr) return;

			// 見本が無い(まだ描き終わっていない・全部達成した)時は、枠ごと消す
			if (!isShow || m_texture == nullptr)
			{
				frame->SetIsDraw(false);
				return;
			}

			// 同じ絵なら、中で作り直さない
			frame->SetTexture(*m_texture);
			frame->SetIsDraw(true);
		}
	}
}
