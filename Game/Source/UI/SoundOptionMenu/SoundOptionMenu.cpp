/***
 * @file SoundOptionMenu.cpp
 * @brief サウンドオプションに関するクラス
 */
#include "stdafx.h"
#include "SoundOptionMenu.h"
#include "Source/Sound/SoundManager.h"
#include <cmath>
#include <string>


namespace app
{
	namespace ui
	{
		namespace
		{
			/**
			 * @brief 1つの項目の UI の名前
			 */
			struct SoundOptionItemName
			{
				/** 文字のUIの名前(JSON の "name") */
				const char* textName;
				/** 音量ゲージのUIの名前(JSON の "name") */
				const char* gaugeName;
				/** 音量ゲージの背景のUIの名前(JSON の "name") */
				const char* gaugeBackName;
				/** 音量の数値のUIの名前(JSON の "name") */
				const char* valueName;
			};


			/** 項目の表(ItemType の順番) */
			constexpr SoundOptionItemName ITEM_TABLE[] =
			{
				{ "MasterText",	"MasterGauge",	"MasterGaugeBack",	"MasterValue" },
				{ "VoiceText",	"VoiceGauge",	"VoiceGaugeBack",	"VoiceValue" },
				{ "BGMText",	"BGMGauge",		"BGMGaugeBack",		"BGMValue" },
				{ "SEText",		"SEGauge",		"SEGaugeBack",		"SEValue" },
			};

			/** 選んでいる文字の下線のUIの名前(JSON の "name") */
			constexpr const char* UNDERLINE_NAME = "SelectUnderline";

			/** 点滅の1往復の秒数 */
			constexpr float BLINK_PERIOD_SEC = 0.8f;
			/** 点滅で一番薄くなったときのα値 */
			constexpr float BLINK_MIN_ALPHA = 0.2f;
			/** 円周率 */
			constexpr float PI = 3.14159265f;

			/** 下線と文字のすき間 */
			constexpr float UNDERLINE_GAP = 4.0f;

			/** 目盛りの線の太さ */
			constexpr float SCALE_MARK_WIDTH = 3.0f;
			/** 目盛りの線が、ゲージから上下にはみ出す長さ */
			constexpr float SCALE_MARK_OVERHANG = 6.0f;
			/** 目盛りの線の色 */
			const Vector4 COLOR_SCALE_MARK = Vector4(0.1f, 0.1f, 0.1f, 1.0f);
			/** 目盛りの線の画像 */
			constexpr const char* SCALE_MARK_ASSET = "Assets/spriteData/UI/Common/White.DDS";

			/** 開いたときのアニメーションのキー */
			constexpr uint32_t ANIM_OPEN = 0;
		}


		SoundOptionMenu::SoundOptionMenu()
			: m_underline(nullptr)
			, m_blinkTimer(0.0f)
			, m_selectIndex(0)
			, m_isOpen(false)
		{
			static_assert(sizeof(ITEM_TABLE) / sizeof(ITEM_TABLE[0]) == ITEM_COUNT, "ITEM_TABLE と ITEM_COUNT の数が合っていません。");
			m_textList.fill(nullptr);
			m_gaugeList.fill(nullptr);
			m_gaugeBackList.fill(nullptr);
			m_valueTextList.fill(nullptr);
			m_volumeStepList.fill(0);
		}


		SoundOptionMenu::~SoundOptionMenu()
		{}


		void SoundOptionMenu::Update()
		{
			// 閉じているときは、何もしない(描画もされない)
			if (!m_isOpen) return;

			m_blinkTimer += g_gameTime->GetFrameDeltaTime();

			ApplyToUI();

			// アニメーション → UI の順に更新して、このフレームの値を描画に反映する
			if (m_openAnimator) m_openAnimator->Update();
			UIMenuBase::Update();
		}


