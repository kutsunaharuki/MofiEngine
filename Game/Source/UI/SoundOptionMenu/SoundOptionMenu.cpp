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
			 * @brief 1つの項目の情報
			 */
			struct SoundOptionItem
			{
				/** 文字のUIの名前(JSON の "name") */
				const char* textName;
				/** 音量ゲージのUIの名前(JSON の "name") */
				const char* gaugeName;
				/** 音量ゲージの背景のUIの名前(JSON の "name") */
				const char* gaugeBackName;
				/** 音量の数値のUIの名前(JSON の "name") */
				const char* valueName;
				/** 全体の音量(マスター音量)の項目か */
				bool isMaster;
				/** 音の種類(isMaster のときは使わない) */
				sound::EnSoundType type;
			};


			/** 項目の表(上から順番) */
			constexpr SoundOptionItem ITEM_TABLE[] =
			{
				{ "MasterText",	"MasterGauge",	"MasterGaugeBack",	"MasterValue",	true,	sound::EnSoundType::BGM },
				{ "VoiceText",	"VoiceGauge",	"VoiceGaugeBack",	"VoiceValue",	false,	sound::EnSoundType::Voice },
				{ "BGMText",	"BGMGauge",		"BGMGaugeBack",		"BGMValue",		false,	sound::EnSoundType::BGM },
				{ "SEText",		"SEGauge",		"SEGaugeBack",		"SEValue",		false,	sound::EnSoundType::SE },
			};

			/** 選んでいる文字の下線のUIの名前(JSON の "name") */
			constexpr const char* UNDERLINE_NAME = "SelectUnderline";


			/**
			 * @brief 項目の音量を取得
			 * @param item 項目
			 * @return 音量(0.0〜1.0)
			 */
			float GetItemVolume(const SoundOptionItem& item)
			{
				const sound::SoundManager* soundManager = sound::SoundManager::GetInstance();
				return item.isMaster ? soundManager->GetMasterVolume() : soundManager->GetVolume(item.type);
			}


			/**
			 * @brief 項目の音量を設定
			 * @param item 項目
			 * @param volume 音量(0.0〜1.0)
			 */
			void SetItemVolume(const SoundOptionItem& item, const float volume)
			{
				sound::SoundManager* soundManager = sound::SoundManager::GetInstance();
				if (item.isMaster)
				{
					soundManager->SetMasterVolume(volume);
				}
				else
				{
					soundManager->SetVolume(item.type, volume);
				}
			}


			/** 音量の段階の数(0〜10) */
			constexpr int VOLUME_STEP_COUNT = 10;

			/**
			 * @brief 音量を段階(0〜10)にする
			 * @param volume 音量(0.0〜1.0)
			 * @return 段階
			 * @details float のまま 0.1 を足し引きすると誤差がたまるので、段階(整数)で扱う
			 */
			int ToVolumeStep(const float volume)
			{
				return static_cast<int>(std::lround(volume * VOLUME_STEP_COUNT));
			}


			/** 選んでいない文字の色 */
			// ※ Vector4::White などを使わない。別のファイルの static 変数は、初期化される順番が決まっていないので、
			//    こちらが先に初期化されると (0, 0, 0, 0) になり、透明で見えなくなる
			const Vector4 COLOR_TEXT = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

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
		}


		SoundOptionMenu::~SoundOptionMenu()
		{}


		void SoundOptionMenu::Update()
		{
			// 閉じているときは、何もしない(描画もされない)
			if (!m_isOpen) return;

			m_blinkTimer += g_gameTime->GetFrameDeltaTime();

			UpdateInput();
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

			// 作り直したキャンバスにも、開いているかを反映する
			GetCanvas()->SetIsDraw(m_isOpen);
			ApplyToUI();
		}


		void SoundOptionMenu::Open()
		{
			m_isOpen = true;
			Select(0);
			GetCanvas()->SetIsDraw(true);
			if (m_openAnimator) m_openAnimator->Play(ANIM_OPEN);

			sound::SoundManager::GetInstance()->Play(sound::EnSoundType::SE, sound::se::DECIDE);
		}


		void SoundOptionMenu::Close()
		{
			m_isOpen = false;
			GetCanvas()->SetIsDraw(false);
		}


		void SoundOptionMenu::UpdateInput()
		{
			sound::SoundManager* soundManager = sound::SoundManager::GetInstance();

			// 閉じる
			if (g_pad[0]->IsTrigger(enButtonB))
			{
				soundManager->Play(sound::EnSoundType::SE, sound::se::DECIDE);
				Close();
				return;
			}

			// 項目を選ぶ(端まで行ったら反対側に戻る)
			if (g_pad[0]->IsTrigger(enButtonUp))
			{
				Select((m_selectIndex + ITEM_COUNT - 1) % ITEM_COUNT);
			}
			if (g_pad[0]->IsTrigger(enButtonDown))
			{
				Select((m_selectIndex + 1) % ITEM_COUNT);
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
				const SoundOptionItem& item = ITEM_TABLE[m_selectIndex];
				if (!item.isMaster && item.type == sound::EnSoundType::Voice)
				{
					soundManager->Play(sound::EnSoundType::Voice, sound::voice::CROWD);
				}
				else
				{
					soundManager->Play(sound::EnSoundType::SE, sound::se::DECIDE);
				}
			}
		}


		void SoundOptionMenu::ApplyToUI()
		{
			// 点滅：cos で 1.0 → BLINK_MIN_ALPHA → 1.0 と、なめらかに行き来する(選んだ瞬間は 1.0)
			const float wave = (std::cos(m_blinkTimer * 2.0f * PI / BLINK_PERIOD_SEC) + 1.0f) * 0.5f;
			const float blinkAlpha = BLINK_MIN_ALPHA + (1.0f - BLINK_MIN_ALPHA) * wave;

			for (int i = 0; i < ITEM_COUNT; ++i)
			{
				const SoundOptionItem& item = ITEM_TABLE[i];
				const int volumeStep = ToVolumeStep(GetItemVolume(item));

				if (m_textList[i])
				{
					Vector4 color = COLOR_TEXT;
					if (i == m_selectIndex)
					{
						color.w = blinkAlpha;
					}
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


		void SoundOptionMenu::Select(const int index)
		{
			m_selectIndex = index;
			// 選んだ瞬間は、はっきり見えるところから点滅を始める
			m_blinkTimer = 0.0f;
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


		void SoundOptionMenu::ChangeVolume(const int step)
		{
			const SoundOptionItem& item = ITEM_TABLE[m_selectIndex];

			int volumeStep = ToVolumeStep(GetItemVolume(item)) + step;
			volumeStep = TClamp<int>::Clamp(volumeStep, 0, VOLUME_STEP_COUNT);
			SetItemVolume(item, static_cast<float>(volumeStep) / VOLUME_STEP_COUNT);

			// 変えた音量で、どのくらいの大きさかを聞かせる(BGMは鳴っているので不要。ボイスはAボタン)
			if (item.isMaster || item.type == sound::EnSoundType::SE)
			{
				sound::SoundManager::GetInstance()->Play(sound::EnSoundType::SE, sound::se::DECIDE);
			}
		}
	}
}
