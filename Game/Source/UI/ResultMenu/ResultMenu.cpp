/**
 * @file ResultMenu.cpp
 * @brief リザルトの UI(写真アルバム風)
 */
#include "stdafx.h"
#include "ResultMenu.h"
#include <algorithm>


namespace app
{
	namespace ui
	{
		namespace
		{
			/** UI の名前(JSON の "name") */
			constexpr const char* TITLE_NAME = "ResultTitle";
			constexpr const char* MISSION_BASE_NAME = "ResultMissionBase";
			constexpr const char* PHOTO_BASE_NAME = "ResultPhotoBase";
			constexpr const char* EMPTY_NAME = "ResultEmpty";
			constexpr const char* RETRY_NAME = "ResultRetry";
			constexpr const char* TITLE_SELECT_NAME = "ResultToTitle";
			constexpr const char* COUNT_SHOT_NAME = "ResultCountShot";
			constexpr const char* COUNT_MATCH_NAME = "ResultCountMatch";
			constexpr const char* COUNT_MISS_NAME = "ResultCountMiss";
			/** 枠などに使う白い画像 */
			constexpr const char* WHITE_ASSET = "Assets/spriteData/UI/Common/White.DDS";
			/** 写真をとめる画びょうの画像(上書きすれば差し替わる)。照合〇 は緑、照合なし× と対象外は赤 */
			constexpr const char* PIN_GREEN_ASSET = "Assets/spriteData/UI/Result/PinGreen.DDS";
			constexpr const char* PIN_RED_ASSET = "Assets/spriteData/UI/Result/PinRed.DDS";

			// ミッションの行
			/** 行の間隔 */
			constexpr float MISSION_ROW_SPACING = 50.0f;
			/** 行の文字のスケール */
			const Vector3 MISSION_ROW_SCALE = { 0.8f,0.8f,1.0f };

			// 写真
			/** 横に並べる枚数 */
			constexpr int PHOTO_COLUMN = 4;
			/** 写真の大きさ(16:9) */
			const Vector2 PHOTO_SIZE = { 384.0f,216.0f };
			/** 写真の横の間隔(写真の左端から次の左端まで) */
			constexpr float PHOTO_SPACING_X = 430.0f;
			/** 写真の縦の間隔(写真の上端から次の上端まで。下に結果の文字が入る) */
			constexpr float PHOTO_SPACING_Y = 330.0f;
			/** プリントの白いふちの太さ(上・左・右) */
			constexpr float PRINT_MARGIN = 10.0f;
			/** プリントの下のふちの太さ(ここに結果の文字を書く) */
			constexpr float PRINT_BOTTOM = 76.0f;
			/** 写真の下端から、結果の文字までの距離 */
			constexpr float PHOTO_LABEL_OFFSET_Y = 8.0f;
			/** 結果の文字のスケール */
			const Vector3 PHOTO_LABEL_SCALE = { 0.5f,0.5f,1.0f };

			// かげと画びょう
			/** プリントのかげのずれ(右下へ) */
			const Vector2 SHADOW_OFFSET = { 8.0f,-8.0f };
			/** プリントのかげの色 */
			const Vector4 COLOR_SHADOW = Vector4(0.0f, 0.0f, 0.0f, 0.4f);
			/** 画びょうの大きさ(画像は正方形で、真ん中に画びょうを描く) */
			const Vector2 PIN_SIZE = { 72.0f,72.0f };
			/** プリントの上の端から、画びょうの真ん中までの距離(下へ) */
			constexpr float PIN_OFFSET_Y = 6.0f;
			/** 画びょうの傾き(度)。写真ごとに順に使い、手でとめたように少しずつばらばらにする */
			constexpr float PIN_ROTATION_DEG_LIST[] = { -20.0f, 15.0f, -8.0f, 24.0f, -14.0f, 8.0f, -26.0f, 18.0f };
			/** 対象外の画びょうの色(赤い画像を暗くする) */
			const Vector4 COLOR_PIN_NEUTRAL = Vector4(0.55f, 0.55f, 0.55f, 1.0f);
			/** 画びょうのかげのずれ(右下へ。画びょうを傾けても、かげはいつも同じ向きに落ちる) */
			const Vector2 PIN_SHADOW_OFFSET = { 6.0f,-7.0f };
			/** 画びょうのかげの色(画びょうの画像を、黒くて半分すける色にする) */
			const Vector4 COLOR_PIN_SHADOW = Vector4(0.0f, 0.0f, 0.0f, 0.45f);

