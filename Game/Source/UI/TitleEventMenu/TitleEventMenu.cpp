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
			: m_selectTextType(TitleTextType::Start)
			, m_selectIndex(static_cast<int>(TitleTextType::Start))
			, m_isDecided(false)
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

			// 上と下の入力で選択項目を変更
			if (g_pad[0]->IsTrigger(enButtonDown)) Select((m_selectIndex + 1) % ITEM_COUNT);
			
			if (g_pad[0]->IsTrigger(enButtonUp)) Select((m_selectIndex + ITEM_COUNT - 1) % ITEM_COUNT);

			// 選択されている テキストを持つ
			m_selectTextType = static_cast<TitleTextType>(m_selectIndex);

			// Aボタンで決定(何をするかはここでは決めない)
			if (g_pad[0]->IsTrigger(enButtonA))
			{
				m_isDecided = true;
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
			Select(m_selectIndex);
		}


		bool TitleEventMenu::ConsumeDecided(TitleTextType& outPutType)
		{
			// 何もしない
			if (!m_isDecided) return false;

			// 1回受け取ったら戻す
			m_isDecided = false;
			outPutType = m_selectTextType;
			return true;
		}


		void TitleEventMenu::Select(int index)
		{
			if (m_textList.empty()) return;

			// 前に選んでいた項目はアニメーションを止めて、色も戻す
			m_textList[m_selectIndex]->GetAnimator().StopAll();
			m_textList[m_selectIndex]->SetScale(Vector3::One);
			m_textList[m_selectIndex]->SetColor(NORMAL_COLOR);

			// 選び直す
			m_selectIndex = index;
			m_selectTextType = static_cast<TitleTextType>(m_selectTextType);

			// 新しく選んだ項目の色を変えて再生する
			m_textList[m_selectIndex]->SetColor(SELECT_COLOR);
			// アニメーションを再生
			m_textList[m_selectIndex]->GetAnimator().Play(animKey::SELECT);
		}
	}
}