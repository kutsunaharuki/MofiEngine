/**
 * @file TitleEventMenu.cpp
 * @brief タイトルのイベントメニュー
 * @details タイトルの下のスタートやサウンド等の項目
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
			/** 文字の右に出すアイコン(ITEM_TEXT_LIST と同じ順番) */
			constexpr const char* ITEM_ICON_LIST[] =
			{
				"Assets/spriteData/UI/Title/Icon_Camera_White.dds",
				"Assets/spriteData/UI/Title/Icon_Sound_White.dds",
				"Assets/spriteData/UI/Title/Icon_End_White.dds"
			};
			static_assert(sizeof(ITEM_ICON_LIST) / sizeof(ITEM_ICON_LIST[0]) == ITEM_COUNT,
				"ITEM_ICON_LIST と ITEM_TEXT_LIST の数が合っていません。");
			/** テキスト(項目)の間隔(縦) */
			constexpr float ITEM_SPACING = 90.0f;
			/** 基準の位置のUI の名前(JSON で書いた name。1 つ目の項目の文字の左はし) */
			constexpr const char* BASE_UI_NAME = "TitleMenuActor";

			/** アイコンの大きさ */
			constexpr float ICON_SIZE = 56.0f;
			/** 文字とアイコンの間 */
			constexpr float ICON_MARGIN = 20.0f;
			/** 下線の太さ */
			constexpr float UNDERLINE_HEIGHT = 4.0f;
			/** 文字(アイコン)の下はしと下線の間 */
			constexpr float UNDERLINE_GAP = 6.0f;
			/** 帯の、項目の外の余白(横 / 縦) */
			constexpr float BAND_PADDING_X = 40.0f;
			constexpr float BAND_PADDING_Y = 10.0f;
			/** 帯の色(うすい黒) */
			const Vector4 BAND_COLOR = { 0.0f, 0.0f, 0.0f, 0.45f };

			/** 選択中の色 / 選んでいない色 */
			const Vector4 SELECT_COLOR = { 1.0f,0.8f,0.2f,1.0f };
			const Vector4 NORMAL_COLOR = g_vec4White;

			/** ロゴの UI の名前(JSON で書いた name) */
			constexpr const char* LOGO_GLOW_NAME = "TitleLogoFlashGlow";
			constexpr const char* SPARKLE_NAME = "TitleSparkle";
			constexpr const char* SCREEN_FLASH_NAME = "TitleScreenFlash";
			constexpr const char* CAMERA_LIGHT_NAME = "TitleCameraFlash";

			/** ボタンの案内の UI の名前(項目と一緒に出したり消したりする) */
			constexpr const char* GUIDE_NAME_LIST[] =
			{
				"TitleGuideIconDpadVertical",
				"TitleGuideTextDpadVertical",
				"TitleGuideIconA",
				"TitleGuideTextA",
			};


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
			/** Start を決めた時、カメラの光の大きさ(カメラから画面いっぱいに広がる) */
			constexpr float START_FLASH_LIGHT_SCALE = 8.0f;
		}


		TitleEventMenu::TitleEventMenu()
			: m_selectIndex(static_cast<int>(TitleTextType::Start))
			, m_itemBand(nullptr)
			, m_cameraLight(nullptr)
			, m_cameraLightBaseScale(Vector3::One)
			, m_logoGlow(nullptr)
			, m_sparkle(nullptr)
			, m_screenFlash(nullptr)
			, m_flashTimer(0.0f)
			, m_isStartFlash(false)
			, m_startFlashTimer(0.0f)
			, m_sparkleBaseScale(Vector3::One)
			, m_isItemOpen(false)
		{}
		
		
		TitleEventMenu::~TitleEventMenu()
		{}


		void TitleEventMenu::Update()
		{
			// ロゴの Flash を光らせる
			UpdateLogoFlash(g_gameTime->GetFrameDeltaTime());

			UIMenuBase::Update();
		}


		void TitleEventMenu::InitializeLogic()
		{
			// ホットリロードでキャンバスごと作り直されたので、前のポインタは使えない
			m_textList.clear();
			m_iconList.clear();
			m_underlineList.clear();
			m_itemBand = nullptr;

			// ロゴ(ホットリロードで作り直されたので、探し直して元の大きさを覚える)
			m_logoGlow = FindUI<UIIcon>(LOGO_GLOW_NAME);
			m_sparkle = FindUI<UIIcon>(SPARKLE_NAME);
			m_screenFlash = FindUI<UIFrame>(SCREEN_FLASH_NAME);
			m_cameraLight = FindUI<UIIcon>(CAMERA_LIGHT_NAME);

			if (m_sparkle) m_sparkleBaseScale = m_sparkle->GetScale();
			if (m_cameraLight) m_cameraLightBaseScale = m_cameraLight->GetScale();

			// 基準となるUIを探す(JSON の name から探す)
			UIBase* baseUI = FindUI<UIBase>(BASE_UI_NAME);

			if (baseUI == nullptr) return;

			// 基準の位置から、項目を作って並べる
			CreateItemList(baseUI->GetPosition());

			// 最初に選んでいる項目に、色と下線を合わせる
			SetSelect(static_cast<TitleTextType>(m_selectIndex));

			// ホットリロードで作り直したので、出すか消すかを入れ直す
			SetIsItemOpen(m_isItemOpen);
		}


		void TitleEventMenu::SetSelect(const TitleTextType type)
		{
			const int index = static_cast<int>(type);
			if (index < 0 || index >= static_cast<int>(m_textList.size())) return;

			// 選び直して、選んだ項目だけ黄色と下線にする(大きさは変えない)
			m_selectIndex = index;
			ApplyItemColor(1.0f);
		}


		void TitleEventMenu::CreateItemList(const Vector3& basePosition)
		{
			// 帯は項目の下に描きたいので、一番先に作る(キャンバスは作った順に描く)
			// ※ 大きさは、項目を並べて幅がわかってから決める
			m_itemBand = GetCanvas()->CreateUI<UIFrame>(Hash32("TitleMenuBand"));

			// 帯の右はしを決めるために、一番長い項目の右はしを覚えておく
			float maxRight = basePosition.x;

			for (int i = 0; i < ITEM_COUNT; i++)
			{
				// 項目を間隔の分だけ下にずらして配置
				Vector3 position = basePosition;
				position.y -= ITEM_SPACING * static_cast<float>(i);

				const std::string index = std::to_string(i);

				// 文字(左はしを基準の位置にそろえる)
				UIText* text = GetCanvas()->CreateUI<UIText>(Hash32(("TitleMenuText" + index).c_str()));
				text->Initialize(ITEM_TEXT_LIST[i], position, Vector3::One, Quaternion::Identity, NORMAL_COLOR, Vector2(0.0f, 0.5f));
				const Vector2 textSize = text->GetSize();

				// アイコン(文字の右)
				const float iconLeft = position.x + textSize.x + ICON_MARGIN;
				const Vector3 iconPosition(iconLeft + ICON_SIZE * 0.5f, position.y, position.z);
				UIIcon* icon = GetCanvas()->CreateUI<UIIcon>(Hash32(("TitleMenuIcon" + index).c_str()));
				icon->Initialize(ITEM_ICON_LIST[i], ICON_SIZE, ICON_SIZE, iconPosition, Vector3::One, Quaternion::Identity, NORMAL_COLOR);

				// 下線(文字の左はしから、アイコンの右はしまで)
				const float right = iconLeft + ICON_SIZE;
				const float rowHeight = textSize.y > ICON_SIZE ? textSize.y : ICON_SIZE;
				const float bottom = position.y - rowHeight * 0.5f - UNDERLINE_GAP;
				const Vector3 underlinePosition((position.x + right) * 0.5f, bottom - UNDERLINE_HEIGHT * 0.5f, position.z);
				UIFrame* underline = GetCanvas()->CreateUI<UIFrame>(Hash32(("TitleMenuUnderline" + index).c_str()));
				underline->Initialize(right - position.x, UNDERLINE_HEIGHT, underlinePosition, Vector3::One, Quaternion::Identity, SELECT_COLOR, SELECT_COLOR, 0.0f);

				m_textList.push_back(text);
				m_iconList.push_back(icon);
				m_underlineList.push_back(underline);

				if (right > maxRight) maxRight = right;
			}

			// 帯: 全部の項目と、その外の余白をおおう
			const float left = basePosition.x - BAND_PADDING_X;
			const float bandRight = maxRight + BAND_PADDING_X;
			const float top = basePosition.y + ITEM_SPACING * 0.5f + BAND_PADDING_Y;
			const float bandBottom = basePosition.y - ITEM_SPACING * (static_cast<float>(ITEM_COUNT) - 0.5f) - BAND_PADDING_Y;
			const Vector3 bandPosition((left + bandRight) * 0.5f, (top + bandBottom) * 0.5f, basePosition.z);
			m_itemBand->Initialize(bandRight - left, top - bandBottom, bandPosition, Vector3::One, Quaternion::Identity, BAND_COLOR, BAND_COLOR, 0.0f);
		}


		void TitleEventMenu::ApplyItemColor(const float alpha)
		{
			for (int i = 0; i < static_cast<int>(m_textList.size()); i++)
			{
				const bool isSelect = i == m_selectIndex;
				const Vector4& color = isSelect ? SELECT_COLOR : NORMAL_COLOR;

				// 文字は、色(r,g,b)にも濃さを掛ける
				// ※ フォントの描き方は、色に濃さを先に掛けておく決まり(premultiplied alpha)なので、w だけでは消えない
				m_textList[i]->SetColor(Vector4(color.x * alpha, color.y * alpha, color.z * alpha, color.w * alpha));
				// アイコンと下線は、w だけ変えればよい
				m_iconList[i]->SetColor(Vector4(color.x, color.y, color.z, color.w * alpha));
				// 下線は、選んでいる項目だけ出す
				m_underlineList[i]->SetIsDraw(m_isItemOpen && isSelect);
				m_underlineList[i]->SetFillColor(Vector4(SELECT_COLOR.x, SELECT_COLOR.y, SELECT_COLOR.z, SELECT_COLOR.w * alpha));
			}
			if (m_itemBand) m_itemBand->SetFillColor(Vector4(BAND_COLOR.x, BAND_COLOR.y, BAND_COLOR.z, BAND_COLOR.w * alpha));
		}


		void TitleEventMenu::StartScreenFlash()
		{
			m_isStartFlash = true;
			m_startFlashTimer = 0.0f;
		}


		void TitleEventMenu::SetIsItemOpen(const bool isOpen)
		{
			m_isItemOpen = isOpen;

			// Start / Sound / End(コードで作った文字・アイコン・帯)
			for (UIText* text : m_textList)
			{
				text->SetIsDraw(isOpen);
			}
			for (UIIcon* icon : m_iconList)
			{
				icon->SetIsDraw(isOpen);
			}
			if (m_itemBand) m_itemBand->SetIsDraw(isOpen);
			// 下線は、選んでいる項目だけ出す
			for (int i = 0; i < static_cast<int>(m_underlineList.size()); i++)
			{
				m_underlineList[i]->SetIsDraw(isOpen && i == m_selectIndex);
			}
			// ボタンの案内(JSON で作ったもの)
			for (const char* name : GUIDE_NAME_LIST)
			{
				UIBase* ui = FindUI<UIBase>(name);
				if (ui) ui->SetIsDraw(isOpen);
			}
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

				if (m_screenFlash)
				{
					m_cameraLight->SetIsDraw(true);
					// 一番大きいときの大きさから画面を覆う大きさまで広げる
					const float lightScale = m_cameraFlashSetting.lightMaxScale
						+ (START_FLASH_LIGHT_SCALE - m_cameraFlashSetting.lightMaxScale) * rate;

					// 2Dの値で設定
					m_cameraLight->SetScale(Vector3(
						m_cameraLightBaseScale.x * lightScale,
						m_cameraLightBaseScale.y * lightScale,
						m_cameraLightBaseScale.z));

					// Start の時はまぶしさを下げずに真っ白で光らせる
					m_cameraLight->SetColor(Vector4::White);
				}

				// 画面全体を真っ白にする
				if (m_screenFlash)
				{
					m_screenFlash->SetIsDraw(true);
					m_screenFlash->SetFillColor(Vector4(1.0f, 1.0f, 1.0f, rate));
				}
				// メニューの文字・アイコン・下線・帯も、白に消えていく
				// (コードで作った UI は、白い板より後に作ったので、板の上に描かれる。なので自分で消す)
				ApplyItemColor(1.0f - rate);
				if (m_sparkle) m_sparkle->SetIsDraw(false);
				return;
			}

			// 光の強さ(0 = ふだん、1 = 一番明るい)と、光ってからの時間
			float power = 0.0f;
			float localTime = -1.0f;
			const float time = m_flashTimer - m_cameraFlashSetting.firstDelay;
			if (time >= 0.0f)
			{
				// 光ってからの時間(繰り返さない1回だけ光る)
				localTime = time;
				if (localTime < m_cameraFlashSetting.riseTime)
				{
					// 一気に明るくする
					power = localTime / m_cameraFlashSetting.riseTime;
				}
				else
				{
					// はじめは速く、だんだんゆっくり暗くする
					const float rate = TClamp<float>::Clamp((localTime - m_cameraFlashSetting.riseTime) / m_cameraFlashSetting.fallTime, 0.0f, 1.0f);
					power = (1.0f - rate) * (1.0f - rate);
				}
			}

			// カメラのフラッシュの光 : 小さいところから一気に大きく明るくなってだんだん小さく消える演出
			if (m_cameraLight)
			{
				// 光っていないときは描かない
				m_cameraLight->SetIsDraw(power > 0.0f);

				// 光の強さに合わせて光始めた大きさから一番大きい(最大)時の大きさまで変える
				const float lightScale = m_cameraFlashSetting.lightMinScale
					+ (m_cameraFlashSetting.lightMaxScale - m_cameraFlashSetting.lightMinScale) * power;

				// 2Dの値で設定
				m_cameraLight->SetScale(Vector3(
					m_cameraLightBaseScale.x * lightScale,
					m_cameraLightBaseScale.y * lightScale,
					m_cameraLightBaseScale.z));

				// 光の強さに合わせて濃くする
				m_cameraLight->SetColor(Vector4(1.0f, 1.0f, 1.0f, m_cameraFlashSetting.lightAlpha * power));
			}
			// まわりの光 : ゆっくり強くなったり弱くなったりするだけ(Flash で光らせない)
			if (m_logoGlow)
			{
				const float breath = std::sin(m_flashTimer * GLOW_BREATH_SPEED * 2.0f * Math::PI) * GLOW_BREATH_WIDTH;
				const float alpha = GLOW_IDLE_ALPHA + breath;
				m_logoGlow->SetColor(Vector4(1.0f, 1.0f, 1.0f, alpha));
			}
			// 画面全体をうっすら白くする
			if (m_screenFlash)
			{
				m_screenFlash->SetIsDraw(power > 0.0f);
				m_screenFlash->SetFillColor(Vector4(1.0f, 1.0f, 1.0f, m_cameraFlashSetting.lightAlpha * power));
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