			// ハンコ
			/** 写真の右上から、ハンコの真ん中までの距離 */
			const Vector2 STAMP_OFFSET = { 95.0f,45.0f };
			/** ハンコのスケール */
			const Vector3 STAMP_SCALE = { 1.1f,1.1f,1.0f };
			/** ハンコの傾き(度) */
			constexpr float STAMP_ROTATION_DEG = 12.0f;
			/** ハンコの文字 */
			constexpr const wchar_t* STAMP_MATCH_TEXT = L"照合〇";
			constexpr const wchar_t* STAMP_MISS_TEXT = L"照合なし×";
			constexpr const wchar_t* STAMP_NEUTRAL_TEXT = L"対象外";

			// 出てくる動き
			/** 1 枚目が出るまでの時間(秒) */
			constexpr float PHOTO_START_TIME = 0.3f;
			/** 次の写真が出るまでの時間(秒) */
			constexpr float PHOTO_INTERVAL = 0.15f;
			/** 写真が出てから、ハンコを押すまでの時間(秒) */
			constexpr float STAMP_DELAY = 0.1f;
			/** ハンコが押し終わるまでの時間(秒) */
			constexpr float STAMP_IN_TIME = 0.12f;
			/** ハンコの最初の大きさ(倍。ここから元の大きさまで小さくなる) */
			constexpr float STAMP_IN_SCALE = 2.2f;

			// 色
			/** 成功・達成の色 */
			const Vector4 COLOR_SUCCESS = Vector4(1.0f, 0.85f, 0.2f, 1.0f);
			/** 失敗の色 */
			const Vector4 COLOR_FAILED = Vector4(1.0f, 0.35f, 0.3f, 1.0f);
			/** 数えなかった(失敗ではない)色 */
			const Vector4 COLOR_NEUTRAL = Vector4(0.65f, 0.65f, 0.65f, 1.0f);
			/** 写真そのままの色 */
			const Vector4 COLOR_WHITE = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
			/** プリントの白いふちの色(真っ白より少し黄色くして、紙に見せる) */
			const Vector4 COLOR_PRINT = Vector4(0.95f, 0.94f, 0.9f, 1.0f);
			/** プリントに書く文字の色 */
			const Vector4 COLOR_PRINT_TEXT = Vector4(0.15f, 0.15f, 0.15f, 1.0f);
			/** ハンコの色(ジャッジのハンコと同じ) */
			const Vector4 COLOR_STAMP_MATCH = Vector4(0.35f, 0.9f, 0.45f, 1.0f);
			const Vector4 COLOR_STAMP_MISS = Vector4(1.0f, 0.3f, 0.3f, 1.0f);
			const Vector4 COLOR_STAMP_NEUTRAL = Vector4(0.6f, 0.6f, 0.6f, 1.0f);

			/** 左上を基準にする */
			const Vector2 PIVOT_LEFT_TOP = { 0.0f,1.0f };
			/** 左端の真ん中を基準にする */
			const Vector2 PIVOT_LEFT_CENTER = { 0.0f,0.5f };
			/** 真ん中を基準にする */
			const Vector2 PIVOT_CENTER = { 0.5f,0.5f };


			/**
			 * @brief 見え方から色を決める(色は View の持ち物)
			 * @param tone 見え方
			 * @return 色
			 */
			const Vector4& GetToneColor(const ResultTone tone)
			{
				switch (tone)
				{
				case ResultTone::Success: return COLOR_SUCCESS;
				case ResultTone::Failed:  return COLOR_FAILED;
				default:                  return COLOR_NEUTRAL;
				}
			}


			/**
			 * @brief 見え方から、ハンコの文字を決める
			 * @param tone 見え方
			 * @return ハンコの文字
			 */
			const wchar_t* GetStampText(const ResultTone tone)
			{
				switch (tone)
				{
				case ResultTone::Success: return STAMP_MATCH_TEXT;
				case ResultTone::Failed:  return STAMP_MISS_TEXT;
				default:                  return STAMP_NEUTRAL_TEXT;
				}
			}


			/**
			 * @brief 見え方から、画びょうの画像を決める
			 * @param tone 見え方
			 * @return 画びょうの画像のファイルパス
			 */
			const char* GetPinAsset(const ResultTone tone)
			{
				return tone == ResultTone::Success ? PIN_GREEN_ASSET : PIN_RED_ASSET;
			}


