/**
 * @file TitleEventMenu.cpp
 * @brief タイトルのイベントメニュー
 * @details タイトルの下のスタートやサウンド等の項目
 */
#include "stdafx.h"
#include "TitleEventMenu.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			/** 並べる文字 */
			constexpr const wchar_t* ITEM_TEXT_LIST[] =
			{
				L"Start",
				L"Sound",
				L"End"
			};
			/** 項目の数 */
			constexpr int ITEM_COUNT = sizeof(ITEM_TEXT_LIST) / sizeof(ITEM_TEXT_LIST[0]);
			/** テキスト(項目)の間隔(縦) */
			constexpr float ITEM_SPACING = 100.0f;
			/** 基準の位置のUI の名前(JSON で書いた name) */
			constexpr const char* BASE_UI_NAME = "TitleMenuActor";

			
			/** 選択中の色 / 選んでいない色 */
			const Vector4 SELECT_COLOR = { 1.0f,0.8f,0.2f,1.0f };
			const Vector4 NORMAL_COLOR = g_vec4Black;
		}


		TitleEventMenu::TitleEventMenu()
			: m_selectIndex(static_cast<int>(TitleTextType::Start))
		{
			// アニメーションのパラメーターを読み込む
			m_animationParameter.Load("Assets/parameter/UI/TitleEventItem/TitleSelectAnimation.json");
		}
		
		
		TitleEventMenu::~TitleEventMenu()
		{}


		void TitleEventMenu::Update()
		{
			// アニメーションのパラメーターが変更されていたらアニメーションを再登録
			if (m_animationParameter.ReloadIfModified())
			{
				for(UIText* text : m_textList)
				{
					UIAnimationFactory::Attach<UIScaleAnimation>(text, m_animationParameter, animKey::SELECT);
				}
			}

			UIMenuBase::Update();
		}


		void TitleEventMenu::InitializeLogic()
		{
			m_textList.clear();

			// 基準となるUIを探す(JSON の name から探す)
			UIBase* baseUI = FindUI<UIBase>(BASE_UI_NAME);

			if (baseUI == nullptr) return;

			// 基準の位置を取得
			const Vector3 basePosition = baseUI->GetPosition();

			for (int i = 0; i < ITEM_COUNT; i++)
			{
				// 項目のUIText を間隔の分だけ下にずらして配置
				Vector3 position = basePosition;
				position.y -= ITEM_SPACING * static_cast<float>(i);

				const std::string name = "TitleMenuText" + std::to_string(i);
				
				UIText* text = GetCanvas()->CreateUI<UIText>(Hash32(name.c_str()));
				text->Initialize(ITEM_TEXT_LIST[i], position, Vector3::One, Quaternion::Identity, g_vec4Black);

				// 選択中のアニメーションを登録
				UIAnimationFactory::Attach<UIScaleAnimation>(text, m_animationParameter, animKey::SELECT);

				// 作ったUIText をリストにどんどん追加していく
				m_textList.push_back(text);
			}

			// 最初の選択中の項目をアニメーション再生
			SetSelect(static_cast<TitleTextType>(m_selectIndex));
		}


		void TitleEventMenu::SetSelect(const TitleTextType type)
		{
			const int index = static_cast<int>(type);
			if (index < 0 || index >= static_cast<int>(m_textList.size())) return;

			// 前に選んでいた項目はアニメーションを止めて、色も戻す
			m_textList[m_selectIndex]->GetAnimator().StopAll();
			m_textList[m_selectIndex]->SetScale(Vector3::One);
			m_textList[m_selectIndex]->SetColor(NORMAL_COLOR);

			// 選び直す
			m_selectIndex = index;

			// 新しく選んだ項目の色を変えて再生する
			m_textList[m_selectIndex]->SetColor(SELECT_COLOR);
			// アニメーションを再生
			m_textList[m_selectIndex]->GetAnimator().Play(animKey::SELECT);
		}
	}
}