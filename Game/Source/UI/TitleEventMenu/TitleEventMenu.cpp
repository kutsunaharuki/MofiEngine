/**
 * @file TitleEventMenu.cpp
 * @brief タイトルのイベントメニュー
 * @details タイトルの下のスタートやサウンド等の項目
 */
#include "stdafx.h"
#include "TitleEventMenu.h"
#include <cmath>


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
			const Vector4 NORMAL_COLOR = g_vec4White;

			/** ロゴの UI の名前(JSON で書いた name) */
			constexpr const char* LOGO_FLASH_NAME = "TitleLogoFlash";
			constexpr const char* LOGO_FLASH_WHITE_NAME = "TitleLogoFlashWhite";
			constexpr const char* LOGO_GLOW_NAME = "TitleLogoFlashGlow";
			constexpr const char* SPARKLE_NAME = "TitleSparkle";
			constexpr const char* SCREEN_FLASH_NAME = "TitleScreenFlash";

			/** タイトルが出てから、最初に光るまでの時間(秒) */
			constexpr float FLASH_FIRST_DELAY = 0.8f;
			/** 光ってから、次に光るまでの時間(秒) */
			constexpr float FLASH_INTERVAL = 3.5f;
			/** 一番明るくなるまでの時間(秒。短いほど「ぴかっ」とする) */
			constexpr float FLASH_RISE_TIME = 0.07f;
			/** 一番明るい所から、元に戻るまでの時間(秒) */
			constexpr float FLASH_FALL_TIME = 0.6f;
			/** 光った時の Flash の大きさ(1 = そのまま) */
			constexpr float FLASH_POP_SCALE = 1.08f;
			/** 光った時の、画面全体の白さ(0〜1) */
			constexpr float SCREEN_FLASH_ALPHA = 0.45f;
			/** ふだんの、Flash のまわりの光の濃さ */
			constexpr float GLOW_IDLE_ALPHA = 0.35f;
			/** ふだんの光が、ゆっくり強くなったり弱くなったりする幅と速さ(1 秒に何回) */
			constexpr float GLOW_BREATH_WIDTH = 0.1f;
			constexpr float GLOW_BREATH_SPEED = 0.5f;
			/** キラッが出ている時間(秒)と、その間に回る角度 */
			constexpr float SPARKLE_TIME = 0.7f;
			constexpr float SPARKLE_ROTATION_DEG = 90.0f;
			/** キラッの一番大きい時の大きさ(1 = JSON の大きさ) */
			constexpr float SPARKLE_MAX_SCALE = 1.2f;

			/** Start を決めた時、真っ白になるまでの時間(秒。短いほどまぶしい) */
			constexpr float START_FLASH_RISE_TIME = 0.08f;
			/** Start を決めた時、真っ白のまま止めておく時間(秒) */
			constexpr float START_FLASH_HOLD_TIME = 0.35f;
			/** Start を決めた時の、まわりの光の大きさ(光が画面いっぱいに広がる) */
			constexpr float START_FLASH_GLOW_SCALE = 2.5f;
		}


		TitleEventMenu::TitleEventMenu()
			: m_selectIndex(static_cast<int>(TitleTextType::Start))
			, m_logoFlash(nullptr)
			, m_logoFlashWhite(nullptr)
			, m_logoGlow(nullptr)
			, m_sparkle(nullptr)
			, m_screenFlash(nullptr)
			, m_flashTimer(0.0f)
			, m_isStartFlash(false)
			, m_startFlashTimer(0.0f)
			, m_logoFlashBaseScale(Vector3::One)
			, m_logoGlowBaseScale(Vector3::One)
			, m_sparkleBaseScale(Vector3::One)
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

			// ロゴの Flash を光らせる
			UpdateLogoFlash(g_gameTime->GetFrameDeltaTime());

			UIMenuBase::Update();
		}


		void TitleEventMenu::InitializeLogic()
		{
			m_textList.clear();

			// ロゴ(ホットリロードで作り直されたので、探し直して元の大きさを覚える)
			m_logoFlash = FindUI<UIIcon>(LOGO_FLASH_NAME);
			m_logoFlashWhite = FindUI<UIIcon>(LOGO_FLASH_WHITE_NAME);
			m_logoGlow = FindUI<UIIcon>(LOGO_GLOW_NAME);
			m_sparkle = FindUI<UIIcon>(SPARKLE_NAME);
			m_screenFlash = FindUI<UIFrame>(SCREEN_FLASH_NAME);
			if (m_logoFlash) m_logoFlashBaseScale = m_logoFlash->GetScale();
			if (m_logoGlow) m_logoGlowBaseScale = m_logoGlow->GetScale();
			if (m_sparkle) m_sparkleBaseScale = m_sparkle->GetScale();

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
				text->Initialize(ITEM_TEXT_LIST[i], position, Vector3::One, Quaternion::Identity, NORMAL_COLOR);

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


		void TitleEventMenu::StartScreenFlash()
		{
			m_isStartFlash = true;
			m_startFlashTimer = 0.0f;
		}


		bool TitleEventMenu::IsScreenFlashFinished() const
		{
			return m_isStartFlash && m_startFlashTimer >= START_FLASH_RISE_TIME + START_FLASH_HOLD_TIME;
		}


		void TitleEventMenu::UpdateLogoFlash(const float deltaTime)
		{
			m_flashTimer += deltaTime;

			// Start を決めた時は、いつもの光り方の代わりに、画面全体を真っ白にする
			if (m_isStartFlash)
			{
				m_startFlashTimer += deltaTime;
				const float rate = TClamp<float>::Clamp(m_startFlashTimer / START_FLASH_RISE_TIME, 0.0f, 1.0f);

				// Flash は真っ白に光ったまま
				if (m_logoFlashWhite)
				{
					m_logoFlashWhite->SetIsDraw(true);
					m_logoFlashWhite->SetColor(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
				}
				// まわりの光が、画面いっぱいに広がる
				if (m_logoGlow)
				{
					const float glowScale = 1.0f + (START_FLASH_GLOW_SCALE - 1.0f) * rate;
					m_logoGlow->SetScale(Vector3(m_logoGlowBaseScale.x * glowScale, m_logoGlowBaseScale.y * glowScale, m_logoGlowBaseScale.z));
					m_logoGlow->SetColor(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
				}
				// 画面全体を真っ白にする
				if (m_screenFlash)
				{
					m_screenFlash->SetIsDraw(true);
					m_screenFlash->SetFillColor(Vector4(1.0f, 1.0f, 1.0f, rate));
				}
				// メニューの文字も、白に消えていく
				for (UIText* text : m_textList)
				{
					Vector4 color = text->GetColor();
					color.w = 1.0f - rate;
					text->SetColor(color);
				}
				if (m_sparkle) m_sparkle->SetIsDraw(false);
				return;
			}

			// 光の強さ(0 = ふだん、1 = 一番明るい)と、光ってからの時間
			float power = 0.0f;
			float localTime = -1.0f;
			const float time = m_flashTimer - FLASH_FIRST_DELAY;
			if (time >= 0.0f)
			{
				localTime = std::fmod(time, FLASH_INTERVAL);
				if (localTime < FLASH_RISE_TIME)
				{
					// 一気に明るくする
					power = localTime / FLASH_RISE_TIME;
				}
				else
				{
					// はじめは速く、だんだんゆっくり暗くする
					const float rate = TClamp<float>::Clamp((localTime - FLASH_RISE_TIME) / FLASH_FALL_TIME, 0.0f, 1.0f);
					power = (1.0f - rate) * (1.0f - rate);
				}
			}

			// 光った時は少し大きくする
			const float popScale = 1.0f + (FLASH_POP_SCALE - 1.0f) * power;
			const Vector3 flashScale(m_logoFlashBaseScale.x * popScale, m_logoFlashBaseScale.y * popScale, m_logoFlashBaseScale.z);
			if (m_logoFlash)
			{
				m_logoFlash->SetScale(flashScale);
			}
			// 真っ白な Flash を重ねて、文字が白く光ったように見せる
			if (m_logoFlashWhite)
			{
				m_logoFlashWhite->SetIsDraw(power > 0.0f);
				m_logoFlashWhite->SetScale(flashScale);
				m_logoFlashWhite->SetColor(Vector4(1.0f, 1.0f, 1.0f, power));
			}
			// まわりの光: ふだんはゆっくり強くなったり弱くなったりして、光った時は一番強くする
			if (m_logoGlow)
			{
				const float breath = std::sin(m_flashTimer * GLOW_BREATH_SPEED * 2.0f * Math::PI) * GLOW_BREATH_WIDTH;
				const float idleAlpha = GLOW_IDLE_ALPHA + breath;
				const float alpha = idleAlpha + (1.0f - idleAlpha) * power;
				const float glowScale = 1.0f + (FLASH_POP_SCALE - 1.0f) * 2.0f * power;
				m_logoGlow->SetScale(Vector3(m_logoGlowBaseScale.x * glowScale, m_logoGlowBaseScale.y * glowScale, m_logoGlowBaseScale.z));
				m_logoGlow->SetColor(Vector4(1.0f, 1.0f, 1.0f, alpha));
			}
			// 画面全体をうっすら白くする
			if (m_screenFlash)
			{
				m_screenFlash->SetIsDraw(power > 0.0f);
				m_screenFlash->SetFillColor(Vector4(1.0f, 1.0f, 1.0f, SCREEN_FLASH_ALPHA * power));
			}
			// キラッ: 大きくなりながら回って、また小さくなって消える
			if (m_sparkle)
			{
				const bool isShow = localTime >= 0.0f && localTime < SPARKLE_TIME;
				m_sparkle->SetIsDraw(isShow);
				if (isShow)
				{
					const float rate = localTime / SPARKLE_TIME;
					const float wave = std::sin(rate * Math::PI);
					m_sparkle->SetScale(Vector3(m_sparkleBaseScale.x * wave * SPARKLE_MAX_SCALE, m_sparkleBaseScale.y * wave * SPARKLE_MAX_SCALE, m_sparkleBaseScale.z));
					Quaternion rotation;
					rotation.SetRotationDegZ(SPARKLE_ROTATION_DEG * rate);
					m_sparkle->SetRotation(rotation);
					m_sparkle->SetColor(Vector4(1.0f, 1.0f, 1.0f, wave));
				}
			}
		}
	}
}