			/**
			 * @brief 見え方から、ハンコの色を決める
			 * @param tone 見え方
			 * @return ハンコの色
			 */
			const Vector4& GetStampColor(const ResultTone tone)
			{
				switch (tone)
				{
				case ResultTone::Success: return COLOR_STAMP_MATCH;
				case ResultTone::Failed:  return COLOR_STAMP_MISS;
				default:                  return COLOR_STAMP_NEUTRAL;
				}
			}
		}


		ResultMenu::ResultMenu()
			: m_titleText(nullptr)
			, m_missionBase(nullptr)
			, m_photoBase(nullptr)
			, m_emptyText(nullptr)
			, m_countShotText(nullptr)
			, m_countMatchText(nullptr)
			, m_countMissText(nullptr)
			, m_timer(0.0f)
			, m_selectButtonList{}
			, m_selectType(SelectType::Retry)
			, m_hasViewData(false)
		{}


		ResultMenu::~ResultMenu()
		{}


		void ResultMenu::InitializeLogic()
		{
			// ホットリロードでキャンバスが作り直されると、作った UI は消えている(キーも要らない)
			m_createdKeyList.clear();
			m_photoGroupList.clear();

			// ※ JSON の "type" と型が違うと nullptr になる(その UI は出なくなるので、JSON を確認する)
			m_titleText = FindUI<UIText>(TITLE_NAME);
			m_missionBase = FindUI<UIBase>(MISSION_BASE_NAME);
			m_photoBase = FindUI<UIBase>(PHOTO_BASE_NAME);
			m_emptyText = FindUI<UIText>(EMPTY_NAME);
			m_countShotText = FindUI<UIText>(COUNT_SHOT_NAME);
			m_countMatchText = FindUI<UIText>(COUNT_MATCH_NAME);
			m_countMissText = FindUI<UIText>(COUNT_MISS_NAME);
			m_selectButtonList[static_cast<size_t>(SelectType::Retry)] = FindUI<UIButton>(RETRY_NAME);
			m_selectButtonList[static_cast<size_t>(SelectType::Title)] = FindUI<UIButton>(TITLE_SELECT_NAME);
			ApplySelect();

			// 値を設定した後(ホットリロード)なら、ここで作り直す
			CreateResultUI();
		}


		void ResultMenu::Update()
		{
			// 写真が順に出てくる動きを進める
			m_timer += g_gameTime->GetFrameDeltaTime();
			ApplyAnimation();

			UIMenuBase::Update();
		}


		void ResultMenu::SetViewData(const ResultViewData& viewData)
		{
			m_viewData = viewData;
			m_hasViewData = true;
			// 出てくる動きを最初から
			m_timer = 0.0f;
			CreateResultUI();
		}


		void ResultMenu::SetSelect(const SelectType type)
		{
			m_selectType = type;
			ApplySelect();
		}


		void ResultMenu::CreateResultUI()
		{
			RemoveCreatedUI();
			if (!m_hasViewData) return;

			if (m_titleText)
			{
				m_titleText->SetText(m_viewData.title.text.c_str());
				m_titleText->SetColor(GetToneColor(m_viewData.title.tone));
			}

			CreateMissionRows();
			CreatePhotos();
			ApplyCount();
			ApplyAnimation();
		}


		void ResultMenu::RemoveCreatedUI()
		{
			for (const uint32_t key : m_createdKeyList)
			{
				GetCanvas()->RemoveUI(key);
			}
			m_createdKeyList.clear();
			m_photoGroupList.clear();
		}


		void ResultMenu::CreateMissionRows()
		{
			if (m_missionBase == nullptr) return;

			const Vector3& basePosition = m_missionBase->GetPosition();
			for (int i = 0; i < static_cast<int>(m_viewData.missionRowList.size()); ++i)
			{
				const ResultTextData& row = m_viewData.missionRowList[i];

				// 基準から下に、行の数だけずらす
				const Vector3 position(basePosition.x, basePosition.y - MISSION_ROW_SPACING * static_cast<float>(i), basePosition.z);
				CreateText("ResultMission" + std::to_string(i), row.text, position, MISSION_ROW_SCALE, GetToneColor(row.tone), PIVOT_LEFT_CENTER);
			}
		}