		void SoundOptionMenu::InitializeLogic()
		{
			// ホットリロードでUIが作り直されたので、全て探し直す
			for (int i = 0; i < ITEM_COUNT; ++i)
			{
				m_textList[i] = FindUI<UIBase>(ITEM_TABLE[i].textName);
				m_gaugeList[i] = FindUI<UIGauge>(ITEM_TABLE[i].gaugeName);
				m_gaugeBackList[i] = FindUI<UIImage>(ITEM_TABLE[i].gaugeBackName);
				m_valueTextList[i] = FindUI<UIText>(ITEM_TABLE[i].valueName);
			}
			m_underline = FindUI<UIImage>(UNDERLINE_NAME);

			// 目盛りは JSON ではなく、ゲージに合わせてここで作る
			CreateScaleMarks();

			// 開いたとき：キャンバスごと少し小さい所から広がる(中のUIは全てキャンバスの子)
			m_openAnimator = std::make_unique<UIAnimator>();
			m_openAnimator->SetUI(GetCanvas());
			m_openAnimator->CreateAnimation<UIScaleAnimation>(ANIM_OPEN)
				->SetParameter(Vector3(0.9f, 0.9f, 1.0f), Vector3::One, 0.15f, util::EasingType::EaseOut);

			// 作り直したキャンバスにも、出しているかと、覚えている音量を反映する
			GetCanvas()->SetIsDraw(m_isOpen);
			ApplyToUI();
		}


		void SoundOptionMenu::SetIsOpen(const bool isOpen)
		{
			m_isOpen = isOpen;
			GetCanvas()->SetIsDraw(isOpen);

			// 出した時だけ、開くアニメーション
			if (isOpen && m_openAnimator) m_openAnimator->Play(ANIM_OPEN);
		}


		void SoundOptionMenu::SetSelect(const ItemType type)
		{
			m_selectIndex = static_cast<int>(type);
			// 選んだ瞬間は、はっきり見えるところから点滅を始める
			m_blinkTimer = 0.0f;
		}


		void SoundOptionMenu::SetVolumeStep(const ItemType type, const int volumeStep)
		{
			m_volumeStepList[static_cast<size_t>(type)] = volumeStep;
		}


		void SoundOptionMenu::ApplyToUI()
		{
			// 点滅：cos で 1.0 → BLINK_MIN_ALPHA → 1.0 と、なめらかに行き来する(選んだ瞬間は 1.0)
			const float wave = (std::cos(m_blinkTimer * 2.0f * PI / BLINK_PERIOD_SEC) + 1.0f) * 0.5f;
			const float blinkAlpha = BLINK_MIN_ALPHA + (1.0f - BLINK_MIN_ALPHA) * wave;

			for (int i = 0; i < ITEM_COUNT; ++i)
			{
				const int volumeStep = m_volumeStepList[i];

				// JSON の色はそのままにして、α値だけ点滅
				if (m_textList[i])
				{
					// テキストの色は選択している時のみ点滅
					Vector4 color = m_textList[i]->GetColor();
					// 選択中の文字は点滅するがそうでない文字は常に不透明
					color.w = (i == m_selectIndex) ? blinkAlpha : 1.0f;
					m_textList[i]->SetColor(color);
				}
				if (m_gaugeList[i])
				{
					// ゲージは段階ごとに伸び縮みする(目盛り1つ分ずつ)
					m_gaugeList[i]->SetRate(static_cast<float>(volumeStep) / VOLUME_STEP_COUNT);
				}
				if (m_valueTextList[i])
				{
					m_valueTextList[i]->SetText(std::to_wstring(volumeStep).c_str());
				}
			}

			UpdateUnderline();
		}