		void ResultMenu::CreatePhotos()
		{
			// 写真が無い時だけ「写真はありません」を出す
			if (m_emptyText) m_emptyText->SetIsDraw(m_viewData.photoList.empty());
			if (m_photoBase == nullptr) return;

			const Vector3& basePosition = m_photoBase->GetPosition();
			for (int i = 0; i < static_cast<int>(m_viewData.photoList.size()); ++i)
			{
				const ResultPhotoData& photoData = m_viewData.photoList[i];

				// 左上から、横に PHOTO_COLUMN 枚ずつ並べる
				const int column = i % PHOTO_COLUMN;
				const int row = i / PHOTO_COLUMN;
				const Vector3 photoPosition(
					basePosition.x + PHOTO_SPACING_X * static_cast<float>(column),
					basePosition.y - PHOTO_SPACING_Y * static_cast<float>(row),
					basePosition.z);

				const std::string number = std::to_string(i);
				PhotoGroup group;

				// プリントの白いふち(写真より大きい紙を、写真の後ろに置く。下は文字を書くので広い)
				// ※ キャンバスは作った順に描くので、かげ → ふち → 写真 の順に作る
				const Vector2 printSize(PHOTO_SIZE.x + PRINT_MARGIN * 2.0f, PHOTO_SIZE.y + PRINT_MARGIN + PRINT_BOTTOM);
				const Vector3 printPosition(photoPosition.x - PRINT_MARGIN, photoPosition.y + PRINT_MARGIN, photoPosition.z);

				// かげ(コルクから少し浮いて見えるように、右下へずらす)
				group.partList.push_back(CreateIcon("ResultPrintShadow" + number, WHITE_ASSET, printSize,
					Vector3(printPosition.x + SHADOW_OFFSET.x, printPosition.y + SHADOW_OFFSET.y, printPosition.z),
					COLOR_SHADOW, PIVOT_LEFT_TOP));

				group.partList.push_back(CreateIcon("ResultPrint" + number, WHITE_ASSET, printSize, printPosition, COLOR_PRINT, PIVOT_LEFT_TOP));

				// 写真(絵が無い時は、白いふちだけ残る)
				UIIcon* photo = CreateIcon("ResultPhoto" + number, "", PHOTO_SIZE, photoPosition, COLOR_WHITE, PIVOT_LEFT_TOP);
				if (photoData.texture)
				{
					photo->SetTexture(*photoData.texture);
				}
				group.partList.push_back(photo);

				// 画びょう(プリントの上の真ん中にとめる。写真より後に作って、手前に出す)
				// ※ 照合〇 は緑、照合なし× は赤。真ん中を基準に少し傾ける
				const char* pinAsset = GetPinAsset(photoData.tone);
				const Vector3 pinPosition(printPosition.x + printSize.x * 0.5f, printPosition.y - PIN_OFFSET_Y, printPosition.z);
				constexpr int pinRotationCount = static_cast<int>(sizeof(PIN_ROTATION_DEG_LIST) / sizeof(PIN_ROTATION_DEG_LIST[0]));
				Quaternion pinRotation = Quaternion::Identity;
				pinRotation.SetRotationDegZ(PIN_ROTATION_DEG_LIST[i % pinRotationCount]);

				// 画びょうのかげ(同じ画像を黒く半分すけさせて、右下にずらして先に描く)
				// ※ 画像にかげを描きこむと、傾けた時にかげの向きもばらばらになるので、コードで付ける
				UIIcon* pinShadow = CreateIcon("ResultPinShadow" + number, pinAsset, PIN_SIZE,
					Vector3(pinPosition.x + PIN_SHADOW_OFFSET.x, pinPosition.y + PIN_SHADOW_OFFSET.y, pinPosition.z),
					COLOR_PIN_SHADOW, PIVOT_CENTER);
				pinShadow->SetRotation(pinRotation);
				group.partList.push_back(pinShadow);

				UIIcon* pin = CreateIcon("ResultPin" + number, pinAsset, PIN_SIZE, pinPosition,
					photoData.tone == ResultTone::Neutral ? COLOR_PIN_NEUTRAL : COLOR_WHITE, PIVOT_CENTER);
				pin->SetRotation(pinRotation);
				group.partList.push_back(pin);

				// 結果の文字(写真の下の、白いふちに書く)
				const Vector3 labelPosition(photoPosition.x, photoPosition.y - PHOTO_SIZE.y - PHOTO_LABEL_OFFSET_Y, photoPosition.z);
				group.partList.push_back(CreateText("ResultPhotoLabel" + number, photoData.label, labelPosition, PHOTO_LABEL_SCALE, COLOR_PRINT_TEXT, PIVOT_LEFT_TOP));

				// 照合のハンコ(写真の右上に、少し傾けて押す)
				Quaternion stampRotation = Quaternion::Identity;
				stampRotation.SetRotationDegZ(STAMP_ROTATION_DEG);
				const Vector3 stampPosition(photoPosition.x + PHOTO_SIZE.x - STAMP_OFFSET.x, photoPosition.y - STAMP_OFFSET.y, photoPosition.z);
				group.stamp = CreateText("ResultPhotoStamp" + number, GetStampText(photoData.tone), stampPosition, STAMP_SCALE,
					GetStampColor(photoData.tone), PIVOT_CENTER, stampRotation);
				group.stamp->SetShadow(true, 2.0f, Vector4(0.0f, 0.0f, 0.0f, 1.0f));

				m_photoGroupList.push_back(group);
			}
		}