		void SoundOptionMenu::CreateScaleMarks()
		{
			for (int i = 0; i < ITEM_COUNT; ++i)
			{
				UIImage* gaugeBack = m_gaugeBackList[i];
				if (gaugeBack == nullptr) continue;

				// ゲージの背景は基底軸が左端(0, 0.5)なので、位置がゲージの左端になる
				const Vector3 left = gaugeBack->GetPosition();
				const Vector3 scale = gaugeBack->GetScale();
				const Vector2 size = gaugeBack->GetSize();
				const float width = size.x * scale.x;
				const float height = size.y * scale.y + SCALE_MARK_OVERHANG * 2.0f;

				// 端を除いた9本の線で、10等分にする
				for (int mark = 1; mark < VOLUME_STEP_COUNT; ++mark)
				{
					Vector3 position = left;
					position.x += width * static_cast<float>(mark) / VOLUME_STEP_COUNT;

					// JSON の名前とかぶらないキーにする
					const std::string name = std::string(ITEM_TABLE[i].gaugeName) + "Mark" + std::to_string(mark);
					UIImage* markUI = GetCanvas()->CreateUI<UIImage>(Hash32(name.c_str()));
					markUI->Initialize(SCALE_MARK_ASSET, SCALE_MARK_WIDTH, height, position, Vector3::One, Quaternion::Identity, COLOR_SCALE_MARK);
				}
			}
		}


		void SoundOptionMenu::UpdateUnderline()
		{
			UIBase* text = m_textList[m_selectIndex];
			if (m_underline == nullptr || text == nullptr) return;

			// 文字の画像の大きさ(スケール込み)から、すぐ下の位置と幅を計算する
			// ※ 文字の基底軸は中央(0.5, 0.5)の前提
			const Vector3& textScale = text->GetScale();
			const Vector2 textSize = text->GetSize();
			const float textWidth = textSize.x * textScale.x;
			const float textHeight = textSize.y * textScale.y;

			Vector3 position = text->GetPosition();
			position.y -= textHeight * 0.5f + UNDERLINE_GAP;
			m_underline->SetPosition(position);

			// 下線の画像の横幅を、文字の幅に合わせて伸ばす
			const Vector2 underlineSize = m_underline->GetSize();
			if (underlineSize.x > 0.0f)
			{
				Vector3 underlineScale = m_underline->GetScale();
				underlineScale.x = textWidth / underlineSize.x;
				m_underline->SetScale(underlineScale);
			}
		}




		/****************************************************/
		// SoundOptionController
		/****************************************************/


		namespace
		{
			using ItemType = SoundOptionMenu::ItemType;


			/**
			 * @brief 項目の音の種類(Master は使わない)
			 * @param type 項目
			 * @return 音の種類
			 */
			sound::EnSoundType ToSoundType(const ItemType type)
			{
				switch (type)
				{
				case ItemType::Voice: return sound::EnSoundType::Voice;
				case ItemType::SE:    return sound::EnSoundType::SE;
				default:              return sound::EnSoundType::BGM;
				}
			}


			/**
			 * @brief 項目の音量を取得
			 * @param type 項目
			 * @return 音量(0.0〜1.0)
			 */
			float GetItemVolume(const ItemType type)
			{
				const sound::SoundManager* soundManager = sound::SoundManager::GetInstance();
				return type == ItemType::Master ? soundManager->GetMasterVolume() : soundManager->GetVolume(ToSoundType(type));
			}


			/**
			 * @brief 項目の音量を設定
			 * @param type 項目
			 * @param volume 音量(0.0〜1.0)
			 */
			void SetItemVolume(const ItemType type, const float volume)
			{
				sound::SoundManager* soundManager = sound::SoundManager::GetInstance();
				if (type == ItemType::Master)
				{
					soundManager->SetMasterVolume(volume);
				}
				else
				{
					soundManager->SetVolume(ToSoundType(type), volume);
				}
			}


			/**
			 * @brief 音量を段階(0〜10)にする
			 * @param volume 音量(0.0〜1.0)
			 * @return 段階
			 * @details float のまま 0.1 を足し引きすると誤差がたまるので、段階(整数)で扱う
			 */
			int ToVolumeStep(const float volume)
			{
				return static_cast<int>(std::lround(volume * SoundOptionMenu::VOLUME_STEP_COUNT));
			}


			/**
			 * @brief 決定の音を鳴らす
			 */
			void PlayDecideSE()
			{
				sound::SoundManager::GetInstance()->Play(sound::EnSoundType::SE, sound::se::DECIDE);
			}
		}