		void ResultMenu::ApplyCount()
		{
			const ResultCountData& count = m_viewData.count;
			if (m_countShotText)
			{
				const std::wstring text = L"撮った写真  " + std::to_wstring(count.shotCount) + L" 枚";
				m_countShotText->SetText(text.c_str());
			}
			if (m_countMatchText)
			{
				const std::wstring text = std::wstring(STAMP_MATCH_TEXT) + L"  " + std::to_wstring(count.matchCount) + L" 枚";
				m_countMatchText->SetText(text.c_str());
				m_countMatchText->SetColor(COLOR_STAMP_MATCH);
			}
			if (m_countMissText)
			{
				const std::wstring text = std::wstring(STAMP_MISS_TEXT) + L"  " + std::to_wstring(count.missCount) + L" 枚";
				m_countMissText->SetText(text.c_str());
				m_countMissText->SetColor(COLOR_STAMP_MISS);
			}
		}


		void ResultMenu::ApplyAnimation()
		{
			for (int i = 0; i < static_cast<int>(m_photoGroupList.size()); ++i)
			{
				PhotoGroup& group = m_photoGroupList[i];

				// 左上から順に 1 枚ずつ出す
				const float photoTime = m_timer - (PHOTO_START_TIME + PHOTO_INTERVAL * static_cast<float>(i));
				const bool isPhotoShown = photoTime >= 0.0f;
				for (UIBase* part : group.partList)
				{
					part->SetIsDraw(isPhotoShown);
				}

				// ハンコは写真の少し後に、大きい所から押し付ける
				if (group.stamp == nullptr) continue;
				const float stampTime = photoTime - STAMP_DELAY;
				group.stamp->SetIsDraw(stampTime >= 0.0f);
				const float rate = (std::min)((std::max)(stampTime / STAMP_IN_TIME, 0.0f), 1.0f);
				const float scale = STAMP_IN_SCALE + (1.0f - STAMP_IN_SCALE) * rate;
				group.stamp->SetScale(Vector3(STAMP_SCALE.x * scale, STAMP_SCALE.y * scale, STAMP_SCALE.z));
			}
		}


		void ResultMenu::ApplySelect()
		{
			for (int i = 0; i < static_cast<int>(m_selectButtonList.size()); ++i)
			{
				UIButton* button = m_selectButtonList[i];
				if (button == nullptr) continue;

				// 色と大きさは、ボタンが形に合わせて変える(全部の画面で同じ見た目)
				const bool isSelected = i == static_cast<int>(m_selectType);
				button->SetState(isSelected ? UIButton::ButtonState::Selected : UIButton::ButtonState::Normal);
			}
		}


		void ResultMenu::PressSelect()
		{
			UIButton* button = m_selectButtonList[static_cast<size_t>(m_selectType)];
			if (button) button->Press();
		}


		UIText* ResultMenu::CreateText(const std::string& name, const std::wstring& text, const Vector3& position, const Vector3& scale, const Vector4& color, const Vector2& pivot,
			const Quaternion& rotation)
		{
			const uint32_t key = Hash32(name.c_str());
			UIText* ui = GetCanvas()->CreateUI<UIText>(key);
			ui->Initialize(text.c_str(), position, scale, rotation, color, pivot);
			m_createdKeyList.push_back(key);
			return ui;
		}


		UIIcon* ResultMenu::CreateIcon(const std::string& name, const char* assetName, const Vector2& size, const Vector3& position, const Vector4& color, const Vector2& pivot)
		{
			const uint32_t key = Hash32(name.c_str());
			UIIcon* ui = GetCanvas()->CreateUI<UIIcon>(key);
			ui->Initialize(assetName, size.x, size.y, position, Vector3::One, Quaternion::Identity, color, pivot);
			m_createdKeyList.push_back(key);
			return ui;
		}
	}
}