		SoundOptionController::SoundOptionController()
			: m_view(nullptr)
			, m_selectIndex(0)
			, m_isOpen(false)
		{}


		SoundOptionController::~SoundOptionController()
		{}


		void SoundOptionController::Initialize(const char* layoutPath)
		{
			m_layout.Initialize<SoundOptionMenu>(layoutPath);
			m_view = m_layout.GetMenu<SoundOptionMenu>();

			// 最初は閉じている。音量は今の値を出しておく
			if (m_view)
			{
				m_view->SetIsOpen(false);
				m_view->SetSelect(static_cast<ItemType>(m_selectIndex));
			}
			ApplyVolumesToView();
		}


		void SoundOptionController::Update()
		{
			// 入力 → View の順にして、このフレームの値を描画に反映する
			if (m_isOpen) UpdateInput();
			m_layout.Update();
		}


		void SoundOptionController::Render(RenderContext& rc)
		{
			m_layout.Render(rc);
		}


		void SoundOptionController::Open()
		{
			m_isOpen = true;
			Select(0);
			// ほかの所で音量が変わっていても、開いた時の値を出す
			ApplyVolumesToView();
			if (m_view) m_view->SetIsOpen(true);

			PlayDecideSE();
		}


		void SoundOptionController::Close()
		{
			m_isOpen = false;
			if (m_view) m_view->SetIsOpen(false);
		}


		void SoundOptionController::UpdateInput()
		{
			constexpr int itemCount = SoundOptionMenu::ITEM_COUNT;

			// 閉じる
			if (g_pad[0]->IsTrigger(enButtonB))
			{
				PlayDecideSE();
				Close();
				return;
			}

			// 項目を選ぶ(端まで行ったら反対側に戻る)
			if (g_pad[0]->IsTrigger(enButtonUp))
			{
				Select((m_selectIndex + itemCount - 1) % itemCount);
			}
			if (g_pad[0]->IsTrigger(enButtonDown))
			{
				Select((m_selectIndex + 1) % itemCount);
			}

			// 音量を変える
			if (g_pad[0]->IsTrigger(enButtonRight))
			{
				ChangeVolume(+1);
			}
			if (g_pad[0]->IsTrigger(enButtonLeft))
			{
				ChangeVolume(-1);
			}

			// ボイスは長いので、変えるたびではなく、Aボタンで試し聞きする
			if (g_pad[0]->IsTrigger(enButtonA))
			{
				if (static_cast<ItemType>(m_selectIndex) == ItemType::Voice)
				{
					sound::SoundManager::GetInstance()->Play(sound::EnSoundType::Voice, sound::voice::CROWD);
				}
				else
				{
					PlayDecideSE();
				}
			}
		}


		void SoundOptionController::Select(const int index)
		{
			m_selectIndex = index;
			if (m_view) m_view->SetSelect(static_cast<ItemType>(m_selectIndex));
		}


		void SoundOptionController::ChangeVolume(const int step)
		{
			const ItemType type = static_cast<ItemType>(m_selectIndex);

			int volumeStep = ToVolumeStep(GetItemVolume(type)) + step;
			volumeStep = TClamp<int>::Clamp(volumeStep, 0, SoundOptionMenu::VOLUME_STEP_COUNT);
			SetItemVolume(type, static_cast<float>(volumeStep) / SoundOptionMenu::VOLUME_STEP_COUNT);

			if (m_view) m_view->SetVolumeStep(type, volumeStep);

			// 変えた音量で、どのくらいの大きさかを聞かせる(BGMは鳴っているので不要。ボイスはAボタン)
			if (type == ItemType::Master || type == ItemType::SE)
			{
				PlayDecideSE();
			}
		}


		void SoundOptionController::ApplyVolumesToView()
		{
			if (m_view == nullptr) return;

			for (int i = 0; i < SoundOptionMenu::ITEM_COUNT; ++i)
			{
				const ItemType type = static_cast<ItemType>(i);
				m_view->SetVolumeStep(type, ToVolumeStep(GetItemVolume(type)));
			}
		}
	}
}
